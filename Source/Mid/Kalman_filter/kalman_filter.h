/*
 * kalman_filter.h
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#ifndef SOURCE_MID_KALMAN_FILTER_KALMAN_FILTER_H_
#define SOURCE_MID_KALMAN_FILTER_KALMAN_FILTER_H_

/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
float _err_measure;
float _err_estimate;
float _q;
float _current_estimate;
float _last_estimate;
float _kalman_gain;
/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/
/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

void KalmanFilterInit(float mea_e, float est_e, float q);
float KalmanFilter_updateEstimate(float mea);
void KalmanFilter_setMeasurementError(float mea_e);
void KalmanFilter_setEstimateError(float est_e);
void KalmanFilter_setProcessNoise(float q);
float KalmanFilter_getKalmanGain(void);
float KalmanFilter_getEstimateError(void);

uint32_t Kalman_sensor(uint32_t sensorValue);

#endif /* SOURCE_MID_KALMAN_FILTER_KALMAN_FILTER_H_ */
