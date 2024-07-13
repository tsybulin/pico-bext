#include "mqtt.h"

#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "string.h"

#include "lwipopts.h"
#include "dns.h"

typedef struct mqtt_t_ {
    ip_addr_t mqtt_addr ;
    mqtt_client_t *mqtt_client ;
    bool done ;
    bool connected ;
} mqtt_t ;

typedef struct mqtt_data_t_ {
    char topic[255] ;
    u32_t tot_len ;
    u8_t buffer[1025] ;
    u16_t data_len ;

} mqtt_data_t ;

mqtt_t mt ;
mqtt_data_t md ;

extern void best_mqtt_e_connected() ;
extern void best_mqtt_e_message_received(const char *topic, u8_t *buffer, u8_t len) ;

int best_mqtt_prepare() {
    ipaddr_aton(MQTT_HOST, &mt.mqtt_addr) ;

    // if (best_dns_gethostbyname(MQTT_HOST, &mt.mqtt_addr)) {
    //     puts("best_dns_gethostbyname failed\n") ;
    //     return 1 ;
    // }

    // // printf("MQTT address %s\n", ipaddr_ntoa(&mt.mqtt_addr)) ;

    if (!mt.mqtt_client) {
        mt.mqtt_client = mqtt_client_new() ;
    }

    return 0 ;
}

void best_mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    mqtt_t *mt = (mqtt_t *)arg ;
    mt->done = true ;
    if (status == MQTT_CONNECT_ACCEPTED) {
        mt->connected = true ;
#ifndef NDEBUG
        printf("MQTT Connected\n") ;
#endif
        best_mqtt_e_connected() ;
    }
#ifndef NDEBUG
     else {
        printf("MQTT Disconnected, reason: %d\n", status) ;
     }
#endif
}

void best_mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    mqtt_data_t *md = (mqtt_data_t *)arg ;
    strcpy(md->topic, topic) ;
    md->tot_len = tot_len ;
    md->data_len = 0 ;
    memset(md->buffer, 0, sizeof(md->buffer)) ;
}

void best_mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    mqtt_data_t *md = (mqtt_data_t *)arg ;
    if (md->tot_len > 0) {
        md->tot_len -= len ;
        memcpy(&md->buffer[md->data_len], data, len) ;
        md->data_len += len ; 

        if (md->tot_len == 0) {
            best_mqtt_e_message_received(md->topic, md->buffer, md->data_len) ;
        }
    }
}

int best_mqtt_connect() {
    struct mqtt_connect_client_info_t ci ;

    memset(&ci, 0, sizeof(ci)) ;
    ci.client_id = MQTT_CLIENT_ID ;
    ci.client_user = MQTT_LOGIN ;
    ci.client_pass = MQTT_PASSWORD ;
    ci.keep_alive = 60 ;

    mt.connected = false ;
    mt.done = false ;

    err_t err = mqtt_client_connect(mt.mqtt_client, &mt.mqtt_addr, MQTT_PORT, best_mqtt_connection_cb, &mt, &ci) ;

    if (err != ERR_OK && err != ERR_INPROGRESS) {
        // printf("mqtt_client_connect err %d\n", err) ;
        return err ;
    }

    while (!mt.done) {
        cyw43_arch_poll() ;
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000)) ;
    }

    if (mt.connected) {
        mqtt_set_inpub_callback(mt.mqtt_client, best_mqtt_incoming_publish_cb, best_mqtt_incoming_data_cb, &md) ;
    }

    return mt.connected ? ERR_OK : ERR_ABRT ;
}

int best_mqtt_connected() {
    return mqtt_client_is_connected(mt.mqtt_client) ;
}

void best_mqtt_sub_cb(void *arg, err_t err) {
    if (arg) {
        bool *done = (bool *)arg ;
        *done = true ;
    }
}

void best_mqtt_subscribe(const char *topic) {
    bool done = false ;
    cyw43_arch_lwip_begin() ;
    mqtt_sub_unsub(mt.mqtt_client, topic, 0, best_mqtt_sub_cb, &done, 1) ;
    cyw43_arch_lwip_end() ;

    while (!done) {
        cyw43_arch_poll() ;
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000)) ;
    }
}

void best_mqtt_pub_cb(void *arg, err_t err) {
    
}

int best_mqtt_publish(const char *topic, const char *payload) {
    if (!best_mqtt_connected()) {
        return ERR_CONN ;
    }
    
    return mqtt_publish(mt.mqtt_client, topic, payload, strlen(payload), 0, 0, best_mqtt_pub_cb, nullptr) ;
}
