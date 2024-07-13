#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

#include "lwipopts.h"
#include "lan.h"
#include "mqtt.h"
#include "entity.h"
#include "nxt.h"

repeating_timer_t rt_blink ;
volatile int blink = 0 ;

Nxt nxt ;

void reboot() {
    watchdog_enable(10, false) ;
    while (true) { }
}

bool rt_blinker(repeating_timer_t *rt) {
    if (!blink) {
        return false ;
    }

    if (blink & 1) {
        blink = 2 ;
    } else {
        blink = 1 ;
    }

    return true ;
}

entity_changed_t entity_changed_cb = [](Entity *e) {
#ifndef NDEBUG
    printf("changed e.id:%d\n", e->id) ;
#endif
    nxt.entityDidChange(e) ;
} ;

int main() {
    stdio_init_all();

#ifndef NDEBUG
    puts("Start\n");
#endif
    uart_init(uart1, 115200) ;
    gpio_set_function(4, GPIO_FUNC_UART) ;
    gpio_set_function(5, GPIO_FUNC_UART) ;

    if (best_lan_init()) {
        reboot() ;
    }

    if (best_lan_wifi_connect()) {
        reboot() ;
    }


#ifndef NDEBUG
    printf("Connected as %s\n", best_lan_get_local_addr()) ;
#endif

    if (best_mqtt_prepare()) {
        reboot() ;
    }

    if (best_mqtt_connect()) {
        reboot() ;
    }

    nxt.begin(uart1, 115200) ;

    bool wifi_reconnect = false ;
    bool mqtt_reconnecting = false ;

    nxt.start() ;

    blink = 1 ;
    add_repeating_timer_ms(250, rt_blinker, nullptr, &rt_blink) ;

    while (true) {
        if (!best_lan_link_is_up()) {
            wifi_reconnect = true ;
            best_lan_poll() ;
            
            if (blink) {
                blink = 0 ;
                cancel_repeating_timer(&rt_blink) ;
            }
            
            continue ;
        } else {
            if (wifi_reconnect) {
                wifi_reconnect = false ;
#ifndef NDEBUG
                printf("Reconnected as %s\n", best_lan_get_local_addr()) ;
#endif
            }
        }

        if (!best_mqtt_connected()) {
            if (!mqtt_reconnecting) {
                mqtt_reconnecting = true ;
                if (best_mqtt_connect()) {
                    mqtt_reconnecting = false ;
                }
            }
            
            best_lan_poll() ;
            
            if (blink) {
                blink = 0 ;
                cancel_repeating_timer(&rt_blink) ;
            }
            
            continue ;
        } else {
            if (mqtt_reconnecting) {
                mqtt_reconnecting = false ;
#ifndef NDEBUG
                printf("MQTT Reconnected\n") ;
#endif
            }
        }

        if (!blink) {
            blink = 1 ;
            add_repeating_timer_ms(250, rt_blinker, nullptr, &rt_blink) ;
        } else if (blink & 1) {
            best_lan_led(true) ;
        } else {
            best_lan_led(false) ;
        }

        best_lan_poll() ;
        nxt.loop() ;
    }
    

    return 0;
}
