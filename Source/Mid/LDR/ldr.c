/*
 * ldr.c
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#include "app/framework/include/af.h"
#include <math.h>
#include "ldr.h"
#include "Source/Mid/Kalman_filter/kalman_filter.h"



EmberEventControl readValueSensorLightControl;
volatile uint32_t registor;
volatile uint32_t lux = 0;
uint32_t valueADC=0;
uint32_t kalman_Light=0;
/**
 * @func    LDRInit
 * @brief   LDR initialize
 * @param   None
 * @retval  None
 */
void LDRInit(void)
{
	// Declare init structs
	  IADC_Init_t init = IADC_INIT_DEFAULT;
	  IADC_AllConfigs_t initAllConfigs = IADC_ALLCONFIGS_DEFAULT;
	  IADC_InitSingle_t initSingle = IADC_INITSINGLE_DEFAULT;
	  IADC_SingleInput_t initSingleInput = IADC_SINGLEINPUT_DEFAULT;

	  // Enable IADC clock
	  CMU_ClockEnable(cmuClock_IADC0, true);
	  // enable pin

	  // Reset IADC to reset configuration in case it has been modified
	  IADC_reset(IADC0);

	  // Configure IADC clock source for use while in EM2
	  CMU_ClockSelectSet(cmuClock_IADCCLK, cmuSelect_FSRCO);

	  // Modify init structs and initialize
	  init.warmup = iadcWarmupKeepWarm;

	  // Set the HFSCLK prescale value here
	  init.srcClkPrescale = IADC_calcSrcClkPrescale(IADC0, CLK_SRC_ADC_FREQ, 0);

	  // Configuration 0 is used by both scan and single conversions by default
	  // Use unbuffered AVDD as reference
	  initAllConfigs.configs[0].reference = iadcCfgReferenceVddx;

	  // Divides CLK_SRC_ADC to set the CLK_ADC frequency for desired sample rate
	  initAllConfigs.configs[0].adcClkPrescale = IADC_calcAdcClkPrescale(IADC0,
	                                                                    CLK_ADC_FREQ,
	                                                                    0,
	                                                                    iadcCfgModeNormal,
	                                                                    init.srcClkPrescale);

	  // Set oversampling rate to 32x
	  // resolution formula res = 11 + log2(oversampling * digital averaging)
	  // in this case res = 11 + log2(32*1) = 16
	  initAllConfigs.configs[0].osrHighSpeed = iadcCfgOsrHighSpeed32x;


	  // Single initialization
	  initSingle.dataValidLevel = _IADC_SINGLEFIFOCFG_DVL_VALID1;

	  // Set conversions to run continuously
	//  initSingle.triggerAction = iadcTriggerActionContinuous;

	  // Set alignment to right justified with 16 bits for data field
	//  initSingle.alignment = iadcAlignRight16;

	  // Configure Input sources for single ended conversion
	  initSingleInput.posInput = iadcPosInputPortCPin5;
	  initSingleInput.negInput = iadcNegInputGnd;

	  // Initialize IADC
	  // Note oversampling and digital averaging will affect the offset correction
	  // This is taken care of in the IADC_init() function in the emlib
	  IADC_init(IADC0, &init, &initAllConfigs);

	  // Initialize Scan
	  IADC_initSingle(IADC0, &initSingle, &initSingleInput);

	  // Allocate the analog bus for ADC0 inputs
	  GPIO->IADC_INPUT_0_BUS |= GPIO_CDBUSALLOC_CDODD0_ADC0;  //IADC_INPUT_BUSALLOC
}

/**
 * @func   LightSensor_AdcPollingReadHandler
 * @brief  Read value from ADC
 * @param  None
 * @retval None
 */
uint32_t LightSensor_AdcPollingRead(void)
{
	emberEventControlSetInactive(readValueSensorLightControl);
	IADC_Result_t iadcResult;
	// Start IADC conversion
	IADC_command(IADC0, iadcCmdStartSingle);

	// Wait for conversion to be complete
	while((IADC0->STATUS & (_IADC_STATUS_CONVERTING_MASK
				| _IADC_STATUS_SINGLEFIFODV_MASK)) != IADC_STATUS_SINGLEFIFODV); //while combined status bits 8 & 6 don't equal 1 and 0 respectively

	// Get ADC result
	iadcResult = IADC_pullSingleFifoResult(IADC0);
	valueADC = iadcResult.data;
	kalman_Light = Kalman_sensor(valueADC);
	// Calculate input voltage:
	//  For differential inputs, the resultant range is from -Vref to +Vref, i.e.,
	//  for Vref = AVDD = 3.30V, 12 bits represents 6.60V full scale IADC range.
	registor= 10000*(3300 - kalman_Light)/(kalman_Light);    // registor  = 10K*ADC / (4095 -ADC)
	lux = abs(316*pow(10,5)*pow(registor,-1.4));
	return lux;
}


