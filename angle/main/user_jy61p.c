#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "user_jy61p.h"
#include "user_udp.h"

static const char *TAG = "jy61p";

#define JY61P_ADDR  	0x50

#define SOFTWARE_IIC

#ifdef SOFTWARE_IIC
#define IIC_SCL1		GPIO_NUM_5
#define IIC_SDA1		GPIO_NUM_4

#define IIC_SCL_H()   	gpio_set_level(IIC_SCL1, 1)
#define IIC_SDA_H()   	gpio_set_level(IIC_SDA1, 1)
#define IIC_SCL_L()   	gpio_set_level(IIC_SCL1, 0)
#define IIC_SDA_L()  	gpio_set_level(IIC_SDA1, 0)


#define SDA_IN()  		gpio_set_direction(IIC_SDA1, GPIO_MODE_INPUT)
#define SDA_OUT() 		gpio_set_direction(IIC_SDA1, GPIO_MODE_OUTPUT)

#define READ_SDA()		gpio_get_level(IIC_SDA1)

static void IIC_Init(void)
{
	gpio_config_t io_conf;

	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1 << IIC_SCL1;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);
	gpio_set_level(IIC_SCL1, 1);

	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = 1 << IIC_SDA1;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	gpio_config(&io_conf);
	gpio_set_level(IIC_SDA1, 1);
}

static void IIC_Start(void) //起始位
{
	SDA_OUT();
	IIC_SDA_H();
	IIC_SCL_H();
	os_delay_us(4);
	IIC_SDA_L();
	os_delay_us(4);
	//IIC_SCL_L();
	//os_delay_us(4);
}

static void IIC_Stop(void) //终止位
{
	SDA_OUT();
	//IIC_SCL_L();
	IIC_SDA_L();
	//os_delay_us(4);
	IIC_SCL_H();
	os_delay_us(4);
	IIC_SDA_H();
	os_delay_us(4);
	//IIC_SCL_L();
}

static uint8_t IIC_Wait_Ack(void) //等待位
{
	uint16_t ucErrTime = 0;
	IIC_SDA_H();
	//os_delay_us(4);
	SDA_IN();

	//IIC_SDA_H();
	//os_delay_us(4);
	IIC_SCL_H();
	//os_delay_us(4);

	while (READ_SDA())
	{

		ucErrTime++;
		os_delay_us(2);
		if (ucErrTime > 250)
		{
			IIC_Stop();
			//IIC_SCL_L();
			return 1;
		}
	}

	IIC_SCL_L();
	return 0;
}

static void IIC_Ack(void) //发送ACK
{
	IIC_SCL_L();
	//os_delay_us(4);
	SDA_OUT();
	IIC_SDA_L();
	os_delay_us(4);

	IIC_SCL_H();
	os_delay_us(4);
	IIC_SCL_L();
	os_delay_us(4);
}
static void IIC_NAck(void) //发送NACK
{
	IIC_SCL_L();
	SDA_OUT();
	IIC_SDA_H();
	os_delay_us(4);
	IIC_SCL_H();
	os_delay_us(4);
	IIC_SCL_L();
	os_delay_us(4);
}

static void IIC_Send_Byte( uint8_t txd) //发送函数
{
	uint8_t t;
	SDA_OUT();
	IIC_SCL_L();
	os_delay_us(4);

	for (t = 0; t < 8; t++)
	{
		if ((txd & 0x80) >> 7)
		{
			IIC_SDA_H();
		}
		else
		{
			IIC_SDA_L();
		}
		txd <<= 1;
		os_delay_us(4);
		IIC_SCL_H();
		os_delay_us(4);
		IIC_SCL_L();
		os_delay_us(4);
	}
}

static uint8_t IIC_Read_Byte_ACK(void) //接收函数，结尾ACK
{
	unsigned char i, receive = 0;
	SDA_IN();
	for (i = 0; i < 8; i++)
	{
		IIC_SCL_L();
		os_delay_us(4);
		IIC_SCL_H();
		receive <<= 1;
		if (READ_SDA())
			receive++;
		os_delay_us(4);
	}
	IIC_Ack();
	return receive;
}

static uint8_t IIC_Read_Byte_NACK(void) //接收函数，结尾NACK
{
	unsigned char i, receive = 0;
	SDA_IN();
	for (i = 0; i < 8; i++)
	{
		IIC_SCL_L();
		os_delay_us(4);
		IIC_SCL_H();
		receive <<= 1;
		if (READ_SDA())
			receive++;
		os_delay_us(4);
	}
	IIC_NAck();
	return receive;
}

static void read_jy61p(void)
{
	uint8_t data[40] = { 0 };
	int16_t a[5];
	uint8_t i = 0;
	static char s1[200], s[2000];

	IIC_Start();
	IIC_Send_Byte(JY61P_ADDR << 1);
	if(IIC_Wait_Ack())
	{
		ESP_LOGI(TAG, "1 no ack\r\n");
	}
	IIC_Send_Byte(0x30);
	if(IIC_Wait_Ack())
	{
		ESP_LOGI(TAG, "2 no ack\r\n");
	}

	IIC_Start();
	IIC_Send_Byte((JY61P_ADDR << 1) + 1);
	if(IIC_Wait_Ack())
	{
		ESP_LOGI(TAG, "3 no ack\r\n");
	}

	for(i = 0; i < 34; i++)
	{
		data[i] = IIC_Read_Byte_ACK();
	}

	data[35] = IIC_Read_Byte_NACK();

	IIC_Stop();

//	for(i = 0; i < 34; i++)
//	{
//		ESP_LOGI(TAG, "data[%d] = 0x%02x",i, data[i]);
//	}

	sprintf(s1, "TIME:%d-%d-%d %d:%d:%d-%d",data[0],data[1],data[2],data[3],data[4],data[5],data[6] + data[7] * 256);
	ESP_LOGI(TAG, "%s",s1);
	sprintf(s, "jp61y:\n%s\n",s1);

	memcpy(a, &data[8], 6);
	sprintf(s1, "ACC:Ax=%fg Ay=%fg Az=%fg",a[0]/32768.0*16,a[1]/32768.0*16,a[2]/32768.0*16);
	ESP_LOGI(TAG, "%s",s1);
	strcat(s, s1);
	strcat(s, "\n");

	memcpy(a, &data[14], 6);
	sprintf(s1, "GYRO:Wx=%f Wy=%f Wz=%f",a[0]/32768.0*2000,a[1]/32768.0*2000,a[2]/32768.0*2000);
	ESP_LOGI(TAG, "%s",s1);
	strcat(s, s1);
	strcat(s, "\n");

	memcpy(a, &data[26], 6);
	sprintf(s1, "ANGLE:Roll=%f Pitch=%f Yaw=%f",a[0]/32768.0*180,a[1]/32768.0*180,a[2]/32768.0*180);
	ESP_LOGI(TAG, "%s",s1);
	strcat(s, s1);
	strcat(s, "\n");

	memcpy(a, &data[32], 2);
	sprintf(s1, "TEMP=%f\r\n",a[0]/100.0);
	ESP_LOGI(TAG, "%s",s1);
	strcat(s, s1);

	user_udp_send_string(s);

}

static void user_jy61p_task(void *arg)
{
	IIC_Init();

	while (1)
	{
		read_jy61p();
		vTaskDelay(980 / portTICK_RATE_MS);
	}

}

void user_jy61p_config(void)
{
	xTaskCreate(user_jy61p_task, "user_jy61p_task", 2048, NULL, 8, NULL);
}

#else

#include "driver/i2c.h"


#define I2C_MASTER_SCL_IO           		5
#define I2C_MASTER_SDA_IO          			4
#define WRITE_BIT                           0x0
#define READ_BIT                            0x1
#define ACK_CHECK_EN                        0x1
#define ACK_CHECK_DIS                       0x0
#define ACK_VAL                             0x0
#define NACK_VAL                            0x1
#define LAST_NACK_VAL                       0x2


static void IIC_Init(void)
{
	i2c_config_t conf;

	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_MASTER_SDA_IO;
	conf.sda_pullup_en = 1;
	conf.scl_io_num = I2C_MASTER_SCL_IO;
	conf.scl_pullup_en = 1;
	conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
}

static void read_jy61p(void)
{
	uint8_t data[100] = { 0 };
	uint8_t i = 0;


	int ret;
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, JY61P_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
		i2c_master_write_byte(cmd, 0x30, ACK_CHECK_EN);
		i2c_master_stop(cmd); //
		ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 500 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);

		if (ret != ESP_OK)
		{
			ESP_LOGI(TAG, "read_jy61p err1 = %x ",ret);
			return ;
		}

		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, JY61P_ADDR << 1 | READ_BIT, ACK_CHECK_EN);
		i2c_master_read(cmd, data, 98, LAST_NACK_VAL);
		i2c_master_stop(cmd);
		ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 500 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);

		if (ESP_OK != ret)
		{
			ESP_LOGI(TAG, "read_jy61p err2 = %x ",ret);
			return ;
		}


	for(i = 0; i < 98; i++)
	{
		ESP_LOGI(TAG, "data[%x] = %x \r\n",i/2 + 0x30, data[i]);
	}
	//ESP_LOGI(TAG, "sm6391[%d]: %x %x %x %x %x %x p = %f,t = %d,%f\r\n", num, data[0], data[1], data[2], data[3], data[4], data[5],Pressure_In_cmH2O,temperature,Temperature_In_Celsius);
}

static void user_jy61p_task(void *arg)
{
	IIC_Init();

	while (1)
	{
		read_jy61p();
		vTaskDelay(1000 / portTICK_RATE_MS);
	}

}

void user_jy61p_config(void)
{
	xTaskCreate(user_jy61p_task, "user_jy61p_task", 2048, NULL, 8, NULL);
}

#endif


