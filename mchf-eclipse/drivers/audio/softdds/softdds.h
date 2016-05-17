/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                                **
************************************************************************************/

#ifndef __SOFTDDS_H
#define __SOFTDDS_H


void 	softdds_setfreq(float freq, ulong samp_rate, uchar smooth);
void 	softdds_runf(float32_t *i_buff, float32_t *q_buff, ushort size);


void    softdds_setfreq_dbl(float freq[2],ulong samp_rate,uchar smooth);
void    softdds_runf_dbl(float32_t *i_buff, float32_t *q_buff, ushort size);
#endif

