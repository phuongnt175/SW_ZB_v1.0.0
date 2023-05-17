/*
 * main.h
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#ifndef SOURCE_APP_MAIN_MAIN_H_
#define SOURCE_APP_MAIN_MAIN_H_

#define RGB1_ENDPOINT									1
#define RGB2_ENDPOINT									2
#define LIGH_ENDPOINT									3

typedef enum{
	POWER_ON_STATE,
	REPORT_STATE,
	IDLE_STATE,
	REBOOT_STATE
}SystemState_e;

#endif /* SOURCE_APP_MAIN_MAIN_H_ */
