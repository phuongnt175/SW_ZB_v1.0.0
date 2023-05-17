/*
 * send.c
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */
#include PLATFORM_HEADER
#include "stack/include/ember.h"
#include "app/framework/include/af.h"
#include "send.h"
#include "Source/App/Receive/receive.h"
#include "zigbee-framework/zigbee-device-common.h"

/**
 * @func    SEND_SendCommandUnicast
 * @brief   Send uinicast command
 * @param   source, destination, address
 * @retval  None
 */
static void SendCommandUnicast(uint8_t bySource,
								uint8_t byDestination,
								uint8_t byAddress)
{
	emberAfSetCommandEndpoints(bySource, byDestination);
	(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, byAddress);
}

/**
 * @func    SEND_FillBufferGlobalCommand
 * @brief   Send fill buffer global command
 * @param   clusterID, attributeID, globalCommand, value, length, dataType
 * @retval  None
 */
static void sendFillBufferGlobalCommand(EmberAfClusterId wClusterID,
								  EmberAfAttributeId wAttributeID,
								  uint8_t byGlobalCommand,
								  uint8_t* pbyValue,
								  uint8_t byLength,
								  uint8_t byDataType)
{
	uint8_t byData[MAX_DATA_COMMAND_SIZE];
	byData[0] = (uint8_t)(wAttributeID & 0x00FF);
	byData[1] = (uint8_t)((wAttributeID & 0xFF00)>>8);
	byData[2] = EMBER_SUCCESS;
	byData[3] = (uint8_t)byDataType;
	memcpy(&byData[4], pbyValue, byLength);

	(void) emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
									wClusterID,
									byGlobalCommand,
									"b",
									byData,
									byLength + 4);
}

/**
 * @func    SEND_ReportInfoHc
 * @brief   Send Report to HC
 * @param   None
 * @retval  None
 */
void sendReportInfoHc(void)
{
	uint8_t modelID[8] = {7, 'S', 'W', '2','_','L','M','1'};
	uint8_t manufactureID[5] = {4, 'L', 'u', 'm', 'i'};
	uint8_t byVersion = 1;

	if(emberAfNetworkState() != EMBER_JOINED_NETWORK){
		return;
	}
	sendFillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 modelID,
								 8,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
	SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATTION_ENDPOINT,
							HC_NETWORK_ADDRESS);

	sendFillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 manufactureID,
								 5,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
	SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATTION_ENDPOINT,
							HC_NETWORK_ADDRESS);
	sendFillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_APPLICATION_VERSION_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 &byVersion,
								 1,
								 ZCL_INT8U_ATTRIBUTE_TYPE);
	SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATTION_ENDPOINT,
							HC_NETWORK_ADDRESS);
}


/**
 * @func    SEND_OnOffStateReport
 * @brief   Send On/Off State
 * @param   Endpoint, value
 * @retval  None
 */
void sendOnOffStateReport(uint8_t byEndpoint, uint8_t byValue){
	sendFillBufferGlobalCommand(ZCL_ON_OFF_CLUSTER_ID,
						   ZCL_ON_OFF_ATTRIBUTE_ID,
						   ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
						   (uint8_t*) &byValue,
						   1,
						   ZCL_BOOLEAN_ATTRIBUTE_TYPE);

	SendCommandUnicast(byEndpoint,
								DESTINATTION_ENDPOINT,
								HC_NETWORK_ADDRESS);

	emberAfWriteServerAttribute(byEndpoint,
								ZCL_ON_OFF_CLUSTER_ID,
								ZCL_ON_OFF_ATTRIBUTE_ID,
								(uint8_t*) &byValue,
								ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

/**
 * @func    sendLDRStateReport
 * @brief   Send lux value to app
 * @param   source, destination, address
 * @retval  None
 */
void sendLDRStateReport(uint8_t byEndpoint, uint32_t dwValue){
	sendFillBufferGlobalCommand(ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								 ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint32_t*) &dwValue,
								 sizeof(dwValue),
								 ZCL_INT32U_ATTRIBUTE_TYPE);

	SendCommandUnicast(byEndpoint,
						DESTINATTION_ENDPOINT,
						HC_NETWORK_ADDRESS);

	emberAfWriteServerAttribute(byEndpoint,
								ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								(uint32_t*) &dwValue,
								ZCL_INT32U_ATTRIBUTE_TYPE);
}

/**
 * @func    sendZigDevRequest
 * @brief   Send ZigDevRequest
 * @param   None
 * @retval  None
 */
void sendZigDevRequest(void)
{
	uint8_t contents[ZDO_MESSAGE_OVERHEAD + 1];
	contents[1] = 0x00;

	(void) emberSendZigDevRequest(HC_NETWORK_ADDRESS, LEAVE_RESPONSE, EMBER_AF_DEFAULT_APS_OPTIONS, contents, sizeof(contents));
}

/**
 * @func    sendBindingInitToTarget
 * @brief   Send Binding command
 * @param   remoteEndpoint, localEndpoint, value, nodeID
 * @retval  None
 */
void sendBindingInitToTarget(uint8_t byRemoteEndpoint, uint8_t byLocalEndpoint, bool boValue, uint16_t wNodeID)
{
	EmberStatus status = EMBER_INVALID_BINDING_INDEX;

	for(uint8_t i = 0; i< EMBER_BINDING_TABLE_SIZE ; i++)
		{
			EmberBindingTableEntry binding;
			status = emberGetBinding(i, &binding);
			uint16_t bindingNodeID = emberGetBindingRemoteNodeId(i);

			// check status send
			if(status != EMBER_SUCCESS)
			{
				return;
			}else if((binding.local == byLocalEndpoint) && (binding.remote == byRemoteEndpoint) && (bindingNodeID == wNodeID))
			{
				continue;
			}
			else if((bindingNodeID != EMBER_SLEEPY_BROADCAST_ADDRESS) &&
						 (bindingNodeID != EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS) &&
						 (bindingNodeID != EMBER_BROADCAST_ADDRESS))
			{
				if(binding.local == byLocalEndpoint && binding.clusterId == ZCL_ON_OFF_CLUSTER_ID){
					switch (boValue) {
						case true:
							emberAfCorePrintln("SEND ON INIT TO TARGET");
							emberAfFillCommandOnOffClusterOn();
							emberAfSetCommandEndpoints(binding.local, binding.remote);
							emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);
							sendOnOffStateReport(binding.local, LED_ON);
							break;
						case false:
							emberAfCorePrintln("SEND OFF INIT TO TARGET");
							emberAfFillCommandOnOffClusterOff();
							emberAfSetCommandEndpoints(binding.local, binding.remote);
							emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);
							sendOnOffStateReport(binding.local, LED_OFF);
							break;
					}
				}
			}
		}
}
