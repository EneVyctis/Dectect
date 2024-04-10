#include<stdlib.h>
#include<string.h>
#include<stdio.h>

#include "frames.h"
#include "utils/collections.h"

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



void read_probe_request_frame(wifi_promiscuous_pkt_t* packet, struct probe_request* pr, TAG_lst* lst)
{
	memcpy(pr, packet->payload, sizeof(*pr));
	int current = sizeof(*pr);
	while(current < packet->rx_ctrl.sig_len - sizeof(uint32_t))
	{
		struct Tag* tag = TAG_lst_insert(lst);
		tag->tag_number = packet->payload[current];
		tag->tag_length = packet->payload[current + 1];
		tag->values = malloc(tag->tag_length * sizeof(uint8_t));

		memcpy(tag->values, &packet->payload[current + 2], tag->tag_length);

		current = current + 2 + tag->tag_length; 
	}
}



void probe_request_identifier_destroy(struct probe_request_identifier* pri)
{
	if(pri->supported_rates.tag_length > 0 || pri->supported_rates.values != NULL)
		free(pri->supported_rates.values);

	if(pri->extended_supported_rates.tag_length > 0 || pri->extended_supported_rates.values != NULL)
		free(pri->extended_supported_rates.values);
	
	if(pri->ht_capabilities.tag_length > 0 || pri->ht_capabilities.values != NULL)
		free(pri->ht_capabilities.values);
		
}

