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
#include "mchf_board.h"
#include "mchf_rtc.h"

#ifdef USE_RTC_LSE
/* Private macros */
/* Internal status registers for RTC */
#define RTC_PRESENCE_REG                   RTC_BKP_DR0
#define RTC_PRESENCE_INIT_VAL              0x0001       // if we find this value after power on, we assume battery is available
#define RTC_PRESENCE_OK_VAL                0x0002       // then we set this value to rembember a clock is present

static void RTC_LSE_Config() {
    RCC_LSEConfig(RCC_LSE_ON);

    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    RCC_RTCCLKCmd(ENABLE);
    RTC_WriteProtectionCmd(DISABLE);
    RTC_WaitForSynchro();
    RTC_WriteProtectionCmd(ENABLE);
    RTC_WriteBackupRegister(RTC_PRESENCE_REG, RTC_PRESENCE_INIT_VAL);
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
bool MchfRtc_enabled()
{
    bool retval = false;
#ifdef USE_RTC_LSE

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    PWR_BackupAccessCmd(ENABLE);

    uint32_t status = RTC_ReadBackupRegister(RTC_PRESENCE_REG);

    if (status == RTC_PRESENCE_INIT_VAL || status == RTC_PRESENCE_OK_VAL) {
        if (status == RTC_PRESENCE_INIT_VAL)
        {
            RTC_WriteProtectionCmd(DISABLE);
            RTC_WaitForSynchro();
            RTC_WriteProtectionCmd(ENABLE);
            RTC_WriteBackupRegister(RTC_PRESENCE_REG,RTC_PRESENCE_OK_VAL);
        }
        RTC_ClearITPendingBit(RTC_IT_WUT);
        EXTI->PR = 0x00400000;
        retval = true;
        // get date/time
    } else {
        RTC_LSE_Config();

        // very first start of rtc
        RTC_InitTypeDef rtc_init;

        rtc_init.RTC_HourFormat = RTC_HourFormat_24;
        // internal clock, 32kHz, see www.st.com/resource/en/application_note/dm00025071.pdf
        // rtc_init.RTC_AsynchPrediv = 123;   // 117; // 111; // 47; // 54; // 128 -1 = 127;
        // rtc_init.RTC_SynchPrediv =  234;   // 246; // 606; // 547; // 250 -1 = 249;
        // internal clock, 32kHz, see www.st.com/resource/en/application_note/dm00025071.pdf
         rtc_init.RTC_AsynchPrediv = 127; // 32768 = 128 * 256
         rtc_init.RTC_SynchPrediv =  255;

        RTC_Init(&rtc_init);

        retval = false;
    }
#endif
    return retval;
}


