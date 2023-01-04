#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "user_gpio.h"
#include "user_adc.h"
#include "user_udp.h"
#include "user_jy61p.h"

#define USER_WIFI_SSID      "FECG5"
#define USER_WIFI_PASS      "12345678"

static const char *TAG = "main";
uint8_t WifiConnectedApState = 0;

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(TAG, "wifi start!");
		esp_wifi_connect();
	}
	else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
	{
		ESP_LOGI(TAG, "wifi connected!");
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		ESP_LOGI(TAG, "wifi disconnected!");
		WifiConnectedApState = 0;
		Led_Crl(0);
		esp_wifi_connect();
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
		WifiConnectedApState = 1;
		Led_Crl(1);
	}
}

void wifi_init_sta(void)
{
	tcpip_adapter_init();

	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

	wifi_config_t wifi_config = { .sta = { .ssid = USER_WIFI_SSID, .password = USER_WIFI_PASS }, };
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	//wifi_set_sleep_type(NONE_SLEEP_T);
	esp_wifi_set_ps(WIFI_PS_NONE);
	esp_wifi_set_max_tx_power(78);
	wifi_country_t my_country = { .cc = "JP", .schan = 1, .nchan = 14,
				.max_tx_power = 78, .policy = WIFI_COUNTRY_POLICY_AUTO
	};
	esp_wifi_set_country(&my_country);
	ESP_LOGI(TAG, "wifi_init_sta finished.");

}

void app_main()
{
	vTaskDelay(50 / portTICK_RATE_MS);

	Gpio_Config();

	ESP_ERROR_CHECK(nvs_flash_init());

	ESP_LOGI(TAG, "hello, app_main();");

	wifi_init_sta();

	vTaskDelay(500 / portTICK_RATE_MS);

	A3p3V_en();

	//Adc_Config();

	UDP_Config();

	vTaskDelay(1000 / portTICK_RATE_MS);

	user_jy61p_config();

	while(1)
	{
		vTaskDelay(500 / portTICK_RATE_MS);
		Feed_Dogs();
		//ESP_LOGI(TAG, "Feed_Dogs");
		//os_delay_us(10);
	}
}
