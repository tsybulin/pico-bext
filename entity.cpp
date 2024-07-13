#include "entity.h"

// JSON Switch
Entity::Entity(const uint8_t id, const char *stateTopic, const char *stateValueTemplate, const char *commandTopic, entity_parser_t parser)
: id(id)
{
    this->stateTopic = stateTopic ;
    this->stateValueTemplate = stateValueTemplate ;
    this->commandTopic = commandTopic ;
    this->parser = parser ;
    this->dimmable = false ;
    this->brightness = 100 ;
    this->brightnessCommandTopic = NULL ;
    this->brightnessValueTemplate = NULL ;
    this->value = 0.0 ;
    this->state[0] = '\0' ;
}

// Switch
Entity::Entity(const uint8_t id, const char *stateTopic, const char *commandTopic, entity_parser_t parser) 
: Entity(id, stateTopic, NULL, commandTopic, parser) 
{
}

// Light
Entity::Entity(const uint8_t id,
                const char *stateTopic, const char *stateValueTemplate,
                const char *commandTopic, 
                const char *brightnessCommandTopic, const char *brightnessValueTemplate,
                entity_parser_t parser)
: Entity(id, stateTopic, stateValueTemplate, commandTopic, parser)
{
    this->brightnessCommandTopic = brightnessCommandTopic ;
    this->brightnessValueTemplate = brightnessValueTemplate ;
    this->dimmable = true ;
    this->brightness = 0 ;
}

// Sensor
Entity::Entity(const uint8_t id, const char *stateTopic, entity_parser_t parser, const char *valueTemplate)
: Entity(id, stateTopic, NULL, parser)
{
    this->stateValueTemplate = valueTemplate ;
}
