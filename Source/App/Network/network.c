 /* File name: network.c
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
#include "network.h"

/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/

EmberEventControl joinNetworkEventControl;
networkEventHandler networkEventHandle = NULL;
uint32_t dwTimeFindAndJoin = 0;

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
/*
 * @function 			: Network_Init
 * @brief				: Handle event network.
 * @parameter			: networkEventHandler
 * @return value		: None
 */
void networkInit(networkEventHandler networkResult)
{
	networkEventHandle = networkResult;
}

/*
 * @function 			: networkFindAndJoin
 * @brief				: Find network
 * @parameter			: None
 * @return value		: None
 */
void networkFindAndJoin(void)
{
	if(emberAfNetworkState() == EMBER_NO_NETWORK)
	{
		emberEventControlSetDelayMS(joinNetworkEventControl, 2000);
	}
}

/*
 * @function 			: networkStopFindAndJoin
 * @brief				: Stop find network
 * @parameter			: None
 * @return value		: None
 */
void networkStopFindAndJoin(void)
{
	emberAfPluginNetworkSteeringStop();
}

/*
 * @function 			: joinNetworkEventHandler
 * @brief				: Handle Event Join network
 * @parameter			: None
 * @return value		: None
 */
void joinNetworkEventHandler(void)
{
	emberEventControlSetInactive(joinNetworkEventControl);

	if(emberAfNetworkState() == EMBER_NO_NETWORK)
	{
		emberAfPluginNetworkSteeringStart();
		dwTimeFindAndJoin++;
	}
}

/*
 * @function 			: emberAfStackStatusCallback
 * @brief				: Stack Status
 * @parameter			: EmberStatus
 * @return value		: True or false
 */
boolean emberAfStackStatusCallback(EmberStatus byStatus)
{
	emberAfCorePrintln("emberAfStackStatusCallback\n");

	if(byStatus == EMBER_NETWORK_UP)
	{
		if(dwTimeFindAndJoin>0)// vao mang thanh cong
		{
			networkStopFindAndJoin();
			if(networkEventHandle != NULL)
			{
				networkEventHandle(NETWORK_JOIN_SUCCESS);
				emberAfCorePrintln("NETWORK_JOIN_SUCCESS");
			}
		}else
		{
			if(networkEventHandle != NULL)
			{
				networkEventHandle(NETWORK_HAS_PARENT);
				emberAfCorePrintln("NETWORK_HAS_PARENT");
			}
		}

	}
	else
	{
		EmberNetworkStatus byNwkStatusCurrent = emberAfNetworkState();
		if(byNwkStatusCurrent == EMBER_NO_NETWORK)
		{
			if(networkEventHandle != NULL)
			{
				networkEventHandle(NETWORK_OUT_NETWORK);
				emberAfCorePrintln("NETWORK_OUT_NETWORK");
			}
		}
		else if(byNwkStatusCurrent == EMBER_JOINED_NETWORK_NO_PARENT){
			emberAfCorePrintln("NETWORK_LOST_PARENT");
			networkEventHandle(NETWORK_LOST_PARENT);
		}
	}
	if(byStatus == EMBER_JOIN_FAILED)
	{
		emberAfCorePrintln("NETWORK_JOIN_FAIL");
		networkEventHandle(NETWORK_JOIN_FAIL);
	}
	return false;
}
