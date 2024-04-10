#include<esp_log.h>

#include "utils/collections.h"
#include "frames.h"
#include "sniffer.h"


static const char *TAG = "sniffer";

static MAC_address_hashset detectedMacAddresses;
static pri_hashset detectedProbeRequests;



static void parse_pri(struct probe_request* pr, TAG_lst* tagLst, struct probe_request_identifier* pri)
{

#ifdef USE_OUI
	pri->OUI[0] = pr->source_address.addr[0];
	pri->OUI[1] = pr->source_address.addr[1];
	pri->OUI[2] = pr->source_address.addr[2];
#endif

	pri->supported_rates.tag_length = 0;
	pri->supported_rates.values = NULL;

	pri->extended_supported_rates.tag_length = 0;
	pri->extended_supported_rates.values = NULL;

	pri->ht_capabilities.tag_length = 0;
	pri->ht_capabilities.values = NULL;
	
	for(int i=0; i<tagLst->length; i++)
	{
		switch(tagLst->content[i].tag_number)
		{
			case SUPPORTED_RATES:
				pri->supported_rates.tag_number = SUPPORTED_RATES;
				pri->supported_rates.tag_length = tagLst->content[i].tag_length;
				pri->supported_rates.values = tagLst->content[i].values;
				break;

			case EXTENDED_SUPPORTED_RATES:
				pri->extended_supported_rates.tag_number = EXTENDED_SUPPORTED_RATES;
				pri->extended_supported_rates.tag_length = tagLst->content[i].tag_length;
				pri->extended_supported_rates.values = tagLst->content[i].values;
				break;

			case HT_CAPABILITIES:
				pri->ht_capabilities.tag_number = HT_CAPABILITIES;
				pri->ht_capabilities.tag_length = tagLst->content[i].tag_length;
				pri->ht_capabilities.values = tagLst->content[i].values;
				break;



			default :
				free(tagLst->content[i].values);

		}

	}
}






void init_sniffer()
{  
	MAC_address_hashset_init(&detectedMacAddresses);
	pri_hashset_init(&detectedProbeRequests);

	wifi_promiscuous_filter_t filter= {.filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_CTRL};
	wifi_promiscuous_filter_t filterCtrl = {.filter_mask=WIFI_PROMIS_CTRL_FILTER_MASK_RTS};

	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_ctrl_filter(&filterCtrl));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(sniffer));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
}



void sniffer(void* buf, wifi_promiscuous_pkt_type_t type)
{
	wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*)buf;
	struct frame* frame;


	switch (type)
	{
		case WIFI_PKT_MGMT:
			frame = (struct frame*) &packet->payload;

			if(frame->frame_control.subtype == 4)
			{
				
				struct probe_request pr;
				TAG_lst tagLst;
				TAG_lst_initialise(&tagLst);
				read_probe_request_frame(packet, &pr, &tagLst);

				struct probe_request_identifier pri;

				parse_pri(&pr, &tagLst, &pri);

				struct probe_request_identifier* res = pri_hashset_insert(&detectedProbeRequests, &pri);

				if(res == NULL)
					probe_request_identifier_destroy(&pri);
				else
				{
					ESP_LOGI(TAG, "new probe request : %ld", detectedProbeRequests.objCount);

					if(detectedProbeRequests.objCount % 10 == 0)
					{
						pri_hashset_iterator it;
						pri_hashset_init_iterator(&detectedProbeRequests, &it);
						
						struct probe_request_identifier* ppp;

						ESP_LOGI(TAG, "All : ");


						while(pri_hashset_iterator_has_next(&it))
						{
							ppp = pri_hashset_iterator_next(&it);

#ifdef USE_OUI
							if(ppp->supported_rates.tag_length == 8)
								ESP_LOGI(TAG, "\tOUI : %02x:%02x:%02x, SR : length %d, %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", ppp->OUI[0], ppp->OUI[1], ppp->OUI[2], ppp->supported_rates.tag_length, ppp->supported_rates.values[0], ppp->supported_rates.values[1], ppp->supported_rates.values[2], ppp->supported_rates.values[3], ppp->supported_rates.values[4], ppp->supported_rates.values[5], ppp->supported_rates.values[6], ppp->supported_rates.values[7]);
							else if(ppp->supported_rates.tag_length)
								ESP_LOGI(TAG, "\tOUI : %02x:%02x:%02x, SR : length %d", ppp->OUI[0], ppp->OUI[1], ppp->OUI[2], ppp->supported_rates.tag_length);
#else
							if(ppp->supported_rates.tag_length == 8)
								ESP_LOGI(TAG, "\tSR : length %d, %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", ppp->supported_rates.tag_length, ppp->supported_rates.values[0], ppp->supported_rates.values[1], ppp->supported_rates.values[2], ppp->supported_rates.values[3], ppp->supported_rates.values[4], ppp->supported_rates.values[5], ppp->supported_rates.values[6], ppp->supported_rates.values[7]);
							else if(ppp->supported_rates.tag_length)
								ESP_LOGI(TAG, "\tSR : length %d", ppp->supported_rates.tag_length);

#endif
						}
					}
				}

				

				TAG_lst_destroy(&tagLst);
			}
			break;
		case WIFI_PKT_CTRL:
			frame = (struct frame*) &packet->payload;

			//ESP_LOGI(TAG, "Ctrl : %d, subtype : %d", frame->frame_control.type, frame->frame_control.subtype);
			
			char str[MAC_ADDR_STR_LEN];
			struct MAC_address* madd = MAC_address_hashset_insert(&detectedMacAddresses, &frame->address2);
			if(madd != NULL)
			{
				getMacStr(str, madd);
				ESP_LOGI(TAG, "New mac address : %s, %ld", str, detectedMacAddresses.objCount);

				
				if(detectedMacAddresses.objCount % 5 == 0)
				{
					ESP_LOGI(TAG, " ");
					ESP_LOGI(TAG, " ");
					ESP_LOGI(TAG, "%ld Detected mac addresses in %ld buckets : ", detectedMacAddresses.objCount, detectedMacAddresses.bucketCount);
					
					MAC_address_hashset_iterator it; 
					MAC_address_hashset_init_iterator(&detectedMacAddresses, &it);

					while(MAC_address_hashset_iterator_has_next(&it))
					{
						getMacStr(str, MAC_address_hashset_iterator_next(&it));
						ESP_LOGI(TAG, "\t %s", str);
					}

					ESP_LOGI(TAG, " ");
				}
			}

			break;
		default:
			break;
	}
}