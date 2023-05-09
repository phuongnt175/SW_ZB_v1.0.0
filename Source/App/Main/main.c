/*
 * main.c
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#include "app/framework/include/af.h"
#include "Source/Mid/Led/led.h"
#include "Source/Mid/Button/button.h"
#include "Source/Mid/Kalman_filter/kalman_filter.h"
#include "Source/Mid/LDR/ldr.h"
#include "protocol/zigbee/stack/include/binding-table.h"
#include "Source/App/Network/network.h"
#include "Source/App/Send/send.h"
#include "Source/App/Receive/receive.h"
#include "main.h"
#include "math.h"

#define PERIOD_SCAN_SENSORLIGHT									1000 	//	ms

bool networkReady = false;
systemState system_State = POWER_ON_STATE;
uint32_t lux_1time = 0;
uint32_t lux_2times = 0;


void Main_ButtonPressCallbackHandler(uint8_t button, BUTTON_Event_t pressHandler);
void Main_ButtonHoldCallbackHandler(uint8_t button, BUTTON_Event_t holdingHandler);
void Main_networkEventHandler(uint8_t networkResult);
/* Event **************************************************************/
EmberEventControl LightSensor_Read_1timeControl;
EmberEventControl mainStateEventControl;
EmberEventControl MTORRsEventControl;
EmberEventControl FindNetworkControl;
/** @brief Main Init
 *
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup.
 * Any code that you would normally put into the top of the application's
 * main() routine should be put into this function.
        Note: No callback
 * in the Application Framework is associated with resource cleanup. If you
 * are implementing your application on a Unix host where resource cleanup is
 * a consideration, we expect that you will use the standard Posix system
 * calls, including the use of atexit() and handlers for signals such as
 * SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If you use the signal()
 * function to register your signal handler, please mind the returned value
 * which may be an Application Framework function. If the return value is
 * non-null, please make sure that you call the returned function from your
 * handler to avoid negating the resource cleanup of the Application Framework
 * itself.
 *
 */
void emberAfMainInitCallback(void)
{
	emberAfCorePrintln("Main Init");
	ledInit();
	buttonInit(Main_ButtonHoldCallbackHandler, Main_ButtonPressCallbackHandler);
	Network_Init(Main_networkEventHandler);
	emberEventControlSetActive(mainStateEventControl);
	LDRInit();
	KalmanFilterInit(2, 2, 0.001); // Initialize Kalman filter
	emberEventControlSetDelayMS(LightSensor_Read_1timeControl, 1000);
}


/****************************EVENT HANDLER MIDDER********************************************************************/
/*
 * @func	Main_ButtonPressCallbackHandler
 * @brief	Event Button Handler
 * @param	button, pressHandler
 * @retval	None
 */
void Main_ButtonPressCallbackHandler(uint8_t button, BUTTON_Event_t pressHandler)
{
	switch(pressHandler)
		{
		case press_1:
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 1 time");
				turnOnLed(LED1,ledBlue);
				SEND_OnOffStateReport(1, LED_ON);
				SEND_BindingInitToTarget(SOURCE_ENDPOINT_PRIMARY,
										 DESTINATTION_ENDPOINT,
										 true,
										 HC_NETWORK_ADDRESS);
			}
			else
			{
				emberAfCorePrintln("SW2: 1 time");
				turnOnLed(LED2,ledBlue);
				SEND_OnOffStateReport(2, LED_ON);

			}
		break;
		case press_2:
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 2 times");
				SEND_OnOffStateReport(1, LED_OFF);
				turnOffRBGLed(LED1);
				SEND_BindingInitToTarget(SOURCE_ENDPOINT_PRIMARY,
										 DESTINATTION_ENDPOINT,
										 false,
										 HC_NETWORK_ADDRESS);
			}
			else
			{
				emberAfCorePrintln("SW2: 2 time");
				turnOffRBGLed(LED2);
				SEND_OnOffStateReport(2, LED_OFF);

			}
			break;
		case press_3:
			if(button == SW_1)
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
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 4 time");


			}
			else
			{
				emberAfCorePrintln("SW2: 4 time");
			}
			break;
		case press_5:
			if(button == SW_1)
			{
				emberAfCorePrintln("SW1: 5 time");

			}
			else
			{
				emberAfCorePrintln("SW2: 5 time");
				toggleLed(LED1,ledRed, 2, 200, 200);
				system_State = REBOOT_STATE;
				emberEventControlSetDelayMS(mainStateEventControl,3000);
			}
			break;
		default:
			break;
		}
}

/*
 * @func	Main_ButtonHoldCallbackHandler
 * @brief	Event Button Handler
 * @param	button, holdingHandler
 * @retval	None
 */
void Main_ButtonHoldCallbackHandler(uint8_t button, BUTTON_Event_t holdingHandler)
{
//	emberAfCorePrintln("SW %d HOLDING %d s\n",button+1,holdingHandler-press_max);
	switch(holdingHandler)
	{
	case hold_1s:
		emberAfCorePrintln("SW1: 1 s");
		break;
	case hold_2s:
		emberAfCorePrintln("SW1: 2 s");
		break;
	case hold_3s:
		emberAfCorePrintln("SW1: 3 s");
		break;
	case hold_4s:
		emberAfCorePrintln("SW1: 4 s");
		break;
	case hold_5s:
		emberAfCorePrintln("SW1: 5 s");
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
	EmberNetworkStatus nwkStatusCurrent;
	switch (system_State) {
		case POWER_ON_STATE:
			nwkStatusCurrent = emberAfNetworkState();
			if(nwkStatusCurrent == EMBER_NO_NETWORK)
			{
				toggleLed(LED1,ledRed,3,200,200);
				emberEventControlSetInactive(FindNetworkControl);
				emberEventControlSetActive(FindNetworkControl);
			}
			system_State = IDLE_STATE;
			break;
		case REPORT_STATE:
			system_State = IDLE_STATE;
			SEND_ReportInfoHc();
			break;
		case IDLE_STATE:
				emberAfCorePrintln("IDLE_STATE");
			break;
		case REBOOT_STATE:
			system_State = IDLE_STATE;
			EmberNetworkStatus networkStatus = emberAfNetworkState();
			if (networkStatus != EMBER_NO_NETWORK) {
				SendZigDevRequest();
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
void Main_networkEventHandler(uint8_t networkResult)
{
	emberAfCorePrintln("Network Event Handle");
	switch (networkResult) {
		case NETWORK_HAS_PARENT:
			emberAfCorePrintln("Network has parent");
			toggleLed(LED1,ledPink,3,300,300);
			networkReady = true;
			system_State = REPORT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		case NETWORK_JOIN_FAIL:
			system_State = IDLE_STATE;
			toggleLed(LED1,ledBlue,3,300,300);
			emberAfCorePrintln("Network Join Fail");
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		case NETWORK_JOIN_SUCCESS:
			emberEventControlSetInactive(FindNetworkControl);
			emberAfCorePrintln("Network Join Success");
			toggleLed(LED1,ledPink,3,300,300);
			networkReady =true;
			system_State = REPORT_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		case NETWORK_LOST_PARENT:
			emberAfCorePrintln("Network lost parent");
			toggleLed(LED1,ledPink,3,300,300);
			system_State = IDLE_STATE;
			emberEventControlSetDelayMS(mainStateEventControl, 1000);
			break;
		case NETWORK_OUT_NETWORK:
			if(networkReady)
			{
				toggleLed(LED1,ledRed,3,300,300);
				system_State = REBOOT_STATE;
				emberEventControlSetDelayMS(mainStateEventControl, 3000);
			}
			break;
		default:
			break;
	}
}

void FindNetworkHandler(void)
{
	emberEventControlSetInactive(FindNetworkControl);
	NETWORK_FindAndJoin();
	emberEventControlSetDelayMS(FindNetworkControl, 10000);

}
/**
 * @func   LightSensor_Read_1timeHandler
 * @brief  Read value from ADC
 * @param  None
 * @retval None
 */
void LightSensor_Read_1timeHandler(void)
{
	emberEventControlSetInactive(LightSensor_Read_1timeControl);
	lux_1time = LightSensor_AdcPollingRead();
	if(abs(lux_2times - lux_1time) > 30)
		{
		lux_2times = lux_1time;
		SEND_LDRStateReport(3,lux_2times);
		emberAfCorePrintln("Light:   %d lux         ",lux_2times);
			if(lux_2times > 500)
			{
				turnOnLed(LED2,ledGreen);
			}
			else
			{
				turnOffRBGLed(LED2);
			}
		}
	emberEventControlSetDelayMS(LightSensor_Read_1timeControl,PERIOD_SCAN_SENSORLIGHT);
}
/**
 * @func    ReadValueTempHumiHandler
 * @brief   Event Sensor Handler
 * @param   None
 * @retval  None
 */

void emberIncomingManyToOneRouteRequestHandler(EmberNodeId source,
                                               EmberEUI64 longId,
                                               uint8_t cost)
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
	uint8_t data;
	EmberAfStatus status = emberAfReadServerAttribute(RGB1_ENDPOINT,
											   ZCL_ON_OFF_CLUSTER_ID,
											   ZCL_ON_OFF_ATTRIBUTE_ID,
											   &data,
											   1);
	if(status == EMBER_ZCL_STATUS_SUCCESS){
		SEND_OnOffStateReport(RGB1_ENDPOINT,data);
	}

	EmberAfStatus status1 = emberAfReadServerAttribute(RGB2_ENDPOINT,
												   ZCL_ON_OFF_CLUSTER_ID,
												   ZCL_ON_OFF_ATTRIBUTE_ID,
												   &data,
												   1);
		if(status1 == EMBER_ZCL_STATUS_SUCCESS){
			SEND_OnOffStateReport(RGB2_ENDPOINT,data);
		}
}
