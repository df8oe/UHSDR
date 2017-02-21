/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

#ifndef HARDWARE_MCHF_RTC_H_
#define HARDWARE_MCHF_RTC_H_

#define RTC_CALIB_PPM_MAX 488
#define RTC_CALIB_PPM_MIN -487
#define RTC_CALIB_PPM_DEFAULT 0



bool MchfRtc_enabled();
void MchfRtc_FullReset();
void MchfRtc_Start();

#endif /* HARDWARE_MCHF_RTC_H_ */
