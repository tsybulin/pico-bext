#include "lan.h"

#include "pico/cyw43_arch.h"

#include "lwipopts.h"

int best_lan_init() {
    return cyw43_arch_init() ;
}

int best_lan_wifi_connect() {
    cyw43_arch_enable_sta_mode() ;
    return cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000) ;
}

void best_lan_led(bool onoff) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, onoff) ;
}

void best_lan_poll() {
    cyw43_arch_poll() ;
}

char* best_lan_get_local_addr() {
    return ip4addr_ntoa(netif_ip4_addr(netif_list)) ;
}

bool best_lan_link_is_up() {
    return netif_is_link_up(netif_default) ;
}
