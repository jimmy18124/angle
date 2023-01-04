#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "user_udp.h"

#define HOST_IP_ADDR 	"192.168.43.255"
#define PORT 			5555
#define SELF_PORT		6666

extern uint8_t WifiConnectedApState;

static const char *TAG = "udp";

int udp_sock = -1;
struct sockaddr_in destAddr;

static void udp_task(void *pvParameters)
{
	char rx_buffer[128];
	//char addr_str[128];

	uint8_t IpInitOnce = 0;
	int ret = -1;
	int rlen = -1;

	destAddr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(PORT);
	//inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

    struct sockaddr_in selfAddr;
    selfAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    selfAddr.sin_family = AF_INET;
    selfAddr.sin_port = htons(SELF_PORT);



	while (1)
	{

		if ((0 == IpInitOnce) && WifiConnectedApState)
		{
			udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
			if (udp_sock < 0)
			{
				close(udp_sock);
				udp_sock = -1;
				ESP_LOGE(TAG, "Unable to create udp socket: errno %d", errno);
				IpInitOnce = 0;
			}
			else
			{
				ESP_LOGI(TAG, "UDP Socket created");
				ret = bind(udp_sock, (struct sockaddr *) &selfAddr, sizeof(selfAddr));
				if (ret < 0)
				{
					ret = -1;
					close(udp_sock);
					udp_sock = -1;
					ESP_LOGE(TAG, "udp socket unable to bind: errno %d", errno);
				}
				else
				{
					IpInitOnce = 1;
					ESP_LOGI(TAG, "usp socket binded");
				}
			}
		}

		if ((-1 != udp_sock) && WifiConnectedApState)
		{
			rlen = recv(udp_sock, rx_buffer, sizeof(rx_buffer) - 1, MSG_DONTWAIT);

			if (rlen > 0)
			{
				rx_buffer[rlen] = 0;
				ESP_LOGI(TAG, "Received %d bytes:", rlen);
				ESP_LOGI(TAG, "%s", rx_buffer);
			}
		}

		vTaskDelay(5 / portTICK_PERIOD_MS);

	}
	vTaskDelete(NULL);
}

void user_udp_send(uint8_t* data, uint16_t len)
{
	if ((-1 != udp_sock) && WifiConnectedApState)
	{
		int err = sendto(udp_sock, data, len, 0, (struct sockaddr *) &destAddr, sizeof(destAddr));
		if (err < 0)
		{
			ESP_LOGE(TAG, "Error occured during udp sending: errno %d", errno);
		}
	}
	else
	{
		ESP_LOGE(TAG, "udp disconn");
	}
}

void user_udp_send_string(const char * data)
{
	user_udp_send(data, strlen(data));
}

void UDP_Config(void)
{

	xTaskCreate(udp_task, "udp_task", 4096, NULL, 6, NULL);
}
