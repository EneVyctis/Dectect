#ifndef FRAMES_H_GUARD
#define FRAMES_H_GUARD

#include<stdint.h>
#include "esp_wifi_types.h"
#include "esp_mac.h"

#define MAC_ADDR_STR_LEN 18

enum FrameType 
{
	MANAGEMENT = 0x00,
	CONTROL = 0x01,
	DATA = 0x02,
	EXTENSION = 0x03
};

enum ManagementSubtype 
{
	MANAGEMENT_ASSOCIATION_REQUEST = 0x00,
	MANAGEMENT_ASSOCIATION_RESPONSE = 0x01,
	MANAGEMENT_REASSOCIATION_REQUEST = 0x02,
	MANAGEMENT_REASSOCIATION_RESPONSE = 0x03,
	MANAGEMENT_PROBE_REQUEST = 0x04,
	MANAGEMENT_PROBE_RESPONSE = 0x05,
	MANAGEMENT_TIMING_ADVERTISEMENT = 0x06,
	MANAGEMENT_RESERVED_MANAGEMENT = 0x07,
	MANAGEMENT_BEACON = 0x08,
	MANAGEMENT_ATIM = 0x09,
	MANAGEMENT_DISASSOCIATION = 0x0A,
	MANAGEMENT_AUTHENTICATION = 0x0B,
	MANAGEMENT_DEAUTHENTICATION = 0x0C,
	MANAGEMENT_ACTION = 0x0D,
	MANAGEMENT_ACTION_NO_ACK = 0x0E,
	MANAGEMENT_RESERVED_MANAGEMENT_2 = 0x0F
};

enum ControlSubtype 
{
	CONTROL_RESERVED_CONTROL_1 = 0x00,
	CONTROL_TRIGGER = 0x02,
	CONTROL_TACK = 0x03,
	CONTROL_BEAMFORMING_REPORT_POLL = 0x04,
	CONTROL_VHT_HE_NDP_ANNOUNCEMENT = 0x05,
	CONTROL_CONTROL_FRAME_EXTENSION = 0x06,
	CONTROL_CONTROL_WRAPPER = 0x07,
	CONTROL_BLOCK_ACK_REQUEST = 0x08,
	CONTROL_BLOCK_ACK = 0x09,
	CONTROL_PS_POLL = 0x0A,
	CONTROL_RTS = 0x0B,
	CONTROL_CTS = 0x0C,
	CONTROL_ACK = 0x0D,
	CONTROL_CF_END = 0x0E,
	CONTROL_CF_END_CF_ACK = 0x0F
};

enum DataSubtype 
{
	DATA_DATA = 0x00,
	DATA_RESERVED_DATA_1 = 0x01,
	DATA_NULL_DATA = 0x04,
	DATA_RESERVED_DATA_2 = 0x05,
	DATA_QOS_DATA = 0x08,
	DATA_QOS_DATA_CF_ACK = 0x09,
	DATA_QOS_DATA_CF_POLL = 0x0A,
	DATA_QOS_DATA_CF_ACK_CF_POLL = 0x0B,
	DATA_QOS_NULL_DATA = 0x0C,
	DATA_RESERVED_DATA_3 = 0x0D,
	DATA_QOS_CF_POLL_NO_DATA = 0x0E,
	DATA_QOS_CF_ACK_CF_POLL_NO_DATA = 0x0F
};

enum ExtensionSubtype 
{
	EXTENSION_DMG_BEACON = 0x00,
	EXTENSION_S1G_BEACON = 0x01,
	EXTENSION_RESERVED_EXTENSION = 0x02
};

enum TAG_NUMBERS
{
	SSID = 0,
	SUPPORTED_RATES = 1,
	HT_CAPABILITIES = 45,
	EXTENDED_SUPPORTED_RATES = 50,
	EXTENDED_CAPABILITIES = 127,
	VENDOR_SPECIFIC = 255,
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
	struct Tag extended_capabilities;
	
};


struct TAG_lst;
typedef struct TAG_lst TAG_lst;

void getMacStr(char* macStr, struct MAC_address* mac_addr);

void read_probe_request_frame(wifi_promiscuous_pkt_t* packet, struct probe_request* pr, TAG_lst* lst);
void probe_request_identifier_destroy(struct probe_request_identifier* pri);

#endif