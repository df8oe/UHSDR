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

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;

    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;

    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    __HAL_RCC_RTC_ENABLE();

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_PRESENCE_REG,RTC_PRESENCE_OK_VAL);

}
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

#endif
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

/**
  * @brief  Gets RTC current time. THIS A replacement for the not working HAL_RTC_GetTime() function/
  * @param  hrtc: pointer to a RTC_HandleTypeDef structure that contains
  *                the configuration information for RTC.
  * @param  sTime: Pointer to Time structure
  * @param  Format: Specifies the format of the entered parameters.
  *          This parameter can be one of the following values:
  *            @arg RTC_FORMAT_BIN: Binary data format
  *            @arg RTC_FORMAT_BCD: BCD data format
  * @note  You cannot use SubSeconds etc since these are not read due to an apparent bug in some STM32F407 silicon.
  *        On my STM32F407 reading the SSR will for sure stop the RTC shadow registers and cause also other troubles.
  *        Probably different hardware does not have this problem but we don't need subseconds anyway.
  * @note You must call HAL_RTC_GetDate() after HAL_RTC_GetTime() to unlock the values
  *        in the higher-order calendar shadow registers to ensure consistency between the time and date values.
  *        Reading RTC current time locks the values in calendar shadow registers until current date is read.
  * @retval HAL status
  */

uint32_t dr_dummy;

void MchfRtc_GetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, uint32_t Format)
{
  uint32_t tmpreg = 0U;

  /* Check the parameters */
  assert_param(IS_RTC_FORMAT(Format));

  /* Get subseconds structure field from the corresponding register */
  // See above:
  sTime->SubSeconds = (uint32_t)(hrtc->Instance->SSR);
  //sTime->SubSeconds = 0;

  /* Get SecondFraction structure field from the corresponding register field*/
  sTime->SecondFraction = (uint32_t)(hrtc->Instance->PRER & RTC_PRER_PREDIV_S);
  // sTime->SecondFraction = 0;

  /* Get the TR register */
  tmpreg = (uint32_t)(hrtc->Instance->TR & RTC_TR_RESERVED_MASK);

  dr_dummy = (uint32_t)hrtc->Instance->DR;

  /* Fill the structure fields with the read parameters */
  sTime->Hours = (uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16U);
  sTime->Minutes = (uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8U);
  sTime->Seconds = (uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU));
  sTime->TimeFormat = (uint8_t)((tmpreg & (RTC_TR_PM)) >> 16U);

  /* Check the input parameters format */
  if(Format == RTC_FORMAT_BIN)
  {
    /* Convert the time structure parameters to Binary format */
    sTime->Hours = (uint8_t)RTC_Bcd2ToByte(sTime->Hours);
    sTime->Minutes = (uint8_t)RTC_Bcd2ToByte(sTime->Minutes);
    sTime->Seconds = (uint8_t)RTC_Bcd2ToByte(sTime->Seconds);
  }
}
