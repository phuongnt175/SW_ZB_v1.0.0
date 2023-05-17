/*
 * network.h
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#ifndef SOURCE_APP_NETWORK_NETWORK_H_
#define SOURCE_APP_NETWORK_NETWORK_H_

typedef enum{
	NETWORK_HAS_PARENT,
	NETWORK_JOIN_FAIL,
	NETWORK_JOIN_SUCCESS,
	NETWORK_LOST_PARENT,
	NETWORK_OUT_NETWORK,
}NetworkEvent_e;

typedef void (*networkEventHandler) (NetworkEvent_e networkEvent);

void networkFindAndJoin(void);
void joinNetworkEventHandler(void);
void networkStopFindAndJoin(void);

void networkInit(networkEventHandler networkResult);

#endif /* SOURCE_APP_NETWORK_NETWORK_H_ */
