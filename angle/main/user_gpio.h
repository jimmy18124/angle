#ifndef __USER_GPIO_H
#define __USER_GPIO_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"

void Gpio_Config(void);

void Feed_Dogs(void);

void Led_Crl(uint8_t flag);

void A3p3V_en(void);

#endif
