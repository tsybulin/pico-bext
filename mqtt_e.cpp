#include "entity.h"

#include "lwip/arch.h"
#include "string.h"

#include "mqtt.h"
#include "tiny-json.h"

extern entity_changed_t entity_changed_cb ;

void switch_parser(Entity *e, u8_t* payload, u8_t length) {
    if (strncmp((char *)payload, "ON", 2) == 0) {
        e->onff = true ;
    } else {
        e->onff = false ;
    }
}

void switch_json_parser(Entity *e, u8_t* payload, u8_t length) {
    enum { MAX_FIELDS = 4 } ;
    json_t pool[MAX_FIELDS] ;
    json_t const* root = json_create((char*) payload, pool, MAX_FIELDS) ;
    if (!root) {
        return ;
    }

    json_t const* onoff_p = json_getProperty(root, e->stateValueTemplate) ;
    if (!onoff_p) {
        return ;
    }

    char const* onoff = json_getValue(onoff_p) ;
    if (!onoff) {
        return ;
    }

    if (strncmp(onoff, "ON", 2) == 0) {
        e->onff = true ;
    } else {
        e->onff = false ;
    }
}

void light_json_parser(Entity *e, u8_t* payload, u8_t length) {
    enum { MAX_FIELDS = 16 } ;
    json_t pool[MAX_FIELDS] ;
    json_t const* root = json_create((char*) payload, pool, MAX_FIELDS) ;
    if (!root) {
        return ;
    }

    json_t const* onoff_p = json_getProperty(root, e->stateValueTemplate) ;
    if (onoff_p) {
        char const* onoff = json_getValue(onoff_p) ;
        if (onoff) {
            if (strncmp(onoff, "ON", 2) == 0) {
                e->onff = true ;
            } else {
                e->onff = false ;
            }
        }
    }

    json_t const* br_p = json_getProperty(root, e->brightnessValueTemplate) ;
    if (!br_p) {
        return ;
    }

    e->brightness = (int) json_getInteger(br_p) ;
}

void temperature_json_parser(Entity *e, u8_t* payload, u8_t length)  {
    enum { MAX_FIELDS = 8 } ;
    json_t pool[MAX_FIELDS] ;
    json_t const* root = json_create((char*) payload, pool, MAX_FIELDS) ;
    if (!root) {
        return ;
    }

    json_t const* ds_p = json_getProperty(root, "DS18B20") ;
    if (!ds_p) {
        return ;
    }

    json_t const* t_p = json_getProperty(ds_p, "Temperature") ;
    if (!t_p) {
        return ;
    }

    e->value = json_getReal(t_p) ;
}

void energy_json_parser(Entity *e, u8_t* payload, u8_t length)  {
    enum { MAX_FIELDS = 16 } ;
    json_t pool[MAX_FIELDS] ;
    json_t const* root = json_create((char*) payload, pool, MAX_FIELDS) ;
    if (!root) {
        return ;
    }

    json_t const* e_p = json_getProperty(root, "ENERGY") ;
    if (!e_p) {
        return ;
    }

    json_t const* v_p = json_getProperty(e_p, e->stateValueTemplate) ;
    if (!v_p) {
        return ;
    }

    e->value = json_getInteger(v_p) ;
    // printf("id: %d, val: %f\n", e->id, e->value) ;
}

void alarm_parser(Entity *e, u8_t* payload, u8_t length)  {
    strncpy(e->state, (char *) payload, length) ;
    e->state[length] = '\0' ;
}

typedef struct weather_pic_t_t {
    const char *icon ;
    int pic ;
} weather_pic_t ;

weather_pic_t weather_pm[] = {
    {"01d", 5},
    {"01n", 6},
    {"02d", 7},
    {"02n", 8},
    {"03d", 9},
    {"03n", 9},
    {"04d", 10},
    {"04n", 10},
    {"09d", 11},
    {"09n", 11},
    {"10d", 12},
    {"10n", 13},
    {"11d", 14},
    {"11n", 14},
    {"13d", 15},
    {"13n", 15},
    {"50d", 16},
    {"50n", 16}
} ;

void weather_parser(Entity *e, u8_t* payload, u8_t length) {
    memset(&e->weather, 0, sizeof(e->weather)) ;

    const int MAX_FIELDS = 40 ;
    json_t pool[MAX_FIELDS] ;

    json_t const* root = json_create((char*) payload, pool, MAX_FIELDS) ;
    if (!root) {
        return ;
    }

    json_t const* main_p = json_getProperty(root, "main") ;
    if (main_p) {
        json_t const* temp_p = json_getProperty(main_p, "temp") ;
        if (temp_p) {
            e->weather.temp =  json_getReal(temp_p) ;
        }

        json_t const* hum_p = json_getProperty(main_p, "humidity") ;
        if (hum_p) {
            e->weather.humidity =  json_getReal(hum_p) ;
        }

        json_t const* pres_p = json_getProperty(main_p, "pressure") ;
        if (pres_p) {
            e->weather.pressure =  json_getReal(pres_p) ;
        }
    }

    json_t const* wind_p = json_getProperty(root, "wind") ;
    if (wind_p) {
        json_t const* speed_p = json_getProperty(wind_p, "speed") ;
        if (speed_p) {
            e->weather.wind =  json_getReal(speed_p) ;
        }
    }

    json_t const* wea_p = json_getProperty(root, "weather") ;
    if (wea_p) {
        json_t const* wea1_p = json_getChild(wea_p) ;
        if (wea1_p) {
            json_t const* icon_p = json_getProperty(wea1_p, "icon") ;
            if (icon_p) {
                const char *icon = json_getValue(icon_p) ;
                for (weather_pic_t wp : weather_pm) {
                    if (strcmp(icon, wp.icon) == 0) {
                        e->weather.icon = wp.pic ;
                        break ;
                    }
                }
            }
        }
    }
}

Entity entities[] = {
    // Cabinet
    {11, "cabinet/stat/toplight/POWER",  "cabinet/cmnd/toplight/POWER",  switch_parser}, // TopLight
    {12, "cabinet/stat/midesk/RESULT", "POWER", "cabinet/cmnd/midesk/POWER", "cabinet/cmnd/midesk/DIMMER", "Dimmer", light_json_parser}, // MiLamp
    {13, "hall/stat/sofa/POWER", "hall/cmnd/sofa/POWER", switch_parser}, // TableLamp
    {14, "cabinet/stat/modlamp/POWER",  "cabinet/cmnd/modlamp/POWER",  switch_parser}, // ModLamp

    // Hall
    {21, "cabinet/stat/tablelamp/POWER", "cabinet/cmnd/tablelamp/POWER", switch_parser}, // Sofa
    {22, "hall/stat/luster/POWER1", "hall/cmnd/luster/POWER1", switch_parser},
    {23, "hall/stat/luster/POWER2", "hall/cmnd/luster/POWER2", switch_parser},
    {24, "cabinet/stat/backlight/RESULT", "POWER", "cabinet/cmnd/backlight/POWER", "cabinet/cmnd/backlight/DIMMER", "Dimmer", light_json_parser},

    // Corridor
    {41, "corridor/stat/switch34/POWER1", "corridor/cmnd/switch34/POWER1", switch_parser}, // Light
    {42, "corridor/stat/switch12/POWER1", "corridor/cmnd/switch12/POWER1", switch_parser}, // PassLight
    {43, "corridor/stat/wardrobe/RESULT", "POWER", "corridor/cmnd/wardrobe/POWER", "corridor/cmnd/wardrobe/DIMMER", "Dimmer", light_json_parser},
    {44, "hall/stat/outlet2/POWER", "hall/cmnd/outlet2/POWER", switch_parser}, // Blackout

    // Bathroom
    {51, "bathroom/stat/bathlight/POWER", "bathroom/cmnd/bathlight/POWER", switch_parser},
    {52, "bathroom/stat/bathfan/POWER", "bathroom/cmnd/bathfan/POWER", switch_parser},
    {53, "bathroom/tele/bathboiler/SENSOR", energy_json_parser, "Voltage"},
    {54, "bathroom/tele/bathboiler/SENSOR", energy_json_parser, "Power"},

    // Kitchen
    {61, "kitchen/stat/kitchswitch123/POWER1", "kitchen/cmnd/kitchswitch123/POWER1", switch_parser}, // Luster
    {62, "kitchen/stat/kitchenlight/POWER", "kitchen/cmnd/kitchenlight/POWER", switch_parser}, // CornerLight
    {63, "kitchen/stat/kitchworkzone/POWER", "kitchen/cmnd/kitchworkzone/POWER", switch_parser}, // Workzone
    {64, "kitchen/stat/kitchenfan/POWER", "kitchen/cmnd/kitchenfan/POWER", switch_parser}, // Fan
    {65, "cabinet/stat/outlet1/POWER",   "cabinet/cmnd/outlet1/POWER",   switch_parser}, // Blackout

    // Lady
    {71, "lady/stat/ladylight/POWER1", "lady/cmnd/ladylight/POWER1", switch_parser},
    {72, "lady/stat/ladylight/POWER2", "lady/cmnd/ladylight/POWER2", switch_parser},

    // Weather
    {100, "gobbit/tele/5/RAW", weather_parser}
} ;

void best_mqtt_e_connected() {
    for (Entity e : entities) {
        // printf("MQTT_E: Subscribing to %s\n", e.stateTopic) ;
        best_mqtt_subscribe(e.stateTopic) ;
    }
}

void best_mqtt_e_message_received(const char *topic, u8_t *buffer, u8_t len) {
    // printf("Message received:: %s : %s\n", topic, buffer) ;
    for (int i = 0; i < (int)(sizeof(entities) / sizeof(Entity)); i++) {
        Entity *e = &entities[i] ;

        if (strcmp(topic, e->stateTopic) != 0) {
            continue ;
        }

        if (e->parser == NULL) {
            continue ;
        }

        e->parser(e, buffer, len) ;

        if (entity_changed_cb) {
            entity_changed_cb(e) ;
        }
    }
}

Entity* get_entity_by_id(uint8_t id) {
    for (int i = 0; i < (int)(sizeof(entities) / sizeof(Entity)); i++) {
        Entity *e = &entities[i] ;
        if (e->id == id) {
            return e ;
        }
    }
    return nullptr ;
}