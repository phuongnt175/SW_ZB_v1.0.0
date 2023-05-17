/*
 * send.h
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#ifndef SOURCE_APP_SEND_SEND_H_
#define SOURCE_APP_SEND_SEND_H_

#define MAX_DATA_COMMAND_SIZE					50
#define SOURCE_ENDPOINT_PRIMARY					1
#define DESTINATTION_ENDPOINT					1
#define HC_NETWORK_ADDRESS						0x0000
 #define ZDO_MESSAGE_OVERHEAD 					1

void sendZigDevRequest(void);
void sendOnOffStateReport(uint8_t byEndpoint, uint8_t byValue);
void sendReportInfoHc(void);
void SendZigDevRequest(void);

void sendLDRStateReport(uint8_t byEndpoint, uint32_t dwValue);
void sendBindingInitToTarget(uint8_t byRemoteEndpoint, uint8_t byLocalEndpoint, bool boValue, uint16_t wNodeID);

#endif /* SOURCE_APP_SEND_SEND_H_ */
