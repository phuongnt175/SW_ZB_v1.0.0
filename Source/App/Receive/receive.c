 /* File name: receive.c
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
#include "Source/Mid/LED/led.h"
#include "Source/App/Send/send.h"
#include "receive.h"

/******************************************************************************/
/*                     PRIVATE TYPES and DEFINITIONS                         */
/******************************************************************************/

/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/

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
/**
 * @func    emberAfPreCommandReceivedCallback
 * @brief   Process Command Received
 * @param   EmberAfClusterCommand
 * @retval  boolean
 */
boolean emberAfPreCommandReceivedCallback(EmberAfClusterCommand* pCmd)
{
	if(pCmd->clusterSpecific)
	{
		switch(pCmd->apsFrame->clusterId)
		{
			case ZCL_ON_OFF_CLUSTER_ID:
				receiveHandleOnOffCluster(pCmd);
				break;
			case ZCL_LEVEL_CONTROL_CLUSTER_ID:
				receiveHandleLevelControlCluster(pCmd);
				break;
			default:
				break;
		}
	}
	return false;
}

/**
 * @func    emberAfPreMessageReceivedCallback
 * @brief   Process Pre message received
 * @param   EmberAfIncomingMessage
 * @retval  None
 */
boolean emberAfPreMessageReceivedCallback(EmberAfIncomingMessage* pIncommingMessage)
{
	if(pIncommingMessage->apsFrame->clusterId == ACTIVE_ENDPOINTS_RESPONSE)
	{
		return true;
	}
 return false;
}

/**
 * @func    receiveHandleLevelControlCluster
 * @brief   Handler Level led
 * @param   pCmd
 * @retval  None
 */
bool receiveHandleLevelControlCluster(EmberAfClusterCommand* pCmd)
{
	uint8_t byCommandID = pCmd->commandId;
	uint8_t byEndPoint  = pCmd->apsFrame -> destinationEndpoint;
	uint8_t byPayloadOffset = pCmd->payloadStartIndex;		// Gan offset = startindex
	uint8_t byLevel;
	uint16_t wTransitionTime;
	emberAfCorePrintln("ClusterID: 0x%2X", pCmd->apsFrame->clusterId);
/******************************************LEVEL CONTROL LED***************************************************************************/
		switch(byCommandID)
			{

				case ZCL_MOVE_TO_LEVEL_COMMAND_ID:
					if(pCmd->bufLen < byPayloadOffset + 1u)
					{
						return EMBER_ZCL_STATUS_MALFORMED_COMMAND;
					}
					byLevel = emberAfGetInt8u(pCmd->buffer, byPayloadOffset, cmd->bufLen);		// Lay 1 byte gia tri cua level control
					wTransitionTime = emberAfGetInt16u(pCmd->buffer, byPayloadOffset+1, pCmd->bufLen);
					emberAfCorePrintln(" RECEIVE_HandleLevelControlCluster LEVEL = %d, time: 0x%2X\n", byLevel, wTransitionTime);

					if(byEndPoint == 1)
					{
						if(byLevel >=80)
						{
							emberAfCorePrintln("LED GREEN");
							turnOnLed(LED1, ledGreen);
						}else if(byLevel>=40)
						{
							emberAfCorePrintln("LED RED");
							turnOnLed(LED1, ledRed);
						}else if(byLevel>0)
						{
							emberAfCorePrintln("LED BLUE");
							turnOnLed(LED1, ledBlue);
						}else
						{
							emberAfCorePrintln("turn 0ff");
							turnOffRBGLed(LED1);
						}

					}
					break;
				default:
					break;
				}
		return false;
}

/**
 * @func    receiveHandleOnOffCluster
 * @brief   Handler On/Off command
 * @param   EmberAfClusterCommand
 * @retval  bool
 */
bool receiveHandleOnOffCluster(EmberAfClusterCommand* pCmd)
{
	uint8_t byCommandID = pCmd->commandId;
	uint8_t byLocalEndpoint = pCmd ->apsFrame -> destinationEndpoint;
	uint8_t byRemoteEndpoint = pCmd->apsFrame -> sourceEndpoint;
	uint16_t wIgnoreNodeID = pCmd->source;
/************************************ON-OFF LED******************************************************************************************/
	emberAfCorePrintln("receiveHandleOnOffCluster SourceEndpoint = %d, RemoteEndpoint = %d, commandID = %d, nodeID %2X\n",byRemoteEndpoint,byLocalEndpoint,byCommandID,wIgnoreNodeID);
	switch(byCommandID)
	{
	case ZCL_OFF_COMMAND_ID:
		emberAfCorePrintln("Turn off LED");

		switch (pCmd->type) {
			case EMBER_INCOMING_UNICAST:
				if(byLocalEndpoint == 1)
				{
				turnOffRBGLed(LED1);
				SEND_OnOffStateReport(byLocalEndpoint, LED_OFF);
				emberAfCorePrintln("check: %d",checkBindingTable(byLocalEndpoint));
					if(checkBindingTable(byLocalEndpoint) >= 1)
					{

						SEND_BindingInitToTarget(byRemoteEndpoint, byLocalEndpoint, false, wIgnoreNodeID);
					}
				}
				if(byLocalEndpoint == 2)
				{
					turnOffRBGLed(LED2);
					SEND_OnOffStateReport(byLocalEndpoint, LED_OFF);
				}
				break;
			case EMBER_INCOMING_MULTICAST:
				emberAfCorePrintln("Multicast");
				turnOnLed(LED1, ledOff);
				turnOnLed(LED2, ledOff);
				break;
			default:
				break;
		}

		break;
	case ZCL_ON_COMMAND_ID:

		emberAfCorePrintln("Turn on LED");
		switch (pCmd->type) {
			case EMBER_INCOMING_UNICAST:
				if(byLocalEndpoint == 1)
				{
					turnOnLed(LED1, ledBlue);
					SEND_OnOffStateReport(byLocalEndpoint, LED_ON);
					if(checkBindingTable(byLocalEndpoint) >= 1)
					{
						SEND_BindingInitToTarget(byRemoteEndpoint, byLocalEndpoint, true, wIgnoreNodeID);
					}
				}
				if(byLocalEndpoint == 2)
				{
					turnOnLed(LED2, ledBlue);
					SEND_OnOffStateReport(byLocalEndpoint, LED_ON);
				}
				break;
			case EMBER_INCOMING_MULTICAST:
				emberAfCorePrintln("Multicast");
				turnOnLed(LED2, ledGreen);
				break;
			default:
				break;
		}
		break;
	default:
		break;
	}
	return false;
}

/*
 * @function 			: checkBindingTable
 * @brief				: API support to check information on binding table.
 * @parameter			: localEndpoint
 * @return value		: True or false
 */
uint8_t checkBindingTable(uint8_t byLocalEndpoint)
{
	uint8_t index = 0;
	for(uint8_t i=0; i< EMBER_BINDING_TABLE_SIZE; i++)
	{
		EmberBindingTableEntry binding;
		if(emberGetBindingRemoteNodeId(i) != EMBER_SLEEPY_BROADCAST_ADDRESS){
			emberGetBinding(i, &binding);
			if(binding.local == byLocalEndpoint && (binding.type == EMBER_UNICAST_BINDING))
			{
				index++;
			}
		}
	}
	return index;
}
