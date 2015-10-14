/*
 * usr_cmd.h
 *
 *  Created on: Sep 15, 2015
 *      Author: uds002
 */

#ifndef USER_USR_CMD_H_
#define USER_USR_CMD_H_

enum cmd_list_t {
	CWSAP = 0,
	UART,
	RF_PWR,
	BAUD_TESTING,
	CIPPORT,
	SCAN,
	Cnt
};


#define CMD_LENGTH	Cnt
extern char * cmd_lst[CMD_LENGTH];

#endif /* USER_USR_CMD_H_ */
