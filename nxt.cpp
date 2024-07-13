#include "nxt.h"

#include "mqtt.h"

typedef struct cell_description_t_ {
    uint8_t button_id ;
    uint8_t text_id ;
    uint8_t gauge_id ;
    uint8_t value_id ;
} cell_description_t ;

#define FIRST_CELL_ID 24

extern Entity* get_entity_by_id(uint8_t id) ;

cell_description_t cell_description[] = {
    {FIRST_CELL_ID +0, 25, 32, 36},
    {FIRST_CELL_ID +2, 27, 33, 37},
    {FIRST_CELL_ID +4, 29, 34, 38},
    {FIRST_CELL_ID +6, 31, 35, 39}
} ;

int button_description[] = {1, 2, 3, 4, 5, 6, 7, 8} ;

Nxt::Nxt() {
}

void nxt_buttonDidTouch(void *arg1, void *arg2) {
    ((Nxt *)arg1)->buttonDidTouch((NexComp *)arg2) ;
}

void nxt_cellDidTouch(void *arg1, void *arg2) {
    ((Nxt *)arg1)->cellDidTouch((NexComp *)arg2) ;
}

void Nxt::begin(uart_inst_t *nexSer, uint32_t nexBaud) {
    this->nc.begin(nexSer, nexBaud) ;
    // this->nc.addDebug(uart0) ;

    for (int i = 0; i < NUM_BUTTONS; i++) {
        NexComp *btn = new NexComp(this->nc, 0, button_description[i]) ;
        btn->setOnTouch(nxt_buttonDidTouch, this, btn) ;
    }

    for (int i = 0; i < NUM_CELLS; i++) {
        NexComp *btn = new NexComp(this->nc, 0, cell_description[i].button_id) ;
        btn->setOnTouch(nxt_cellDidTouch, this, btn) ;
        this->cells[i].button = btn ;
        this->cells[i].text = new NexComp(this->nc, 0, cell_description[i].text_id) ;
        this->cells[i].gauge = new NexComp(this->nc, 0, cell_description[i].gauge_id) ;
        this->cells[i].value = new NexComp(this->nc, 0, cell_description[i].value_id) ;

    }

    NexComp *btn = new NexComp(this->nc, 1, 1) ;
    btn->setOnTouch([](void *arg1, void *arg2) {
        ((Nxt *)arg1)->buttonDidTouch((NexComp *)arg2) ;
    }, this, btn) ;

    this->main.temp = new NexComp(this->nc, 0, 19) ;
    this->main.hum = new NexComp(this->nc, 0, 20) ;
    this->main.press = new NexComp(this->nc, 0, 21) ;
    this->main.wind = new NexComp(this->nc, 0, 22) ;
    this->main.icon = new NexComp(this->nc, 0, 23) ;

    // Home
    this->cell_configs[0][0] = cell_config_t{21, "ДИВАН", 19, 20} ;
    this->cell_configs[0][1] = cell_config_t{41, "ПРИХОЖАЯ", 35, 36} ;
    this->cell_configs[0][2] = cell_config_t{22, "СТОЛ", 23, 24} ;
    this->cell_configs[0][3] = cell_config_t{23, "ЛЮСТРА", 23, 24} ;

    // Cabinet
    this->cell_configs[1][0] = cell_config_t{11, "ВЕРХНИЙ", 23, 24} ;
    this->cell_configs[1][1] = cell_config_t{12, "МИ", 25, 26} ;
    this->cell_configs[1][2] = cell_config_t{13, "СТОЛ", 19, 20} ;
    this->cell_configs[1][3] = cell_config_t{14, "MOD", 19, 20} ;

    // Hall
    this->cell_configs[2][0] = cell_config_t{21, "ДИВАН", 19, 20} ;
    this->cell_configs[2][1] = cell_config_t{22, "СТОЛ", 23, 24} ;
    this->cell_configs[2][2] = cell_config_t{24, "АВАРИЙКА", 17, 18} ;
    this->cell_configs[2][3] = cell_config_t{23, "ЛЮСТРА", 23, 24} ;

    // Corridor
    this->cell_configs[3][0] = cell_config_t{44, "АВАРИЙКА", 29, 30} ;
    this->cell_configs[3][1] = cell_config_t{41, "ПРИХОЖАЯ", 35, 36} ;
    this->cell_configs[3][2] = cell_config_t{43, "ШКАФ", 35, 36} ;
    this->cell_configs[3][3] = cell_config_t{42, "ПРОХОД", 35, 36} ;

    // Bathroom
    this->cell_configs[4][0] = cell_config_t{51, "СВЕТ", 33, 34} ;
    this->cell_configs[4][1] = cell_config_t{52, "ВЕНТИЛЯЦИЯ", 27, 28} ;
    this->cell_configs[4][2] = cell_config_t{53, "Сеть V", 31, 32, 190, 250} ;
    this->cell_configs[4][3] = cell_config_t{54, "Бойлер W", 31, 32, 0, 2000} ;

    // Kitchen
    this->cell_configs[5][0] = cell_config_t{61, "СВЕТ", 23, 24} ;
    this->cell_configs[5][1] = cell_config_t{64, "ВЕНТИЛЯЦИЯ", 27, 28} ;
    this->cell_configs[5][2] = cell_config_t{63, "РАБЗОНА", 35, 36} ;
    this->cell_configs[5][3] = cell_config_t{65, "АВАРИЙКА", 29, 30} ;

    //Lady
    this->cell_configs[6][0] = cell_config_t{71, "Ближний", 23, 24} ;
    this->cell_configs[6][1] = cell_config_t{72, "Дальний", 23, 24} ;
    this->cell_configs[6][2] = cell_config_t{0} ;
    this->cell_configs[6][3] = cell_config_t{0} ;
}

void Nxt::start() {
    this->current_page = 0 ;
    this->nc.cmdWrite("page 0") ;
    this->displayPage() ;
    this->nc.cmdWrite("dim=5") ;
}

int32_t gaugeValue(int32_t minValue, int32_t value, int32_t maxValue) {
	const double minDeg = 330 ;
	const double maxDeg = 210 ;

	int32_t val = value > minValue ? value : minValue ;
    val = val > maxValue ? maxValue : value ;

	double result = minDeg + (maxDeg + (360.0 - minDeg)) * ((double)(val - minValue) / (double)(maxValue - minValue)) ;
    while (result > 360) {
        result -= 360 ;
    }

	return result ;
}

void Nxt::displayPage() {
    for (int c = 0; c < NUM_CELLS; c++) {
        if (this->cell_configs[this->current_page][c].entity_id > 0) {
            this->cells[c].text->setAttr("txt", this->cell_configs[this->current_page][c].title) ;
            this->cells[c].text->setVisible(true) ;

            this->cells[c].gauge->setVisible(false) ;
            this->cells[c].value->setVisible(false) ;

            Entity *e = get_entity_by_id(this->cell_configs[this->current_page][c].entity_id) ;
            if (e) {
                if (e->brightnessValueTemplate) {
                    this->cells[c].value->setAttr("val", e->brightness * 10) ;
                    this->cells[c].value->setVisible(true) ;
                }

                if (e->commandTopic) {
                    this->cells[c].button->setAttr("pic", e->onff ? this->cell_configs[this->current_page][c].icon_on : this->cell_configs[this->current_page][c].icon_off) ;
                    this->cells[c].button->setAttr("pic2", this->cell_configs[this->current_page][c].icon_on) ;
                    this->cells[c].button->setVisible(true) ;
                } else {
                    this->cells[c].button->setVisible(false) ;

                    if (this->cell_configs[this->current_page][c].max_value != 0) {
                        this->cells[c].gauge->setAttr("val", gaugeValue(this->cell_configs[this->current_page][c].min_value, e->value, this->cell_configs[this->current_page][c].max_value)) ;
                        this->cells[c].gauge->setVisible(true) ;

                        this->cells[c].value->setAttr("val", e->value * 10) ;
                        this->cells[c].value->setVisible(true) ;
                    }
                }
            }
        } else {
            this->cells[c].button->setVisible(false) ;
            this->cells[c].text->setVisible(false) ;
            this->cells[c].gauge->setVisible(false) ;
            this->cells[c].value->setVisible(false) ;
        }

        Entity *e = get_entity_by_id(100) ;
        if (e && e->weather.icon > 0) {
            this->entityDidChange(e) ;
        }
    }
}

void Nxt::buttonDidTouch(NexComp *btn) {
    if ((btn->getGuid() && 0xFF) == 2) {
        this->current_page = 0 ;
    } else {
        this->current_page = (btn->getGuid() >> 8) -1 ;
    }

    this->displayPage() ;
#ifndef NDEBUG
    printf("current page %d\n", this->current_page) ;
#endif
}

void Nxt::cellDidTouch(NexComp *btn) {
    int cell_id = ((btn->getGuid() >> 8) - FIRST_CELL_ID) >> 1 ;
#ifndef NDEBUG
    printf("touched cell %d\n", cell_id) ;
#endif
    uint8_t entity_id = this->cell_configs[this->current_page][cell_id].entity_id ;
    if (entity_id > 0) {
        Entity *e = get_entity_by_id(entity_id) ;
        if (e && e->commandTopic) {
            best_mqtt_publish(e->commandTopic, e->onff ? "OFF" : "ON") ;
        }
    }
}

void Nxt::loop() {
    this->nc.loop() ;
}

void Nxt::entityDidChange(Entity *e) {
    if (e->id == 100) {
        this->main.temp->setAttr("val", e->weather.temp) ;
        this->main.hum->setAttr("val", e->weather.humidity) ;
        this->main.press->setAttr("val", e->weather.pressure) ;
        this->main.wind->setAttr("val", e->weather.wind) ;
        this->main.icon->setAttr("pic", e->weather.icon) ;
        return ;
    }

    for (int c = 0; c < NUM_CELLS; c++) {
        if (this->cell_configs[this->current_page][c].entity_id == e->id) {
            this->cells[c].button->setAttr("pic", e->onff ? this->cell_configs[this->current_page][c].icon_on : this->cell_configs[this->current_page][c].icon_off) ;

            if (e->brightnessValueTemplate) {
                this->cells[c].value->setAttr("val", e->brightness * 10) ;
            }

            if (this->cell_configs[this->current_page][c].max_value != 0) {
                this->cells[c].gauge->setAttr("val", gaugeValue(this->cell_configs[this->current_page][c].min_value, e->value, this->cell_configs[this->current_page][c].max_value)) ;
                this->cells[c].value->setAttr("val", e->value * 10) ;
            }
        }
    }
}
