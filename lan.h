#pragma once

#define WIFI_SSID "panda"
#define WIFI_PASSWORD "pandapandapan"

// #define WIFI_SSID "pico"
// #define WIFI_PASSWORD "picopicopicop"

int best_lan_init() ;
int best_lan_wifi_connect() ;
void best_lan_led(bool onoff) ;
void best_lan_poll() ;
char* best_lan_get_local_addr() ;
bool best_lan_link_is_up() ;
