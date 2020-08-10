/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef HARDWARE_MCHF_RTC_H_
#define HARDWARE_MCHF_RTC_H_

#define RTC_CALIB_PPM_MAX 488
#define RTC_CALIB_PPM_MIN -487
#define RTC_CALIB_PPM_DEFAULT 0

#include "rtc.h"

bool Rtc_isEnabled(void);
void Rtc_FullReset(void);
void Rtc_Start(void);
bool Rtc_SetPpm(int16_t ppm);
void Rtc_GetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format);

#endif /* HARDWARE_MCHF_RTC_H_ */
