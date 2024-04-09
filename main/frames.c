#include<stdio.h>
#include "frames.h"

void getMacStr(char* macStr, struct MAC_address* mac_addr)
{
	sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac_addr->addr[0],
		mac_addr->addr[1],
		mac_addr->addr[2],
		mac_addr->addr[3],
		mac_addr->addr[4],
		mac_addr->addr[5]);
}