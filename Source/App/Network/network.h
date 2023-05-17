 /* File name: main.h
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

#ifndef SOURCE_APP_MAIN_MAIN_H_
#define SOURCE_APP_MAIN_MAIN_H_

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/

/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

#define RGB1_ENDPOINT				1
#define RGB2_ENDPOINT				2

typedef enum{
	POWER_ON_STATE,
	REPORT_STATE,
	IDLE_STATE,
	REBOOT_STATE
}SystemState_e;

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

#endif /* SOURCE_APP_MAIN_MAIN_H_ */
