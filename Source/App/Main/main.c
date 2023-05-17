 /* File name: main.c
 *
 * Description:
 *
 *
 * Last Changed By:  $Author: $
 * Revision:         $Revision: $
 * Last Changed:     $Date: $May 17, 2023
 *
 * Code sample:
 ******************************************************************************/
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include "app/framework/include/af.h"
#include "Source/Mid/Led/led.h"
#include "Source/Mid/Button/button.h"
#include "Source/Mid/Kalman_filter/kalman_filter.h"
#include "Source/Mid/LDR/ldr.h"
#include "protocol/zigbee/stack/include/binding-table.h"
#include "Source/App/Network/network.h"
#include "Source/App/Send/send.h"
#include "Source/App/Receive/receive.h"
#include "math.h"
#include "main.h"

/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/

#define PERIOD_SCAN_SENSORLIGHT									1000 	//	ms

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

bool g_boNetworkReady = false;
SystemState_e g_SystemState = POWER_ON_STATE;
uint32_t g_iluxFirsttime = 0;
uint32_t g_iluxSecondtimes = 0;

/* Event **************************************************************/
EmberEventControl lightSensorRead1timeControl;
EmberEventControl mainStateEventControl;
EmberEventControl MTORRsEventControl;
EmberEventControl findNetworkControl;

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/

void mainButtonPressCallbackHandler(uint8_t byButton, ButtonEvent_e pressHandler);
void mainButtonHoldCallbackHandler(uint8_t byButton, ButtonEvent_e holdingHandler);
void mainNetworkEventHandler(uint8_t byNetworkResult);

/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

/******************************************************************************/
/*
 * * @brief Main Init
 */
void emberAfMainInitCallback(void)
{
	emberAfCorePrintln("Main Init");
	ledInit();
	buttonInit(mainButtonHoldCallbackHandler, mainButtonPressCallbackHandler);
	networkInit(mainNetworkEventHandler);
	emberEventControlSetActive(mainStateEventControl);
	LDRInit();
	KalmanFilterInit(2, 2, 0.001); // Initialize Kalman filter
	emberEventControlSetDelayMS(lightSensorRead1timeControl, 1000);
}

/****************************EVENT HANDLER MIDDER********************************************************************/
/*
 * @func	Main_ButtonPressCallbackHandler
 * @brief	Event Button Handler
 * @param	button, pressHandler
 * @retval	None
 */
void mainButtonPressCallbackHandler(uint8_t byButton, ButtonEvent_e pressHandler)
{
	switch(pressHandler)
		{
		case press_1:
			if(byButton == SW_1)
			{
				emberAfCorePrintln("SW1: 1 time");
				turnOnLed(LED1,ledBlue);
				sendOnOffStateReport(1, LED_ON);
				sendBindingInitToTarget(SOURCE_ENDPOINT_PRIMARY,
										 DESTINATTION_ENDPOINT,
										 true,
										 HC_NETWORK_ADDRESS);
			}
			else
			{
				emberAfCorePrintln("SW2: 1 time");
				turnOnLed(LED2,ledBlue);
				sendOnOffStateReport(2, LED_ON);
			}
		break;

		case press_2:
			if(byButton == SW_1)
			{
				emberAfCorePrintln("SW1: 2 times");
				sendOnOffStateReport(1, LED_OFF);
				turnOffRGBLed(LED1);
				sendBindingInitToTarget(SOURCE_ENDPOINT_PRIMARY,
										 DESTINATTION_ENDPOINT,
										 false,
										 HC_NETWORK_ADDRESS);
			}
			else
			{
				emberAfCorePrintln("SW2: 2 time");
				turnOffRGBLed(LED2);
				sendOnOffStateReport(2, LED_OFF);
			}
			break;

		case press_3:
			if(byButton == SW_1)
			{
				emberAfCorePrintln("SW1: 3 time");
				emberAfPluginFindAndBindTargetStart(1);
				toggleLed(LED2,ledyellow,3,200,200);
			}
			else
			{
				emberAfCorePrintln("SW2: 3 time");
				emberAfPluginFindAndBindInitiatorStart(1);
				toggleLed(LED2,ledRGB,3,200,200);
			}
			break;

		case press_4:
			if(byButton == SW_1)
			{
				emberAfCorePrintln("SW1: 4 time");
			}
			else
			{
				emberAfCorePrintln("SW2: 4 time");
			}
			break;

		case press_5:
			if(byButton == SW_1)
			{
				emberAfCorePrintln("SW1: 5 time");
			}
			else
			{
				emberAfCorePrintln("SW2: 5 time");
				toggleLed(LED1,ledRed, 2, 200, 200);
				g_SystemState = REBOOT_STATE;
				emberEventControlSetDelayMS(mainStateEventControl,3000);
			}
			break;

		default:
			break;
		}
}

/*
 * @func	Main_ButtonHoldCallbackHandler
 * @brief	Handle Event button holding callback
 * @param	button, holdingHandler
 * @retval	None
 */
void mainButtonHoldCallbackHandler(uint8_t byButton, ButtonEvent_e holdingHandler)
{
//	emberAfCorePrintln("SW %d HOLDING %d s\n",button+1,holdingHandler-press_max);
	switch(holdingHandler)
	{
		case hold_1s:
			emberAfCorePrintln("Holding: 1 s");
			break;

		case hold_2s:
			emberAfCorePrintln("Holding: 2 s");
			break;

		case hold_3s:
			emberAfCorePrintln("Holding: 3 s");
			break;

		case hold_4s:
			emberAfCorePrintln("Holding: 4 s");
			break;

		case hold_5s:
			emberAfCorePrintln("Holding: 5 s");
			break;

		default:
			break;
	}
}

/*
 * @func	mainStateEventHandler
 * @brief	Handle Event State Network
 * @param	None
 * @retval	None
 */
void mainStateEventHandler(void)
{
	emberEventControlSetInactive(mainStateEventControl);
	EmberNetworkStatus byNwkStatusCurrent;
	switch (g_SystemState) {
		case POWER_ON_STATE:
			byNwkStatusCurrent = emberAfNetworkState();
			if(byNwkStatusCurrent == EMBER_NO_NETWORK)
			{
				toggleLed(LED1,ledRed,3,200,200);
				emberEventControlSetInactive(findNetworkControl);
				emberEventControlSetActive(findNetworkControl);
			}
			g_SystemState = IDLE_STATE;
			break;

		case REPORT_STATE:
			g_SystemState = IDLE_STATE;
			sendReportInfoHc();
			break;

		case IDLE_STATE:
				emberAfCorePrintln("IDLE_STATE");
			break;

		case REBOOT_STATE:
			g_SystemState = IDLE_STATE;
			EmberNetworkStatus networkStatus = emberAfNetworkState();
			if (networkStatus != EMBER_NO_NETWORK) {
				sendZigDevRequest();
				emberClearBindingTable();
				emberLeaveNetwork();
			} else {
				halReboot();
			}
			break;
		default:
			break;
	}
}

/*
 * @func	Main_networkEventHandler
 * @brief	Handler Event Result Network
 * @param	networkResult
 * @retval	None
 */
void mainNetworkEventHandler(uint8_t byNetworkResult)
{
	emberAfCorePrintln("Network Event Handle");
	switch (byNetworkResult) {
		case NETWORK_HAS_PARENT:
			emberAfCorePrintln("Network has parent");
			toggleLed(LED1, ledPink, 3, 300, 300);
			g_boNetworkReady = true;
			g_SystemState = REPORT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_JOIN_FAIL:
			g_SystemState = IDLE_STATE;
			toggleLed(LED1, ledBlue, 3, 300, 300);
			emberAfCorePrintln("Network Join Fail");
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_JOIN_SUCCESS:
			emberEventControlSetInactive(findNetworkControl);
			emberAfCorePrintln("Network Join Success");
			toggleLed(LED1, ledPink, 3, 300, 300);
			g_boNetworkReady =true;
			g_SystemState = REPORT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_LOST_PARENT:
			emberAfCorePrintln("Network lost parent");
			toggleLed(LED1, ledPink, 3, 300, 300);
			g_SystemState = IDLE_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;

		case NETWORK_OUT_NETWORK:
			if(g_boNetworkReady)
			{
				toggleLed(LED1, ledRed, 3, 300, 300);
				g_SystemState = REBOOT_STATE;
				emberEventControlSetDelayMS(mainStateEventControl, 3000);
			}
			break;

		default:
			break;
	}
}

/**
 * @func   FindNetworkHandler
 * @brief  Handle Event Find Network
 * @param  None
 * @retval None
 */
void findNetworkHandler(void)
{
	emberEventControlSetInactive(findNetworkControl);
	networkFindAndJoin();
	emberEventControlSetDelayMS(findNetworkControl, 10000);
}

/**
 * @func   LightSensor_Read_1timeHandler
 * @brief  Read value from ADC
 * @param  None
 * @retval None
 */
void LightSensor_Read_1timeHandler(void)
{
	emberEventControlSetInactive(lightSensorRead1timeControl);
	g_iluxFirsttime = LightSensor_AdcPollingRead();

	if(abs(g_iluxSecondtimes - g_iluxFirsttime) > 30)
		{
		g_iluxSecondtimes = g_iluxFirsttime;
		sendLDRStateReport(3,g_iluxSecondtimes);
		emberAfCorePrintln("Light:   %d lux         ",g_iluxSecondtimes);
			if(g_iluxSecondtimes > 500)
			{
				turnOnLed(LED2,ledGreen);
			}
			else
			{
				turnOffRGBLed(LED2);
			}
		}
	emberEventControlSetDelayMS(lightSensorRead1timeControl,PERIOD_SCAN_SENSORLIGHT);
}

/**
 * @func    ReadValueTempHumiHandler
 * @brief   Event Sensor Handler
 * @param   None
 * @retval  None
 */
void emberIncomingManyToOneRouteRequestHandler(EmberNodeId iSource,
                                               EmberEUI64 byLongId,
                                               uint8_t byCost)
{
	//hanlde for MTORRs
	emberAfCorePrintln("Receive MTORRs");
	emberEventControlSetInactive(MTORRsEventControl);
	emberEventControlSetDelayMS(MTORRsEventControl, 2* ((uint8_t)halCommonGetRandom())); //1-2p
}

/*
 * @func	MTORRsEventHandler
 * @brief	Read Status
 * @param	None
 * @retval	None
 */
void MTORRsEventHandler(void)
{
	emberEventControlSetInactive(MTORRsEventControl);
	uint8_t byData;

	EmberAfStatus status = emberAfReadServerAttribute(RGB1_ENDPOINT,
												ZCL_ON_OFF_CLUSTER_ID,
												ZCL_ON_OFF_ATTRIBUTE_ID,
												&byData, 1);
	if(status == EMBER_ZCL_STATUS_SUCCESS){
		sendOnOffStateReport(RGB1_ENDPOINT,byData);
	}

	EmberAfStatus status1 = emberAfReadServerAttribute(RGB2_ENDPOINT,
													ZCL_ON_OFF_CLUSTER_ID,
													ZCL_ON_OFF_ATTRIBUTE_ID,
													&byData, 1);
	if(status1 == EMBER_ZCL_STATUS_SUCCESS){
		sendOnOffStateReport(RGB2_ENDPOINT,byData);
	}
}
