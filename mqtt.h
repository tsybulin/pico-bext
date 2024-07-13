#pragma once

#define MQTT_HOST "172.16.101.251"
#define MQTT_CLIENT_ID "pico-best"
#define MQTT_LOGIN "tasmota"
#define MQTT_PASSWORD "123456"

int best_mqtt_prepare() ;
int best_mqtt_connect() ;
int best_mqtt_connected() ;
void best_mqtt_subscribe(const char *topic) ;
int best_mqtt_publish(const char *topic, const char *payload) ;
