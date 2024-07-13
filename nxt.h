#pragma once

#include <stdio.h>
#include "pico/stdlib.h"

#include "NextionX.h"
#include "entity.h"

#define NUM_CELLS 4
#define NUM_BUTTONS 8

typedef struct cell_t_ {
    NexComp *button ;
    NexComp *text ;
    NexComp *gauge ;
    NexComp *value ;
} cell_t ;

typedef struct cell_config_t_ {
    uint8_t entity_id = 0 ;
    const char *title ;
    uint8_t icon_off = 17 ;
    uint8_t icon_on = 18 ;
    int min_value = 0 ;
    int max_value = 0;

} cell_config_t ;

typedef struct main_t_ {
    NexComp *temp ;
    NexComp *hum ;
    NexComp *press ;
    NexComp *wind ;
    NexComp *icon ;
} main_t ;

class Nxt {
    public:
        Nxt() ;
        void begin(uart_inst_t *nexSer, uint32_t nexBaud) ;
        void buttonDidTouch(NexComp *btn) ;
        void cellDidTouch(NexComp *btn) ;
        void start() ;
        void loop() ;
        void entityDidChange(Entity *e) ;
    private:
        void displayPage() ;
        
        NexComm nc ;
        cell_t cells[NUM_CELLS] ;
        cell_config_t cell_configs[NUM_BUTTONS][NUM_CELLS] ;
        main_t main ;
        int current_page = 0;
} ;
