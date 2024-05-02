#include <esp_log.h>
#include <esp_timer.h>

#include "frames.h"
#include "sniffer.h"


static const char *TAG = "sniffer";

MAC_address_hashmap detectedMacAddresses;
pri_hashset detectedProbeRequests;
static MAC_address_hashset detectedAPs;


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

	pri->extended_capabilities.tag_length = 0;
	pri->extended_capabilities.values = NULL;
	
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

			case EXTENDED_CAPABILITIES:
				pri->extended_capabilities.tag_number = EXTENDED_CAPABILITIES;
				pri->extended_capabilities.tag_length = tagLst->content[i].tag_length;
				pri->extended_capabilities.values = tagLst->content[i].values;
				break;



			default :
				free(tagLst->content[i].values);

		}

	}
}



void init_wifi_drivers()
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
	ESP_ERROR_CHECK(esp_wifi_start());	
}


void init_sniffer()
{
	MAC_address_hashmap_init(&detectedMacAddresses);
	MAC_address_hashset_init(&detectedAPs);
	pri_hashset_init(&detectedProbeRequests);
}

void wifi_start_sniffer()
{
	wifi_promiscuous_filter_t filter= {.filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_CTRL | WIFI_PROMIS_FILTER_MASK_DATA};
	wifi_promiscuous_filter_t filterCtrl = {.filter_mask=WIFI_PROMIS_CTRL_FILTER_MASK_RTS};

	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_ctrl_filter(&filterCtrl));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(sniffer));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
}



void sniffer(void* buf, wifi_promiscuous_pkt_type_t type)
{
	wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*)buf;
	struct frame* frame = (struct frame*) &packet->payload;
	char str[MAC_ADDR_STR_LEN];

	switch (type)
	{
		case MANAGEMENT:

			switch(frame->frame_control.subtype)
			{
				case MANAGEMENT_PROBE_REQUEST:	

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
						ESP_LOGI(TAG, "new probe request : %ld", detectedProbeRequests.objCount);
					

					TAG_lst_destroy(&tagLst);
					break;

				case MANAGEMENT_BEACON:
					struct MAC_address* madd = MAC_address_hashset_insert(&detectedAPs, &frame->address2);
					
					if(madd != NULL)
					{
						getMacStr(str, madd);
						ESP_LOGI(TAG, "New access point : %s, %ld", str, detectedAPs.objCount);

					}
					break;

			}
			break;



		case CONTROL:

			bool isNewMacAddress = false;
			int64_t time = esp_timer_get_time();

			struct MAC_address_hashmap_entry* madd = MAC_address_hashmap_insert_or_modify(&detectedMacAddresses, &frame->address2, &time, &isNewMacAddress);
			if(isNewMacAddress)
			{
				getMacStr(str, &madd->key);
				ESP_LOGI(TAG, "New RTS mac address : %s, %ld", str, detectedMacAddresses.objCount);
			}

			break;
		
		
		case DATA:
			if(!MAC_address_hashset_contains(&detectedAPs, &frame->address2))
			{
				bool isNewMacAddress = false;
				int64_t time = esp_timer_get_time();
				struct MAC_address_hashmap_entry* madd = MAC_address_hashmap_insert_or_modify(&detectedMacAddresses, &frame->address2, &time, &isNewMacAddress);
				if(isNewMacAddress)
				{
					getMacStr(str, &madd->key);
					ESP_LOGI(TAG, "New data mac address : %s, %ld", str, detectedMacAddresses.objCount);
				}
			}
			break;
		default:
			break;
	}
}