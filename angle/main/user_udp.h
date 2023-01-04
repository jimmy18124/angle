#ifndef __USER_UDP_H
#define __USER_UDP_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void UDP_Config(void);
void user_udp_send(uint8_t* data, uint16_t len);
void user_udp_send_string(const char * data);

#endif
