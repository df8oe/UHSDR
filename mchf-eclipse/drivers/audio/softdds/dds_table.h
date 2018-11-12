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
**  Licence:		GNU GPLv3                                                      **
************************************************************************************/

#ifndef __DDS_TABLE_H
#define __DDS_TABLE_H


#define DDS_TBL_BITS        10
#define DDS_TBL_SIZE        (1 << DDS_TBL_BITS) // 10 = 1024

extern const int16_t DDS_TABLE[DDS_TBL_SIZE];

#endif
