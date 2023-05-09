/*
 * led.h
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#ifndef SOURCE_MID_LED_LED_H_
#define SOURCE_MID_LED_LED_H_

/******************************************************************************/
/*                     		DEFINITIONS            				              */
/******************************************************************************/
#define LED_RGB_COUNT				2
#define LED_RGB_ELEMENT				3

#define LED_RGB_DIM_T				10000

#define LED_PORT_1					gpioPortA
#define LED_BLUE_PIN_1				(0U)
#define LED_GREEN_PIN_1				(3U)
#define LED_RED_PIN_1				(4U)

#define LED_PORT_2					gpioPortD
#define LED_BLUE_PIN_2				(2U)
#define LED_GREEN_PIN_2				(1U)
#define LED_RED_PIN_2				(0U)

#define LED_RGB_1                       { { LED_PORT_1, LED_RED_PIN_1 }, { LED_PORT_1, LED_GREEN_PIN_1 }, { LED_PORT_1, LED_BLUE_PIN_1 } }
#define LED_RGB_2                       { { LED_PORT_2, LED_RED_PIN_2 }, { LED_PORT_2, LED_GREEN_PIN_2 }, { LED_PORT_2, LED_BLUE_PIN_2 } }

#define TIMER_DIM_INIT                                                           \
{                                                                                   \
    false,                 /* Enable timer when initialization completes. */           \
    false,                /* Stop counter during debug halt. */                       \
    timerPrescale1,       /* No prescaling. */                                        \
    timerClkSelHFPerClk,  /* Select HFPER / HFPERB clock. */                          \
    false,                /* Not 2x count mode. */                                    \
    false,                /* No ATI. */                                               \
    timerInputActionNone, /* No action on falling input edge. */                      \
    timerInputActionNone, /* No action on rising input edge. */                       \
    timerModeUp,          /* Up-counting. */                                          \
    false,                /* Do not clear DMA requests when DMA channel is active. */ \
    false,                /* Select X2 quadrature decode mode (if used). */           \
    false,                /* Disable one shot. */                                     \
    false                 /* Not started/stopped/reloaded by other timers. */         \
}
typedef enum{
	LED1,
	LED2
}ledNumber;
typedef enum
{
	ledOff 				= 0x000,
	ledRed				= BIT(0),
	ledGreen			= BIT(1),
	ledBlue				= BIT(2),
	ledPink				= ledRed  | ledBlue,
	ledyellow			= ledGreen| ledRed,
	ledRGB				= ledRed  | ledGreen | ledBlue
}ledColor_e;

typedef enum
{
	red,
	green,
	blue,
	off
}ledState_e;

enum{
	LED_FREE,
	LED_TOGGLE
};

/* Function prototypes -----------------------------------------------*/
void ledInit(void);
void turnOffRBGLed(ledNumber index);
void toggleLedHandle(ledNumber ledIndex);
void toggleLed(ledNumber ledIndex, ledColor_e color, uint8_t toggleTime, uint32_t onTimeMs, uint32_t offTimeMs);
void turnOnLed(ledNumber index, ledColor_e color);

#endif /* SOURCE_MID_LED_LED_H_ */
