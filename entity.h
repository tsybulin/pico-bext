#pragma once

#include "pico/stdlib.h"
#include "lwip/arch.h"

struct Entity ;

typedef void (*entity_parser_t)(Entity*, u8_t*, u8_t) ;
typedef void (*entity_changed_t)(Entity*) ;


typedef struct weather_t_ {
    int temp ;
    int humidity ;
    int pressure ;
    int wind ;
    int icon ;
} weather_t ;


struct Entity {
    const uint8_t id ;
    const char *stateTopic ;
    const char *stateValueTemplate ;
    const char *commandTopic ;
    bool onff = false ;
    entity_parser_t parser ;

    bool dimmable ;
    int brightness ;
    const char *brightnessCommandTopic ;
    const char *brightnessValueTemplate ;

    double value ;
    char state[24] ;

    weather_t weather ;

    public:
        Entity(const uint8_t id, const char *stateTopic, const char *stateValueTemplate, const char *commandTopic, entity_parser_t parser) ;
        Entity(const uint8_t id, const char *stateTopic, const char *commandTopic = NULL, entity_parser_t parser = NULL) ;
        Entity(const uint8_t id,
               const char *stateTopic, const char *stateValueTemplate,
               const char *commandTopic, 
               const char *brightnessCommandTopic, const char *brightnessValueTemplate,
               entity_parser_t parser) ;
        Entity(const uint8_t id, const char *stateTopic, entity_parser_t parser, const char *valueTemplate = NULL) ;
} ;
