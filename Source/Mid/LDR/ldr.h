/*
 * ldr.h
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#ifndef SOURCE_MID_LDR_LDR_H_
#define SOURCE_MID_LDR_LDR_H_

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_adc.h"
#include "em_iadc.h"
#include "em_gpio.h"

/******************************************************************************/
/*                     		DEFINITIONS            				              */
/******************************************************************************/
// Set CLK_ADC to 10MHz
#define CLK_SRC_ADC_FREQ          1000000 // CLK_SRC_ADC
#define CLK_ADC_FREQ              10000 // CLK_ADC - 10MHz max in normal mode

#define IADC_INPUT_0_PORT_PIN     iadcPosInputPortCPin5;
#define IADC_INPUT_1_PORT_PIN     iadcNegInputGnd;

#define IADC_INPUT_0_BUS          CDBUSALLOC
#define IADC_INPUT_0_BUSALLOC     GPIO_CDBUSALLOC_CDEVEN0_ADC0

/* Function prototypes -----------------------------------------------*/
void LDRInit(void);
uint32_t LightSensor_AdcPollingRead(void);

#endif /* SOURCE_MID_LDR_LDR_H_ */
