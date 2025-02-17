//
// Created by consti10 on 19.04.23.
//

#include "TCPEndpoint.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <csignal>
#include <utility>

TCPEndpoint::TCPEndpoint(TCPEndpoint::Config config)
    : MEndpoint("TCPServer"),
      m_config(std::move(config))
{
  m_console = openhd::log::create_or_get(TAG);
  assert(m_console);
  m_loop_thread=std::make_unique<std::thread>(&TCPEndpoint::loop, this);
  m_console->debug("created with {}",m_config.port);
}

TCPEndpoint::~TCPEndpoint() {
  keep_alive= false;
  // this signals the fd to stop if needed
  //close(server_fd);
  shutdown(server_fd, SHUT_RDWR);
  if(m_loop_thread){
    m_loop_thread->join();
    m_loop_thread.reset();
  }
}

bool TCPEndpoint::sendMessagesImpl(
    const std::vector<MavlinkMessage>& messages) {
  if(client_socket !=0){
    auto message_buffers= aggregate_pack_messages(messages,1024);
    for(const auto& message_buffer:message_buffers){
      //send(client_socket, message_buffer.data(), message_buffer.size(), 0);
      send(client_socket, message_buffer.aggregated_data->data(), message_buffer.aggregated_data->size(), MSG_NOSIGNAL); //otherwise we might crash if the socket disconnects
    }
    return true;
  }
  return false;
}

void TCPEndpoint::loop() {
  while (keep_alive){
    m_console->debug("TCPEndpoint::setup_and_allow_connection_once() begin");
    setup_and_allow_connection_once();
    m_console->debug("TCPEndpoint::setup_and_allow_connection_once() end");
    if(keep_alive){
      // Don't peg cpu on errors
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

void TCPEndpoint::setup_and_allow_connection_once() {
  struct sockaddr_in sockaddr{};
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    m_console->warn("open socket failed");
    return ;
  }
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET,
                 SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    m_console->warn("setsockopt failed");
    close(server_fd);
    return ;
  }
  sockaddr.sin_family = AF_INET;
  sockaddr.sin_addr.s_addr = INADDR_ANY;
  sockaddr.sin_port = htons(m_config.port);

  if (bind(server_fd, (struct sockaddr*)&sockaddr,sizeof(sockaddr))< 0) {
    m_console->warn("bind failed");
    close(server_fd);
    return ;
  }
  // signal readiness to accept clients
  if (listen(server_fd, 3) < 0) {
    m_console->warn("listen failed");
    close(server_fd);
    return ;
  }
  m_console->debug("After listen");
  const  int addrlen = sizeof(sockaddr);
  const auto accept_result=accept(server_fd, (struct sockaddr*)&sockaddr,(socklen_t*)&addrlen);
  if (accept_result < 0) {
    m_console->debug("accept failed");
    close(server_fd);
    return ;
  }
  client_socket =accept_result;
  m_console->debug("accepted client,sockfd:{}, ip:{}, port:{}",client_socket,
                   inet_ntoa(sockaddr.sin_addr),ntohs(sockaddr.sin_port));
  // read buffer
  const auto buff = std::make_unique<std::array<uint8_t,READ_BUFF_SIZE>>();
  while (keep_alive){
    // Read from all the client(s)
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    //const ssize_t message_length = recv(client_socket, buff->data(), buff->size(), MSG_WAITALL);
    const ssize_t message_length = read(client_socket, buff->data(), buff->size());
    if(message_length<0){
      m_console->debug("Read error {} {}",message_length,strerror(errno));
      break ;
    }
    if(message_length==0){
      m_console->debug("Client disconnected");
      break ;
    }
    MEndpoint::parseNewData(buff->data(),message_length);
  }
  // closing the connected (client) socket(s)
  close(client_socket);
  client_socket =0;
  // closing the listening socket
  close(server_fd);
}

