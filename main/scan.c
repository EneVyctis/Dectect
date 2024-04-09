/* Scan Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
	This example shows how to scan for available set of APs.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <pthread.h>
#include "frames.h"
#include "utils/collections.h"

#define DEFAULT_SCAN_LIST_SIZE CONFIG_EXAMPLE_SCAN_LIST_SIZE

static const char *TAG = "sniffer";



static void print_auth_mode(int authmode)
{
	switch (authmode) {
	case WIFI_AUTH_OPEN:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OPEN");
		break;
	case WIFI_AUTH_OWE:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_OWE");
		break;
	case WIFI_AUTH_WEP:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WEP");
		break;
	case WIFI_AUTH_WPA_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_PSK");
		break;
	case WIFI_AUTH_WPA2_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_PSK");
		break;
	case WIFI_AUTH_WPA_WPA2_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
		break;
	case WIFI_AUTH_ENTERPRISE:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_ENTERPRISE");
		break;
	case WIFI_AUTH_WPA3_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_PSK");
		break;
	case WIFI_AUTH_WPA2_WPA3_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
		break;
	case WIFI_AUTH_WPA3_ENT_192:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_ENT_192");
		break;
	case WIFI_AUTH_WPA3_EXT_PSK:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_EXT_PSK");
		break;
	case WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE");
		break;
	default:
		ESP_LOGI(TAG, "Authmode \tWIFI_AUTH_UNKNOWN");
		break;
	}
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
	switch (pairwise_cipher) {
	case WIFI_CIPHER_TYPE_NONE:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;
	case WIFI_CIPHER_TYPE_WEP40:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
	case WIFI_CIPHER_TYPE_WEP104:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
	case WIFI_CIPHER_TYPE_TKIP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
	case WIFI_CIPHER_TYPE_CCMP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;
	case WIFI_CIPHER_TYPE_TKIP_CCMP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;
	case WIFI_CIPHER_TYPE_AES_CMAC128:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_AES_CMAC128");
		break;
	case WIFI_CIPHER_TYPE_SMS4:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_SMS4");
		break;
	case WIFI_CIPHER_TYPE_GCMP:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP");
		break;
	case WIFI_CIPHER_TYPE_GCMP256:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_GCMP256");
		break;
	default:
		ESP_LOGI(TAG, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}

	switch (group_cipher) {
	case WIFI_CIPHER_TYPE_NONE:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;
	case WIFI_CIPHER_TYPE_WEP40:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
	case WIFI_CIPHER_TYPE_WEP104:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
	case WIFI_CIPHER_TYPE_TKIP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
	case WIFI_CIPHER_TYPE_CCMP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;
	case WIFI_CIPHER_TYPE_TKIP_CCMP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;
	case WIFI_CIPHER_TYPE_SMS4:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_SMS4");
		break;
	case WIFI_CIPHER_TYPE_GCMP:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP");
		break;
	case WIFI_CIPHER_TYPE_GCMP256:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_GCMP256");
		break;
	default:
		ESP_LOGI(TAG, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}
}


void printThreadName()
{
	char *tName = pcTaskGetName(xTaskGetCurrentTaskHandle());
	ESP_LOGI(TAG, "thread : %s", tName);
}

MAC_address_hashset detectedMacAddresses;

static void sniffer(void* buf, wifi_promiscuous_pkt_type_t type)
{
	wifi_promiscuous_pkt_t* packet = (wifi_promiscuous_pkt_t*)buf;
	struct frame* frame;


	switch (type)
	{
		case WIFI_PKT_MGMT:
			frame = (struct frame*) &packet->payload;
			if(frame->frame_control.subtype != 4)
			{

				struct probe_request pr;
				TAG_lst tagLst;
				TAG_lst_initialise(&tagLst);
				read_probe_request_frame(packet, &pr, &tagLst);

				char str[MAC_ADDR_STR_LEN];
				getMacStr(str, &pr.source_address);


				ESP_LOGI(TAG, "Probe request from %s", str);
				for(int i=0; i<tagLst.length; i++)
				{
					ESP_LOGI(TAG, "\t tag : %d", tagLst.content[i].tag_number);
					free(tagLst.content[i].values);
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

static void initialize_wifi_sniffer(void)
{
	MAC_address_hashset_init(&detectedMacAddresses);

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
	ESP_ERROR_CHECK(esp_wifi_start());


	wifi_promiscuous_filter_t filter= {.filter_mask=WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_CTRL};
	wifi_promiscuous_filter_t filterCtrl = {.filter_mask=WIFI_PROMIS_CTRL_FILTER_MASK_RTS};

	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&filter));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_ctrl_filter(&filterCtrl));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(sniffer));
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
	

}

/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
	assert(sta_netif);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	uint16_t ap_count = 0;
	memset(ap_info, 0, sizeof(ap_info));


	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_MAX));
	ESP_ERROR_CHECK(esp_wifi_start());
	
	while(1)
	{
		esp_wifi_scan_start(NULL, true);
		ESP_LOGI(TAG, "Max AP number ap_info can hold = %u", number);
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
		ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
		ESP_LOGI(TAG, " ");
		ESP_LOGI(TAG, " ");
		ESP_LOGI(TAG, "Total APs scanned = %u, actual AP number ap_info holds = %u", ap_count, number);
		
		for (int i = 0; i < number; i++)
		{
			ESP_LOGI(TAG, " ");
			ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
			ESP_LOGI(TAG, "RSSI \t\t%d", ap_info[i].rssi);
			print_auth_mode(ap_info[i].authmode);
			if (ap_info[i].authmode != WIFI_AUTH_WEP) {
				print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
			}
			ESP_LOGI(TAG, "Channel \t\t%d", ap_info[i].primary);
		}
	}


}

void app_main(void)
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	printThreadName();

	initialize_wifi_sniffer();
}
