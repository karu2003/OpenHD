//
// Created by consti10 on 17.05.22.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFIHOTSPOT_H_
#define OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFIHOTSPOT_H_

#include <future>
#include <string>
#include <vector>

#include "openhd_settings_imp.hpp"
#include "wifi_card.h"

/**
 * Wifi hotspot refers to creating a WiFi Access point on the device we are running on.
 * External clients like QOpenHD running on a tablet can then connect to the hotspot.
 * Note that auto video and telemetry forwarding is not implemented for WiFi hotspot -
 * On the one hand, this is prone to errors anyways, on the other hand, it is hard to do to actively search for connected devices and their IPs.
 * TCP mavlink (perhaps also video in the future) is the way to go here.
 * Change Nov4 2022: Uses network manager - we already have network manager installed and enabled by default on the rpi on the openhd images,
 * but the default raspbian images from pi foundation have it only installed, but disabled by default (they'l use it eventually)
 */
class WifiHotspot {
 public:
  /**
   * Utility for starting, stopping WIFI AP (Hotspot) and forwarding the client connect/disconnect events.
   */
  explicit WifiHotspot(WiFiCard wifiCard,const openhd::WifiSpace& wifibroadcast_frequency_space);
  WifiHotspot(const WifiHotspot&)=delete;
  WifiHotspot(const WifiHotspot&&)=delete;
  ~WifiHotspot();
  // Use opposite frequency band to wfb if possible
  static bool get_use_5g_channel(const WiFiCard& wifiCard,const openhd::WifiSpace& wifibroadcast_frequency_space);
  void set_enabled(bool enable);
 private:
  // NOTE: might block, use async
  // just runs the appropriate network manager (nmcli) command to start an already created wifi hotspot connection
  void start();
  // NOTE: might block,use async
  // just runs the appropriate network manager (nmcli) command to stop an already created wifi hotspot connection
  void stop();
  void start_async();
  void stop_async();
  // Ip addresses of all connected clients.
  // A client might dynamically connect or disconnect from the AP at run time,
  // In this case the apropriate callbacks have to be called.
  std::vector<std::string> connectedClientsIps;
  const WiFiCard m_wifi_card;
  bool started=false;
  std::shared_ptr<spdlog::logger> m_console;
  std::future<void> m_last_async_operation;
  bool m_use_5G_channel;
};

#endif //OPENHD_OPENHD_OHD_INTERFACE_SRC_WIFIHOTSPOT_H_
