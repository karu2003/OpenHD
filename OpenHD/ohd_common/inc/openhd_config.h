//
// Created by consti10 on 17.02.23.
//

#ifndef OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
#define OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_

#include <string>
#include <vector>

namespace openhd{

static constexpr auto RPI_ETHERNET_ONLY="RPI_ETHERNET_ONLY";

// NOTE: Read the .config - file itself for documentation what these variables do
struct Config{
  // WIFI
  bool WIFI_ENABLE_AUTODETECT= true;
  std::vector<std::string> WIFI_WB_LINK_CARDS{};
  std::string WIFI_WIFI_HOTSPOT_CARD;
  // CAMERAS
  bool CAMERA_ENABLE_AUTODETECT = true;
  int CAMERA_N_CAMERAS;
  std::string CAMERA_CAMERA0_TYPE;
  std::string CAMERA_CAMERA1_TYPE;
  // NETWORKING
  std::string NW_ETHERNET_CARD=RPI_ETHERNET_ONLY;
  std::vector<std::string> NW_MANUAL_FORWARDING_IPS;
  bool NW_FORWARD_TO_LOCALHOST_58XX=false;
  // GENERAL
  bool GEN_ENABLE_LAST_KNOWN_POSITION= true;
};

Config load_config();

void debug_config(const Config& config);
void debug_config();

// Control ethernet via mavlink & Network manager - only on on rpi by default, otherwise card needs to be manually specified
bool nw_ethernet_card_manual_active(const Config& config);

}

#endif  // OPENHD_OPENHD_OHD_COMMON_INC_OPENHD_CONFIG_H_
