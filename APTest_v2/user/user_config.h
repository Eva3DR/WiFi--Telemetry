/*
 * user_config.h
 *
 *  Created on: Sep 10, 2015
 *      Author: uds002
 */

#ifndef USER_USER_CONFIG_H_
#define USER_USER_CONFIG_H_

#include <c_types.h>

#define APNAME_MAX_LENGTH	32
#define APPSW_MAX_LENGTH	64

#define TESTING_MSG			"HELLO\r\n"

//#define DEBUG_MODE
//#define DEVELOP_CONFIG

#define MAX_PACKET_LENGTH	252
 // packet buffer
uint8_t	pbuf[MAX_PACKET_LENGTH];

#endif /* USER_USER_CONFIG_H_ */
