 /* File name: led.c
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
#include "em_timer.h"
#include "led.h"


/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/

typedef struct {
  GPIO_Port_TypeDef			port;
  unsigned int				pin;
  bool						ledBlinkMode;
  LedColor_e				color;
  uint32_t					onTime;
  uint32_t					offTime;
  uint8_t					blinkTime;
}LedArray_t;

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

EmberEventControl led1ToggleEventControl,led2ToggleEventControl;
EmberEventControl *ledEventControl[LED_RGB_COUNT];

LedArray_t ledArray[LED_RGB_COUNT][LED_RGB_ELEMENT] = {LED_RGB_1, LED_RGB_2};

LedArray_t ledAction[LED_RGB_COUNT];

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/

/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

/******************************************************************************/
/**
 * @func    ledInit
 * @brief   Initialize LED
 * @param   None
 * @retval  None
 */
void ledInit(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	for(int i = 0;i <LED_RGB_COUNT;i++){

		for(int j = 0; j< LED_RGB_ELEMENT;j++){

			GPIO_PinModeSet(ledArray[i][j].port, ledArray[i][j].pin, gpioModePushPull, 0);
		}
	}
	turnOffRGBLed(LED1);
	turnOffRGBLed(LED2);
	ledEventControl[LED1] =(EmberEventControl *) &led1ToggleEventControl;
	ledEventControl[LED2] =(EmberEventControl *) &led2ToggleEventControl;
}

/**
 * @func    turnOffRGBLed
 * @brief   turn off RGB led
 * @param   ledNumber_e
 * @retval  None
 */
void turnOffRGBLed(LedNumber_e index)
{
	for(int j=0;j<LED_RGB_ELEMENT;j++){
		GPIO_PinOutSet(ledArray[index][j].port, ledArray[index][j].pin);
	}
}

/**
 * @func    turnOnLed
 * @brief   turn on led with option color
 * @param   ledNumber_e, ledColor_e
 * @retval  None
 */
void turnOnLed(LedNumber_e index, LedColor_e color)
{
	for(int j=0;j<LED_RGB_ELEMENT;j++){

		if(((color >> j) & 0x01) == 1){

			GPIO_PinOutClear(ledArray[index][j].port, ledArray[index][j].pin);
		}else {

			GPIO_PinOutSet(ledArray[index][j].port, ledArray[index][j].pin);
		}
	}
}

/**
 * @func    toggleLed
 * @brief   toggled LED
 * @param   ledNumber_e, ledColor_e, uint8_t, uint32_t
 * @retval  None
 */
void toggleLed(LedNumber_e ledIndex, LedColor_e color, uint8_t byToggleTime, uint32_t byOnTimeMs, uint32_t byOffTimeMs)
{
	ledAction[ledIndex].ledBlinkMode = LED_TOGGLE;
	ledAction[ledIndex].color = color;
	ledAction[ledIndex].onTime = byOnTimeMs;
	ledAction[ledIndex].offTime = byOffTimeMs;
	ledAction[ledIndex].blinkTime = byToggleTime*2;

	emberEventControlSetActive(*ledEventControl[ledIndex]);
}

/**
 * @func    toggleLedHandle
 * @brief   Event Led Handler
 * @param   ledNumber_e
 * @retval  None
 */
void toggleLedHandle(LedNumber_e ledIndex)
{
	if(ledAction[ledIndex].blinkTime !=0){

		if(ledAction[ledIndex].blinkTime % 2){

			for( int i = 0; i<LED_RGB_ELEMENT; i++){

				if(((ledAction[ledIndex].color >> i ) & 0x01) == 1 ){

					GPIO_PinOutClear(ledArray[ledIndex][i].port, ledArray[ledIndex][i].pin);
				}else {

					GPIO_PinOutSet(ledArray[ledIndex][i].port, ledArray[ledIndex][i].pin);
				}
			}
			emberEventControlSetDelayMS(*ledEventControl[ledIndex], ledAction[ledIndex].onTime);
		}else {

			for( int j = 0; j<LED_RGB_ELEMENT; j++){

				GPIO_PinOutSet(ledArray[ledIndex][j].port, ledArray[ledIndex][j].pin);
			}
			emberEventControlSetDelayMS(*ledEventControl[ledIndex], ledAction[ledIndex].offTime);
		}
		ledAction[ledIndex].blinkTime--;
	}else {

		ledAction[ledIndex].ledBlinkMode = LED_FREE;
		for( int j = 0; j<LED_RGB_ELEMENT; j++){

			GPIO_PinOutSet(ledArray[ledIndex][j].port, ledArray[ledIndex][j].pin);
		}
	}
}

/**
 * @func    led1ToggleEventHandle
 * @brief   Event Led Handler
 * @param   None
 * @retval  None
 */
void led1ToggleEventHandle(void)
{
	emberEventControlSetInactive(led1ToggleEventControl);
	switch(ledAction[LED1].ledBlinkMode)
	{
		case LED_TOGGLE:
			toggleLedHandle(LED1);
			break;

		default:
			break;
	}
}

/**
 * @func    led2ToggleEventHandle
 * @brief   Event Led Handler
 * @param   None
 * @retval  None
 */
void led2ToggleEventHandle(void)
{
	emberEventControlSetInactive(led2ToggleEventControl);
	switch(ledAction[LED2].ledBlinkMode)
	{
		case LED_TOGGLE:
			toggleLedHandle(LED2);
			break;

		default:
			break;
	}
}
