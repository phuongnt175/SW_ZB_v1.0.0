 /* File name: button.h
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

#ifndef SOURCE_MID_BUTTON_BUTTON_H_
#define SOURCE_MID_BUTTON_BUTTON_H_

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

#include "stddef.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "gpiointerrupt.h"

/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/


/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define BUTTON_COUNT					2
#define GPIO_DOUT						0
#define BUTTON_DEBOUNCE					5
#define BUTTON_CHECK_HOLD_CYCLES_MS		200
#define BUTTON_CHECK_RELEASE_MS			500

#define HOLD_TIME_MIN					5

#define BUTTON_1_PORT				gpioPortD
#define BUTTON_1_PIN				(4U)

#define BUTTON_2_PORT				gpioPortD
#define BUTTON_2_PIN				(3U)

#define BUTTON_INIT					{{BUTTON_1_PORT,BUTTON_1_PIN},{BUTTON_2_PORT,BUTTON_2_PIN}}

typedef enum
{
	press_1	= 1,
	press_2 ,
	press_3 ,
	press_4 ,
	press_5,
	press_max = 10,
	hold_1s	,
	hold_2s	,
	hold_3s	,
	hold_4s	,
	hold_5s ,
	hold_max,
	unknown,
}ButtonEvent_e;

enum
{
	SW_1,
	SW_2
};

enum
{
	BUTTON_PRESS,
	BUTTON_RELEASE
};

typedef struct {
  GPIO_Port_TypeDef		port;
  unsigned int			pin;
  boolean				state;
  uint8_t				pressCount;
  uint8_t				holdTime;
  boolean				release;
  boolean				press;
  boolean               isHolding;
} Button_t;

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

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

typedef void (*BUTTON_holdingEvent_t)(uint8_t btIndex, ButtonEvent_e holdEvent );
typedef void (*BUTTON_pressEvent_t)(uint8_t btIndex, ButtonEvent_e pressEvent);

void buttonInit(BUTTON_holdingEvent_t holdingHandler,BUTTON_pressEvent_t pressHandler);

#endif /* SOURCE_MID_BUTTON_BUTTON_H_ */
