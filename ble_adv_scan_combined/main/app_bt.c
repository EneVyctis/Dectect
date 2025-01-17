/*
 * BLE Combined Advertising and Scanning Example.
 *
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "freertos/queue.h"
#include "bt_hci_common.h"
#include <string.h>


#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_POUR_LE_PROJET
#ifdef WIFI_POUR_LE_PROJET
#include "esp_wifi.h"
#include "esp_event.h"
//#include "wifi.h"
#include "client.h"
wifi_config_t wifi_config;
#endif


//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_system.h"
//#include "esp_log.h"
//#include "esp_wifi.h"
//#include "wifi.h"

//#include "lwip/err.h"
//#include "lwip/sys.h"

#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif



#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

wifi_config_t wifi_config = {
		.sta = {
			.ssid = EXAMPLE_ESP_WIFI_SSID,
			.password = EXAMPLE_ESP_WIFI_PASS,
			/* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
			 * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
			 * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
			 * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
			 */
			.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
			.sae_pwe_h2e = ESP_WIFI_SAE_MODE,
			.sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
			.channel = 0 // n'impose pas de channel précis pour la connexion
		},
	};


#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY
int s_retry_num = 0;


static EventGroupHandle_t s_wifi_event_group;

void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    char*TAG ="event_handler";
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		ESP_LOGI(TAG, "Lancement de la connexion au Wifi");
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
		if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) 
		{
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP, %d", s_retry_num);
		} 
		else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
		ESP_LOGI(TAG,"connect to the AP fail");
	} 
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
	else if (event_base == WIFI_EVENT) {
		ESP_LOGI(TAG, "Wifi event(%s) n°%ld detected",event_base,  event_id);
	}
	else {
		ESP_LOGI(TAG, "Base : %s", event_base);
	}

}


bool premier_passage = 1;
void wifi_init() {
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    if (premier_passage) {
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        premier_passage = 0;
	    esp_netif_create_default_wifi_sta();
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&event_handler,
														NULL,
														&instance_got_ip));

}

void wifi_init_sta(wifi_config_t* wifi_config)
{
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, wifi_config));
	wifi_config_t control;
	ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &control));
    char* TAG = "Wifi";
	ESP_LOGI("Wifi_init_sta", "%s", control.sta.ssid);
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
			pdTRUE,
			pdFALSE,
			portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
				 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
				 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
	}
	#ifdef WIFI_POUR_LE_PROJET
	#endif
	ESP_LOGI(TAG, "Fin de Wifi_init_sta");
}



static const char *TAG = "BLE_ADV_SCAN";

typedef struct {
    char scan_local_name[32];
    uint8_t name_len;
} ble_scan_local_name_t;

typedef struct {
    uint8_t *q_data;
    uint16_t q_data_len;
} host_rcv_data_t;

static uint8_t hci_cmd_buf[128];

static uint16_t scanned_count = 0;
static QueueHandle_t adv_queue;

#define MAX_DEVICES 1000 // Définir selon le besoin

typedef struct {
    uint8_t addr[6]; // Adresse de l'appareil
    int64_t last_seen; // Temps en millisecondes depuis le démarrage du système lors de la dernière détection
} detected_device_t;

static detected_device_t detected_devices[MAX_DEVICES];
// static uint8_t detected_devices[MAX_DEVICES][6]; // Stocke les adresses des appareils détectés
static uint16_t detected_count = 0; // Nombre d'appareils détectés

// Fonction pour vérifier si l'appareil a déjà été détecté
bool is_device_detected(const uint8_t* addr) {
    for (int i = 0; i < detected_count; ++i) {
        if (memcmp(detected_devices[i].addr, addr, 6) == 0) {
            detected_devices[i].last_seen = esp_timer_get_time() / 1000; // Conversion en millisecondes
            //break;
            return true; // Appareil déjà détecté
        }
    }
    return false; // Nouvel appareil
}

// Fonction pour ajouter un appareil à la liste des détectés
void add_detected_device(const uint8_t* addr) {
    if (detected_count < MAX_DEVICES && !is_device_detected(addr)) {
        memcpy(detected_devices[detected_count].addr, addr, 6);
        detected_devices[detected_count].last_seen = esp_timer_get_time() / 1000; // Conversion en millisecondes
        detected_count++;
    }
}

void check_and_remove_stale_devices(void) {
    int64_t current_time = esp_timer_get_time() / 1000;
    for (int i = 0; i < detected_count; ) {
        if ((current_time - detected_devices[i].last_seen) > 60000) {
            // Déplacer tous les appareils d'une position vers le bas pour enlever l'appareil obsolète
            memmove(&detected_devices[i], &detected_devices[i + 1], 
                    (detected_count - i - 1) * sizeof(detected_device_t));
            detected_count--;
        } else {
            i++;
        }
    }
}


/*
 * @brief: BT controller callback function, used to notify the upper layer that
 *         controller is ready to receive command
 */
static void controller_rcv_pkt_ready(void)
{
    ESP_LOGI(TAG, "controller rcv pkt ready");
}

/*
 * @brief: BT controller callback function to transfer data packet to
 *         the host
 */
static int host_rcv_pkt(uint8_t *data, uint16_t len)
{
    host_rcv_data_t send_data;
    uint8_t *data_pkt;
    /* Check second byte for HCI event. If event opcode is 0x0e, the event is
     * HCI Command Complete event. Sice we have recieved "0x0e" event, we can
     * check for byte 4 for command opcode and byte 6 for it's return status. */
    if (data[1] == 0x0e) {
        if (data[6] == 0) {
            ESP_LOGI(TAG, "Event opcode 0x%02x success.", data[4]);
        } else {
            ESP_LOGE(TAG, "Event opcode 0x%02x fail with reason: 0x%02x.", data[4], data[6]);
            return ESP_FAIL;
        }
    }

    data_pkt = (uint8_t *)malloc(sizeof(uint8_t) * len);
    if (data_pkt == NULL) {
        ESP_LOGE(TAG, "Malloc data_pkt failed!");
        return ESP_FAIL;
    }
    memcpy(data_pkt, data, len);
    send_data.q_data = data_pkt;
    send_data.q_data_len = len;
    if (xQueueSend(adv_queue, (void *)&send_data, ( TickType_t ) 0) != pdTRUE) {
        ESP_LOGD(TAG, "Failed to enqueue advertising report. Queue full.");
        /* If data sent successfully, then free the pointer in `xQueueReceive'
         * after processing it. Or else if enqueue in not successful, free it
         * here. */
        free(data_pkt);
    }
    return ESP_OK;
}

static esp_vhci_host_callback_t vhci_host_cb = {
    controller_rcv_pkt_ready,
    host_rcv_pkt
};

static void hci_cmd_send_reset(void)
{
    uint16_t sz = make_cmd_reset (hci_cmd_buf);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_set_evt_mask(void)
{
    /* Set bit 61 in event mask to enable LE Meta events. */
    uint8_t evt_mask[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20};
    uint16_t sz = make_cmd_set_evt_mask(hci_cmd_buf, evt_mask);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_ble_scan_params(void)
{
    /* Set scan type to 0x01 for active scanning and 0x00 for passive scanning. */
    uint8_t scan_type = 0x01;

    /* Scan window and Scan interval are set in terms of number of slots. Each slot is of 625 microseconds. */
    uint16_t scan_interval = 0x50; /* 50 ms */
    uint16_t scan_window = 0x30; /* 30 ms */

    uint8_t own_addr_type = 0x00; /* Public Device Address (default). */
    uint8_t filter_policy = 0x00; /* Accept all packets excpet directed advertising packets (default). */
    uint16_t sz = make_cmd_ble_set_scan_params(hci_cmd_buf, scan_type, scan_interval, scan_window, own_addr_type, filter_policy);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_ble_scan_start(void)
{
    uint8_t scan_enable = 0x01; /* Scanning enabled. */
    uint8_t filter_duplicates = 0x00; /* Duplicate filtering disabled. */
    uint16_t sz = make_cmd_ble_set_scan_enable(hci_cmd_buf, scan_enable, filter_duplicates);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
    ESP_LOGI(TAG, "BLE Scanning started..");
}

static void hci_cmd_send_ble_adv_start(void)
{
    uint16_t sz = make_cmd_ble_set_adv_enable (hci_cmd_buf, 1);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
    ESP_LOGI(TAG, "BLE Advertising started..");
}

static void hci_cmd_send_ble_set_adv_param(void)
{
    /* Minimum and maximum Advertising interval are set in terms of slots. Each slot is of 625 microseconds. */
    uint16_t adv_intv_min = 0x100;
    uint16_t adv_intv_max = 0x100;

    /* Connectable undirected advertising (ADV_IND). */
    uint8_t adv_type = 0;

    /* Own address is public address. */
    uint8_t own_addr_type = 0;

    /* Public Device Address */
    uint8_t peer_addr_type = 0;
    uint8_t peer_addr[6] = {0x80, 0x81, 0x82, 0x83, 0x84, 0x85};

    /* Channel 37, 38 and 39 for advertising. */
    uint8_t adv_chn_map = 0x07;

    /* Process scan and connection requests from all devices (i.e., the White List is not in use). */
    uint8_t adv_filter_policy = 0;

    uint16_t sz = make_cmd_ble_set_adv_param(hci_cmd_buf,
                  adv_intv_min,
                  adv_intv_max,
                  adv_type,
                  own_addr_type,
                  peer_addr_type,
                  peer_addr,
                  adv_chn_map,
                  adv_filter_policy);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
}

static void hci_cmd_send_ble_set_adv_data(void)
{
    char *adv_name = "ESP-BLE-1";
    uint8_t name_len = (uint8_t)strlen(adv_name);
    uint8_t adv_data[31] = {0x02, 0x01, 0x06, 0x0, 0x09};
    uint8_t adv_data_len;

    adv_data[3] = name_len + 1;
    for (int i = 0; i < name_len; i++) {
        adv_data[5 + i] = (uint8_t)adv_name[i];
    }
    adv_data_len = 5 + name_len;

    uint16_t sz = make_cmd_ble_set_adv_data(hci_cmd_buf, adv_data_len, (uint8_t *)adv_data);
    esp_vhci_host_send_packet(hci_cmd_buf, sz);
    ESP_LOGI(TAG, "Starting BLE advertising with name \"%s\"", adv_name);
}

static esp_err_t get_local_name (uint8_t *data_msg, uint8_t data_len, ble_scan_local_name_t *scanned_packet)
{
    uint8_t curr_ptr = 0, curr_len, curr_type;
    while (curr_ptr < data_len) {
        curr_len = data_msg[curr_ptr++];
        curr_type = data_msg[curr_ptr++];
        if (curr_len == 0) {
            return ESP_FAIL;
        }

        /* Search for current data type and see if it contains name as data (0x08 or 0x09). */
        if (curr_type == 0x08 || curr_type == 0x09) {
            for (uint8_t i = 0; i < curr_len - 1; i += 1) {
                scanned_packet->scan_local_name[i] = data_msg[curr_ptr + i];
            }
            scanned_packet->name_len = curr_len - 1;
            return ESP_OK;
        } else {
            /* Search for next data. Current length includes 1 octate for AD Type (2nd octate). */
            curr_ptr += curr_len - 1;
        }
    }
    return ESP_FAIL;
}

void hci_evt_process(void *pvParameters)
{
    host_rcv_data_t *rcv_data = (host_rcv_data_t *)malloc(sizeof(host_rcv_data_t));
    if (rcv_data == NULL) {
        ESP_LOGE(TAG, "Malloc rcv_data failed!");
        return;
    }
    esp_err_t ret;

    while (1) {
        uint8_t sub_event, num_responses, total_data_len, data_msg_ptr, hci_event_opcode;
        uint8_t *queue_data = NULL, *event_type = NULL, *addr_type = NULL, *addr = NULL, *data_len = NULL, *data_msg = NULL;
        short int *rssi = NULL;
        uint16_t data_ptr;
        ble_scan_local_name_t *scanned_name = NULL;
        total_data_len = 0;
        data_msg_ptr = 0;
        if (xQueueReceive(adv_queue, rcv_data, portMAX_DELAY) != pdPASS) {
            ESP_LOGE(TAG, "Queue receive error");
        } else {
            /* `data_ptr' keeps track of current position in the received data. */
            data_ptr = 0;
            queue_data = rcv_data->q_data;

            /* Parsing `data' and copying in various fields. */
            hci_event_opcode = queue_data[++data_ptr];
            if (hci_event_opcode == LE_META_EVENTS) {
                /* Set `data_ptr' to 4th entry, which will point to sub event. */
                data_ptr += 2;
                sub_event = queue_data[data_ptr++];
                /* Check if sub event is LE advertising report event. */
                if (sub_event == HCI_LE_ADV_REPORT) {

                    scanned_count += 1;



                    /* Get number of advertising reports. */
                    num_responses = queue_data[data_ptr++];
                    event_type = (uint8_t *)malloc(sizeof(uint8_t) * num_responses);
                    if (event_type == NULL) {
                        ESP_LOGE(TAG, "Malloc event_type failed!");
                        goto reset;
                    }
                    for (uint8_t i = 0; i < num_responses; i += 1) {
                        event_type[i] = queue_data[data_ptr++];
                    }

                    /* Get advertising type for every report. */
                    addr_type = (uint8_t *)malloc(sizeof(uint8_t) * num_responses);
                    if (addr_type == NULL) {
                        ESP_LOGE(TAG, "Malloc addr_type failed!");
                        goto reset;
                    }
                    for (uint8_t i = 0; i < num_responses; i += 1) {
                        addr_type[i] = queue_data[data_ptr++];
                    }

                    /* Get BD address in every advetising report and store in
                     * single array of length `6 * num_responses' as each address
                     * will take 6 spaces. */
                    addr = (uint8_t *)malloc(sizeof(uint8_t) * 6 * num_responses);
                    if (addr == NULL) {
                        ESP_LOGE(TAG, "Malloc addr failed!");
                        goto reset;
                    }
                    for (int i = 0; i < num_responses; i += 1) {
                        for (int j = 0; j < 6; j += 1) {
                            addr[(6 * i) + j] = queue_data[data_ptr++];
                        }
                        if (!is_device_detected(addr)) {
                            add_detected_device(addr);
                        }
                    }

                    /* Get length of data for each advertising report. */
                    data_len = (uint8_t *)malloc(sizeof(uint8_t) * num_responses);
                    if (data_len == NULL) {
                        ESP_LOGE(TAG, "Malloc data_len failed!");
                        goto reset;
                    }
                    for (uint8_t i = 0; i < num_responses; i += 1) {
                        data_len[i] = queue_data[data_ptr];
                        total_data_len += queue_data[data_ptr++];
                    }

                    if (total_data_len != 0) {
                        /* Get all data packets. */
                        data_msg = (uint8_t *)malloc(sizeof(uint8_t) * total_data_len);
                        if (data_msg == NULL) {
                            ESP_LOGE(TAG, "Malloc data_msg failed!");
                            goto reset;
                        }
                        for (uint8_t i = 0; i < num_responses; i += 1) {
                            for (uint8_t j = 0; j < data_len[i]; j += 1) {
                                data_msg[data_msg_ptr++] = queue_data[data_ptr++];
                            }
                        }
                    }

                    /* Counts of advertisements done. This count is set in advertising data every time before advertising. */
                    rssi = (short int *)malloc(sizeof(short int) * num_responses);
                    if (data_len == NULL) {
                        ESP_LOGE(TAG, "Malloc rssi failed!");
                        goto reset;
                    }
                    for (uint8_t i = 0; i < num_responses; i += 1) {
                        rssi[i] = -(0xFF - queue_data[data_ptr++]);
                    }

                    /* Extracting advertiser's name. */
                    data_msg_ptr = 0;
                    scanned_name = (ble_scan_local_name_t *)malloc(num_responses * sizeof(ble_scan_local_name_t));
                    if (data_len == NULL) {
                        ESP_LOGE(TAG, "Malloc scanned_name failed!");
                        goto reset;
                    }
                    for (uint8_t i = 0; i < num_responses; i += 1) {
                        ret = get_local_name(&data_msg[data_msg_ptr], data_len[i], scanned_name);

                        /* Print the data if adv report has a valid name. */
                        /*
                        if (ret == ESP_OK) {
                            printf("******** Response %d/%d ********\n", i + 1, num_responses);
                            printf("Event type: %02x\nAddress type: %02x\nAddress: ", event_type[i], addr_type[i]);
                            for (int j = 5; j >= 0; j -= 1) {
                                printf("%02x", addr[(6 * i) + j]);
                                if (j > 0) {
                                    printf(":");
                                }
                            }

                            printf("\nData length: %d", data_len[i]);
                            data_msg_ptr += data_len[i];
                            printf("\nAdvertisement Name: ");
                            for (int k = 0; k < scanned_name->name_len; k += 1 ) {
                                printf("%c", scanned_name->scan_local_name[k]);
                            }
                            printf("\nRSSI: %ddB\n", rssi[i]);
                        }
                        */
                        
                    }

                    /* Freeing all spaces allocated. */
reset:
                    free(scanned_name);
                    free(rssi);
                    free(data_msg);
                    free(data_len);
                    free(addr);
                    free(addr_type);
                    free(event_type);
                }
            }
#if (CONFIG_LOG_DEFAULT_LEVEL_DEBUG || CONFIG_LOG_DEFAULT_LEVEL_VERBOSE)
            printf("Raw Data:");
            for (uint8_t j = 0; j < rcv_data->q_data_len; j += 1) {
                printf(" %02x", queue_data[j]);
            }
            printf("\nQueue free size: %d\n", uxQueueSpacesAvailable(adv_queue));
#endif
            free(queue_data);
        }
        memset(rcv_data, 0, sizeof(host_rcv_data_t));
    }
}

void lancement_bluetooth(void) {
    bool continue_commands = 1;
    int cmd_cnt = 0;

    esp_vhci_host_register_callback(&vhci_host_cb);
    while (continue_commands) {
        if (continue_commands && esp_vhci_host_check_send_available()) {
            switch (cmd_cnt) {
            case 0: hci_cmd_send_reset(); ++cmd_cnt; break;
            case 1: hci_cmd_send_set_evt_mask(); cmd_cnt=cmd_cnt+4; break;

            /* Send advertising commands. 
            case 2: hci_cmd_send_ble_set_adv_param(); ++cmd_cnt; break;
            case 3: hci_cmd_send_ble_set_adv_data(); ++cmd_cnt; break;
            case 4: hci_cmd_send_ble_adv_start(); ++cmd_cnt; break;
            */

            /* Send scan commands. */
            case 5: hci_cmd_send_ble_scan_params(); ++cmd_cnt; break;
            case 6: hci_cmd_send_ble_scan_start(); ++cmd_cnt; break;
            default: continue_commands = 0; break;
            }
            ESP_LOGI(TAG, "BLE Advertise, cmd_sent: %d", cmd_cnt);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    xTaskCreatePinnedToCore(&hci_evt_process, "hci_evt_process", 4096, NULL, 6, NULL, 0);

}


static void periodic_timer_callback(void *arg)
{
    check_and_remove_stale_devices(); // Vérifie et supprime les appareils obsolètes
    ESP_LOGI(TAG, "Number of received advertising reports: %d", scanned_count);
    ESP_LOGI(TAG, "Nombre final d'appareils détectés : %d", detected_count);
    ESP_ERROR_CHECK(esp_bt_controller_disable());
    ESP_ERROR_CHECK(esp_bt_controller_deinit());

#ifdef WIFI_POUR_LE_PROJET
    ESP_LOGI(TAG, "Lancement du code après wifi_init_STA");
    int sock = establish_connexion();
    if (sock != -1) {
        for (int i = 0; i<detected_count; i++) {
            char macStr[18];
            sprintf(macStr, "%02x:%02x:%02x:%02x:%02x:%02x",
            detected_devices[i].addr[0],
            detected_devices[i].addr[1],
            detected_devices[i].addr[2],
            detected_devices[i].addr[3],
            detected_devices[i].addr[4],
            detected_devices[i].addr[5]);
            ESP_LOGI(TAG,"%s",macStr);
            send_message(sock, macStr);
        }
    }
    close(sock);
    #endif
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    esp_err_t ret;
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGI(TAG, "Bluetooth controller initialize failed: %s", esp_err_to_name(ret));
        return;
    }
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK) {
        ESP_LOGI(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return;
    }
    lancement_bluetooth();
}

void app_main(void)
{


    /* Initialize NVS — it is used to store PHY calibration data */
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "inside");
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret) {
        ESP_LOGI(TAG, "Bluetooth controller release classic bt memory failed: %s", esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGI(TAG, "Bluetooth controller initialize failed: %s", esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_BLE)) != ESP_OK) {
        ESP_LOGI(TAG, "Bluetooth controller enable failed: %s", esp_err_to_name(ret));
        return;
    }

    /* A queue for storing received HCI packets. */
    adv_queue = xQueueCreate(15, sizeof(host_rcv_data_t));
    if (adv_queue == NULL) {
        ESP_LOGE(TAG, "Queue creation failed");
        return;
    }
    lancement_bluetooth();

    wifi_init();
    wifi_init_sta(&wifi_config);

    int sock = establish_connexion();
    send_message(sock, "00:00:00:00:00:00");
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &periodic_timer_callback,
        .name = "periodic"
    };

    /* Create timer for logging scanned devices. */
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));

    /* Start periodic timer for 5 sec. */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000000));
}
