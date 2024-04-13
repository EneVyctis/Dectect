#include "esp_wifi.h"

#include "utils/collections.h"

extern MAC_address_hashmap detectedMacAddresses;
extern pri_hashset detectedProbeRequests;

void init_wifi_drivers();
void init_sniffer();
void wifi_start_sniffer();
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type);


