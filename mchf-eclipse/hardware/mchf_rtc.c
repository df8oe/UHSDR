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
#include "mchf_board.h"
#include "mchf_rtc.h"
#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_rtc_ex.h"
#include "stm32f4xx_hal_rcc.h"
#include "rtc.h"

/* Private macros */
/* Internal status registers for RTC */
#define RTC_PRESENCE_REG                   RTC_BKP_DR0
#define RTC_PRESENCE_INIT_VAL              0x0001       // if we find this value after power on, we assume battery is available
#define RTC_PRESENCE_OK_VAL                0x0002       // then we set this value to rembember a clock is present
//#define RTC_PRESENCE_ACK_VAL               0x0003       // if we find this value after power on, we assume user enabled RTC
//#define RTC_PRESENCE_NACK_VAL              0x0004       // if we find this value after power on, we assume user decided against using RTC
#ifdef USE_RTC_LSE

static void RTC_LSE_Config() {

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;

    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    __HAL_RCC_RTC_ENABLE();

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_PRESENCE_REG,RTC_PRESENCE_OK_VAL);


}

void MchfRtc_FullReset() {
    __HAL_RCC_BACKUPRESET_FORCE();
}

#if 0
static void RTC_LSI_Config() {
    RCC_LSICmd(ENABLE);

    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WriteProtectionCmd(DISABLE);
    RTC_WaitForSynchro();
    RTC_WriteProtectionCmd(ENABLE);
    RTC_WriteBackupRegister(RTC_STATUS_REG, RTC_STATUS_INIT_OK);
}
#endif

#endif

void MchfRtc_Start()
{


    // ok, there is a battery, so let us now start the oscillator
    RTC_LSE_Config();

    // very first start of rtc
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    HAL_RTC_Init(&hrtc);
}

bool MchfRtc_enabled()
{
    bool retval = false;
#ifdef USE_RTC_LSE

    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_RTC_ENABLE();

    static volatile uint32_t status;
    status = HAL_RTCEx_BKUPRead(&hrtc,RTC_PRESENCE_REG);

    if (status == RTC_PRESENCE_OK_VAL) {
        __HAL_RCC_RTC_ENABLE();
        __HAL_RCC_CLEAR_RESET_FLAGS();
        // HAL_RTCEx_EnableBypassShadow(&hrtc);
        // FIXME: Why do we need to set BYPSHAD ? ABP1 CLK should be high enough....
        retval = true;
        ts.vbat_present = true;
    } else if (status == 0) {
        // if we find the RTC_PRESENCE_INIT_VAL in the backup register next time we boot
        // we know there is a battery present.
        HAL_RTCEx_BKUPWrite(&hrtc,RTC_PRESENCE_REG,RTC_PRESENCE_INIT_VAL);
    } else {
        ts.vbat_present = true;
    }
#endif
    return retval;
}

bool MchfRtc_SetPpm(int16_t ppm)
{
    bool retval = false;
    if (ppm >= RTC_CALIB_PPM_MIN && ppm <= RTC_CALIB_PPM_MAX)
    {
        uint32_t calm;
        uint32_t calp;
        float64_t ppm2pulses = rint((float64_t)ppm * 1.048576); //  = (32 * 32768) / 1000.0000
        if (ppm2pulses <= 0.0) // important, we must make sure to not set calp if 0ppm
        {
            calm = - ppm2pulses;
            calp = 0;
        }
        else
        {
            calm = 512 - ppm2pulses;
            if (calm > 512)
            {
                calm = 0;
            }
            calp = 1;
        }
        HAL_RTCEx_SetSmoothCalib(&hrtc,RTC_SMOOTHCALIB_PERIOD_32SEC,calp,calm);

        retval = true;
    }
    return retval;
}
