#ifndef FRAMES_H_GUARD
#define FRAMES_H_GUARD

#include<stdint.h>

#define MAC_ADDR_STR_LEN 18

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

void getMacStr(char* macStr, struct MAC_address* mac_addr);


#endif