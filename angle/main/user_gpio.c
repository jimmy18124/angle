#include "user_gpio.h"

#define GPIO_OUTPUT_LED     14
#define GPIO_OUTPUT_LED_PIN_SEL  (1ULL<<GPIO_OUTPUT_LED)

#define GPIO_OUTPUT_AV3V3EN     15
#define GPIO_OUTPUT_AV3V3EN_PIN_SEL  (1ULL<<GPIO_OUTPUT_AV3V3EN)

#define GPIO_OUTPUT_WDI     16
#define GPIO_OUTPUT_WDI_PIN_SEL  (1ULL<<GPIO_OUTPUT_WDI)

void Gpio_Config(void)
{
    gpio_config_t io_conf;

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_LED_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_OUTPUT_LED, 1);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_AV3V3EN_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_OUTPUT_AV3V3EN, 0);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_WDI_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    gpio_set_level(GPIO_OUTPUT_WDI, 1);

//        io_conf.intr_type = GPIO_INTR_DISABLE;
//        io_conf.mode = GPIO_MODE_OUTPUT;
//        io_conf.pin_bit_mask = 1<<0;
//        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<2;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<4;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<5;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<12;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<13;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<14;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<15;
//        gpio_config(&io_conf);
//
//        io_conf.pin_bit_mask = 1<<16;
//        gpio_config(&io_conf);

}

void Feed_Dogs(void)
{
	static uint8_t i = 0;

	gpio_set_level(GPIO_OUTPUT_WDI, i);
//	gpio_set_level(0, i);
//	gpio_set_level(2, i);
//	gpio_set_level(4, i);
//	gpio_set_level(5, i);
//	gpio_set_level(12, i);
//	gpio_set_level(13, i);
//	gpio_set_level(14, i);
//	gpio_set_level(15, i);
//	gpio_set_level(16, i);


	i = 1 - i;
}

void Led_Crl(uint8_t flag)
{
	flag ? gpio_set_level(GPIO_OUTPUT_LED, 0) : gpio_set_level(GPIO_OUTPUT_LED, 1);
}

void A3p3V_en(void)
{
	gpio_set_level(GPIO_OUTPUT_AV3V3EN, 1);
}
