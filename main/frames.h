#ifndef FRAMES_H_GUARD
#define FRAMES_H_GUARD

#include<stdint.h>
#include "esp_wifi_types.h"
#include "esp_mac.h"

#include "utils/collections.h"

#define MAC_ADDR_STR_LEN 18

enum TAG_NUMBERS
{
	SSID = 0,
	SUPPORTED_RATES = 1,
	HT_CAPABILITIES = 45,
	EXTENDED_SUPPORTED_RATES = 50,
	VENDOR_SPECIFIC = 50,
};

struct frame_control
{
	uint16_t protocol_version :2;
	uint16_t type :2;
	uint16_t subtype :4;
	uint16_t toDs:1;
	uint16_t fromDs:1;
	uint16_t moreFragments:1;
	uint16_t retry:1;
	uint16_t powerMgmt:1;
	uint16_t moreData:1;
	uint16_t protecteFrame:1;
	uint16_t HTCOrder:1;
};

struct MAC_address
{
	uint8_t addr[6];
};

struct frame
{
	struct frame_control frame_control;
	uint16_t duration;
	struct MAC_address address1;
	struct MAC_address address2;
	struct MAC_address address3;
};

struct probe_request
{
	struct frame_control frame_control;
	uint16_t duration;

	struct MAC_address destination_address;
	struct MAC_address source_address;
	struct MAC_address BSSID;

	uint16_t fragment_sequence_number;
};

struct Tag
{
	uint8_t tag_number;
	uint8_t tag_length;
	uint8_t* values;
};


struct probe_request_identifier
{
#ifdef USE_OUI
	uint8_t OUI[3];
#endif
	struct Tag supported_rates;
	struct Tag extended_supported_rates;
	struct Tag ht_capabilities;
};

void getMacStr(char* macStr, struct MAC_address* mac_addr);

void read_probe_request_frame(wifi_promiscuous_pkt_t* packet, struct probe_request* pr, TAG_lst* lst);
void probe_request_identifier_destroy(struct probe_request_identifier* pri);

#endif