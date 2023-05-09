/*
 * kalman_filter.c
 *
 *  Created on: Apr 16, 2023
 *      Author: admin1
 */

#include "app/framework/include/af.h"
#include "kalman_filter.h"
#include <math.h>

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
/*
 */
void KalmanFilterInit(float mea_e, float est_e, float q)
{
  _err_measure=mea_e;
  _err_estimate=est_e;
  _q = q;
}

float KalmanFilter_updateEstimate(float mea)
{
  _kalman_gain = _err_estimate/(_err_estimate + _err_measure);
  _current_estimate = _last_estimate + _kalman_gain * (mea - _last_estimate);
  _err_estimate =  (1.0 - _kalman_gain)*_err_estimate + fabs(_last_estimate-_current_estimate)*_q;
  _last_estimate=_current_estimate;

  return _current_estimate;
}

void KalmanFilter_setMeasurementError(float mea_e)
{
  _err_measure=mea_e;
}

void KalmanFilter_setEstimateError(float est_e)
{
  _err_estimate=est_e;
}

void KalmanFilter_setProcessNoise(float q)
{
  _q=q;
}

float KalmanFilter_getKalmanGain(void) {
  return _kalman_gain;
}

float KalmanFilter_getEstimateError(void) {
  return _err_estimate;
}

/**
 * @func   Kalman_sensor
 * @brief  Use Kanman filter for Light Sensor
 * @param  sensorValue
 * @retval uint32_t
 */
uint32_t Kalman_sensor(uint32_t sensorValue)
{
	uint32_t sensorValueAfterFilter = KalmanFilter_updateEstimate(sensorValue);

	return sensorValueAfterFilter;
}

/* END FILE */
