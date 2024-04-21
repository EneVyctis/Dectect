#ifndef WIFI_H_GUARD
#define WIFI_H_GUARD

#include "esp_wifi.h"


void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init();
void wifi_init_sta(wifi_config_t* wifi_config);
#endif