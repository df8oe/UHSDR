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

#ifndef __HW_SI570
#define __HW_SI570

typedef enum
{
    SI570_OK = 0, // tuning ok
    SI570_TUNE_LIMITED, // tuning to freq close to desired freq, still ok
    SI570_TUNE_IMPOSSIBLE, // did not tune, tune freq unknown
    SI570_I2C_ERROR, // could not talk to Si570, tune freq unknown
    SI570_ERROR_VERIFY, // register do not match, tune freq unknown
    SI570_LARGE_STEP, // did not tune, just checking

} Si570_ResultCodes;


// normal operations interface
Si570_ResultCodes Si570_PrepareNextFrequency(ulong freq, int temp_factor);
Si570_ResultCodes Si570_ChangeToNextFrequency();
bool              Si570_IsNextStepLarge();


// startup handling
void 	Si570_Init();

// presence information
bool    Si570_IsPresent();

// startup/runtime reconfiguration
void    Si570_SetPPM(float32_t ppm);


// non-critical device information reading
float32_t   Si570_GetStartupFrequency();
uint8_t     Si570_GetI2CAddress();


// temperature sensor for SoftTcxo
uint8_t   Si570_InitExternalTempSensor();
uint8_t   Si570_ReadExternalTempSensor(int32_t *temp);


#endif
