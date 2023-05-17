 /* File name: button.c
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
#include "button.h"

/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/


/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/


/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

EmberEventControl buttonPressAndHoldEventControl;
EmberEventControl buttonReleaseEventControl;

BUTTON_holdingEvent_t g_holdingCallbackFunc = NULL;
BUTTON_pressEvent_t g_pressAndHoldingCallbackFunc = NULL;
Button_t g_buttonArray[BUTTON_COUNT] = BUTTON_INIT;

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/

/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/

void buttonPressAndHoldEventHandle(void);
void buttonReleaseEventHandle(void);

static void 	halInternalButtonIsr(uint8_t pin);
static void 	resetButtonParameter(uint8_t index);
static uint8_t 	getButtonIndex(uint8_t pin);

/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

/******************************************************************************/
/*
 * @func	buttonInit
 * @brief	Initialize Button
 * @param	holdingHandle, pressHandler
 * @retval	None
 */
void buttonInit(BUTTON_holdingEvent_t holdingHandle,BUTTON_pressEvent_t pressHandler)
{
  GPIOINT_Init();

  CMU_ClockEnable(cmuClock_GPIO, true);
  uint8_t i;
  for ( i = 0; i < BUTTON_COUNT; i++ ) {
    /* Configure pin as input */
    GPIO_PinModeSet(g_buttonArray[i].port,
    		g_buttonArray[i].pin,
					gpioModeInput,
					GPIO_DOUT);
    /* Register callbacks before setting up and enabling pin interrupt. */
    GPIOINT_CallbackRegister(g_buttonArray[i].pin,
                             halInternalButtonIsr);
    /* Set rising and falling edge interrupts */
    GPIO_ExtIntConfig(g_buttonArray[i].port,
    		g_buttonArray[i].pin,
			g_buttonArray[i].pin,
                      true,
                      true,
                      true);
  }

  g_holdingCallbackFunc = holdingHandle;
  g_pressAndHoldingCallbackFunc = pressHandler;

}
/*
 * @func	halInternalButtonIsr
 * @brief	IRQ button
 * @param	pin
 * @retval	None
 */
void halInternalButtonIsr(uint8_t byPin)
{
  uint8_t byButtonStateNow;
  uint8_t byButtonStatePrev;
  uint32_t dwDebounce;
  uint8_t byButtonIndex;

  byButtonIndex = getButtonIndex(byPin);
  // check valid index
  if(byButtonIndex==-1)
	  return;

  byButtonStateNow = GPIO_PinInGet(g_buttonArray[byButtonIndex].port, g_buttonArray[byButtonIndex].pin);
  for ( dwDebounce = 0;
        dwDebounce < BUTTON_DEBOUNCE;
        dwDebounce = (byButtonStateNow == byButtonStatePrev) ? dwDebounce + 1 : 0 )
  {
	byButtonStatePrev = byButtonStateNow;
    byButtonStateNow = GPIO_PinInGet(g_buttonArray[byButtonIndex].port, g_buttonArray[byButtonIndex].pin);
  }

  g_buttonArray[byButtonIndex].state = byButtonStateNow;

  if(byButtonStateNow == BUTTON_PRESS)
  {
	  g_buttonArray[byButtonIndex].pressCount++;
	  if(g_buttonArray[byButtonIndex].press != true)
	  {
		  emberEventControlSetActive(buttonPressAndHoldEventControl);
	  }

	  g_buttonArray[byButtonIndex].isHolding=false;
	  g_buttonArray[byButtonIndex].holdTime=0;
	  g_buttonArray[byButtonIndex].press = true;
	  g_buttonArray[byButtonIndex].release = false;
  }
  else
  {
	  g_buttonArray[byButtonIndex].release = true;
	  g_buttonArray[byButtonIndex].press = false;
	  emberEventControlSetInactive(buttonReleaseEventControl);
	  emberEventControlSetDelayMS(buttonReleaseEventControl,BUTTON_CHECK_RELEASE_MS);
  }
}

/*
 * @func	buttonPressAndHoldEventHandle
 * @brief	Event Button Handler
 * @param	None
 * @retval	None
 */
void buttonPressAndHoldEventHandle(void)
{
	emberEventControlSetInactive(buttonPressAndHoldEventControl);
	bool boHoldTrigger = false;
	for(int i=0; i<BUTTON_COUNT; i++)
	{
		if(g_buttonArray[i].press ==true)
		{
			boHoldTrigger = true;
			g_buttonArray[i].holdTime++;
			if(g_buttonArray[i].holdTime>=5)
			{
				g_buttonArray[i].isHolding=true;
				g_buttonArray[i].pressCount=0;
			}

			if(g_holdingCallbackFunc != NULL)
			{
				if((g_buttonArray[i].holdTime %5)==0)
				{
					g_holdingCallbackFunc(i,g_buttonArray[i].holdTime/5 + press_max);
				}
			}
		}
	}
	if(boHoldTrigger == true)
		emberEventControlSetDelayMS(buttonPressAndHoldEventControl,BUTTON_CHECK_HOLD_CYCLES_MS);
}

/*
 * @func	buttonReleaseEventHandle
 * @brief	Event Button Handler
 * @param	button, holdingHandler
 * @retval	None
 */
void buttonReleaseEventHandle(void)
{
	emberEventControlSetInactive(buttonReleaseEventControl);
	for(int i=0; i<BUTTON_COUNT; i++)
	{
		if(g_buttonArray[i].release == true)
		{
			if(g_pressAndHoldingCallbackFunc != NULL)
			{
				if(g_buttonArray[i].isHolding==false)
				{
					g_pressAndHoldingCallbackFunc(i, g_buttonArray[i].pressCount >= press_max ? unknown:g_buttonArray[i].pressCount);
				}else
				{
					g_holdingCallbackFunc(i, (g_buttonArray[i].holdTime/5 + press_max) >= hold_max ? unknown :(g_buttonArray[i].holdTime/5 + press_max));
				}
			}

			resetButtonParameter(i);
		}
	}
}

/*
 * @func	resetButtonParameter
 * @brief	Reset parameter
 * @param	index
 * @retval	None
 */
static void resetButtonParameter(uint8_t byIndex)
{
	g_buttonArray[byIndex].holdTime 	= 0;
	g_buttonArray[byIndex].pressCount	= 0;
	g_buttonArray[byIndex].press		= false;
	g_buttonArray[byIndex].release		= false;
	g_buttonArray[byIndex].state		= 0;
	g_buttonArray[byIndex].isHolding    =false;
}

/*
 * @func	getButtonIndex
 * @brief	get number button
 * @param	pin
 * @retval	None
 */
static uint8_t getButtonIndex(uint8_t byPin)
{
	for(int i=0; i<BUTTON_COUNT; i++)
	{
		if(g_buttonArray[i].pin == byPin)
			return i;
	}
	return -1;
}
