/*
 * hw_timer.h
 *
 *  Created on: Oct 8, 2015
 *      Author: uds002
 */

#ifndef INCLUDE_DRIVER_HW_TIMER_H_
#define INCLUDE_DRIVER_HW_TIMER_H_

typedef enum {
    FRC1_SOURCE = 0,
    NMI_SOURCE = 1,
} FRC1_TIMER_SOURCE_TYPE;


#define REG_READ(_r) (*(volatile uint32 *)(_r))
#define WDEV_NOW() REG_READ(0x3ff20c00)

#endif /* INCLUDE_DRIVER_HW_TIMER_H_ */
