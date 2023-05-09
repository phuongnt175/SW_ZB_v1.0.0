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
static void SEND_SendCommandUnicast(uint8_t source,
							 uint8_t destination,
							 uint8_t address)
{
	emberAfSetCommandEndpoints(source, destination);
	(void) emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, address);
}


/**
 * @func    SEND_FillBufferGlobalCommand
 * @brief   Send fill buffer global command
 * @param   clusterID, attributeID, globalCommand, value, length, dataType
 * @retval  None
 */
static void SEND_FillBufferGlobalCommand(EmberAfClusterId clusterID,
								  EmberAfAttributeId attributeID,
								  uint8_t globalCommand,
								  uint8_t* value,
								  uint8_t length,
								  uint8_t dataType)
{
	uint8_t data[MAX_DATA_COMMAND_SIZE];
	data[0] = (uint8_t)(attributeID & 0x00FF);
	data[1] = (uint8_t)((attributeID & 0xFF00)>>8);
	data[2] = EMBER_SUCCESS;
	data[3] = (uint8_t)dataType;
	memcpy(&data[4], value, length);

	(void) emberAfFillExternalBuffer((ZCL_GLOBAL_COMMAND | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER | ZCL_DISABLE_DEFAULT_RESPONSE_MASK),
									  clusterID,
									  globalCommand,
									  "b",
									  data,
									  length + 4);
}


/**
 * @func    SEND_ReportInfoHc
 * @brief   Send Report to HC
 * @param   None
 * @retval  None
 */
void SEND_ReportInfoHc(void)
{
	uint8_t modelID[8] = {7, 'S', 'W', '2','_','L','M','1'};
	uint8_t manufactureID[5] = {4, 'L', 'u', 'm', 'i'};
	uint8_t version = 1;

	if(emberAfNetworkState() != EMBER_JOINED_NETWORK){
		return;
	}
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 modelID,
								 8,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
	SEND_SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATTION_ENDPOINT,
							HC_NETWORK_ADDRESS);

	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 manufactureID,
								 5,
								 ZCL_CHAR_STRING_ATTRIBUTE_TYPE);
	SEND_SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATTION_ENDPOINT,
							HC_NETWORK_ADDRESS);
	SEND_FillBufferGlobalCommand(ZCL_BASIC_CLUSTER_ID,
								 ZCL_APPLICATION_VERSION_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 &version,
								 1,
								 ZCL_INT8U_ATTRIBUTE_TYPE);
	SEND_SendCommandUnicast(SOURCE_ENDPOINT_PRIMARY,
							DESTINATTION_ENDPOINT,
							HC_NETWORK_ADDRESS);
}


/**
 * @func    SEND_OnOffStateReport
 * @brief   Send On/Off State
 * @param   Endpoint, value
 * @retval  None
 */
void SEND_OnOffStateReport(uint8_t Endpoint, uint8_t value){
	SEND_FillBufferGlobalCommand(ZCL_ON_OFF_CLUSTER_ID,
						   ZCL_ON_OFF_ATTRIBUTE_ID,
						   ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
						   (uint8_t*) &value,
						   1,
						   ZCL_BOOLEAN_ATTRIBUTE_TYPE);

	SEND_SendCommandUnicast(Endpoint,
								DESTINATTION_ENDPOINT,
								HC_NETWORK_ADDRESS);

	emberAfWriteServerAttribute(Endpoint,
								ZCL_ON_OFF_CLUSTER_ID,
								ZCL_ON_OFF_ATTRIBUTE_ID,
								(uint8_t*) &value,
								ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

/**
 * @func    SEND_LDRStateReport
 * @brief   Send lux value to app
 * @param   source, destination, address
 * @retval  None
 */
void SEND_LDRStateReport(uint8_t Endpoint, uint32_t value){
	SEND_FillBufferGlobalCommand(ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								 ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								 ZCL_READ_ATTRIBUTES_RESPONSE_COMMAND_ID,
								 (uint32_t*) &value,
								 sizeof(value),
								 ZCL_INT32U_ATTRIBUTE_TYPE);

	SEND_SendCommandUnicast(Endpoint,
								DESTINATTION_ENDPOINT,
								HC_NETWORK_ADDRESS);

	emberAfWriteServerAttribute(Endpoint,
								ZCL_ILLUM_MEASUREMENT_CLUSTER_ID,
								ZCL_ILLUM_MEASURED_VALUE_ATTRIBUTE_ID,
								(uint32_t*) &value,
								ZCL_INT32U_ATTRIBUTE_TYPE);
}


/**
 * @func    SendZigDevRequest
 * @brief   Send ZigDevRequest
 * @param   None
 * @retval  None
 */
void SendZigDevRequest(void)
{
	uint8_t contents[ZDO_MESSAGE_OVERHEAD + 1];
	contents[1] = 0x00;

	(void) emberSendZigDevRequest(HC_NETWORK_ADDRESS, LEAVE_RESPONSE, EMBER_AF_DEFAULT_APS_OPTIONS, contents, sizeof(contents));
}

/**
 * @func    SEND_BindingInitToTarget
 * @brief   Send Binding command
 * @param   remoteEndpoint, localEndpoint, value, nodeID
 * @retval  None
 */
void SEND_BindingInitToTarget(uint8_t remoteEndpoint, uint8_t localEndpoint, bool value, uint16_t nodeID)
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
			}else if((binding.local == localEndpoint) && (binding.remote == remoteEndpoint) && (bindingNodeID == nodeID))
			{
				continue;
			}
			else if((bindingNodeID != EMBER_SLEEPY_BROADCAST_ADDRESS) &&
						 (bindingNodeID != EMBER_RX_ON_WHEN_IDLE_BROADCAST_ADDRESS) &&
						 (bindingNodeID != EMBER_BROADCAST_ADDRESS))
			{
				if(binding.local == localEndpoint && binding.clusterId == ZCL_ON_OFF_CLUSTER_ID){
					switch (value) {
						case true:
							emberAfCorePrintln("SEND ON INIT TO TARGET");
							emberAfFillCommandOnOffClusterOn();
							emberAfSetCommandEndpoints(binding.local, binding.remote);
							emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);
							SEND_OnOffStateReport(binding.local, LED_ON);
							break;
						case false:
							emberAfCorePrintln("SEND OFF INIT TO TARGET");
							emberAfFillCommandOnOffClusterOff();
							emberAfSetCommandEndpoints(binding.local, binding.remote);
							emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT, bindingNodeID);
							SEND_OnOffStateReport(binding.local, LED_OFF);
							break;
					}
				}
			}
		}
}
