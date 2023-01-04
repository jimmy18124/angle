#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_log.h"

#include "user_adc.h"
#include "user_udp.h"

static const char *TAG = "adc";

static void adc_task()
{
	uint16_t adc_data[100];

	while (1)
	{
		if (ESP_OK == adc_read(&adc_data[0]))
		{
			//ESP_LOGI(TAG, "adc read: %d\r\n", adc_data[0]);
			//user_udp_send_string("adc read");
		}

		vTaskDelay(1000 / portTICK_RATE_MS);

	}
}

void Adc_Config(void)
{
	adc_config_t adc_config;

	adc_config.mode = ADC_READ_TOUT_MODE;
	adc_config.clk_div = 8;
	ESP_ERROR_CHECK(adc_init(&adc_config));

	xTaskCreate(adc_task, "adc_task", 1024, NULL, 5, NULL);
}
