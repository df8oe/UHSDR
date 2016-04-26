/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name: system_bootloader.c                                                 **
**  Description:                                                                   **
**  Last Modified:                                                                 **
**  Licence:		CC BY-NC-SA 3.0                                            **
************************************************************************************/

#include "stm32f4xx.h"

#define VECT_TAB_OFFSET  0x10000

/* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N */
#define PLL_M      16
#define PLL_N      336

/* SYSCLK = PLL_VCO / PLL_P */
#define PLL_P      2

/* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ */
#define PLL_Q      7

uint32_t SystemCoreClock = 168000000;

__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};


static void SetSysClock(void);
#ifdef DATA_IN_ExtSRAM
static void SystemInit_ExtMemCtl(void);
#endif /* DATA_IN_ExtSRAM */

void SystemInit(void)
{
    /* Reset the RCC clock configuration to the default reset state */
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset CFGR register */
    RCC->CFGR = 0x00000000;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset PLLCFGR register */
    RCC->PLLCFGR = 0x24003010;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

#ifdef DATA_IN_ExtSRAM
    SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM */

    /* Configure the System clock source, PLL Multiplier and Divider factors,
         AHB/APBx prescalers and Flash settings */
    SetSysClock();

    /* Configure the Vector Table location add offset address */
#ifdef VECT_TAB_SRAM
    SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
#else
    SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
#endif
}

void SystemCoreClockUpdate(void)
{
    uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

    /* Get SYSCLK source */
    tmp = RCC->CFGR & RCC_CFGR_SWS;

    switch (tmp)
    {
    case 0x00:  /* HSI used as system clock source */
        SystemCoreClock = HSI_VALUE;
        break;
    case 0x04:  /* HSE used as system clock source */
        SystemCoreClock = HSE_VALUE;
        break;
    case 0x08:  /* PLL used as system clock source */

        pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
        pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;

        if (pllsource != 0)
        {
            /* HSE used as PLL clock source */
            pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }
        else
        {
            /* HSI used as PLL clock source */
            pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> 6);
        }

        pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >>16) + 1 ) *2;
        SystemCoreClock = pllvco/pllp;
        break;
    default:
        SystemCoreClock = HSI_VALUE;
        break;
    }
    /* Compute HCLK frequency */
    /* Get HCLK prescaler */
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    /* HCLK frequency */
    SystemCoreClock >>= tmp;
}

static void SetSysClock(void)
{
    /******************************************************************************/
    /*            PLL (clocked by HSE) used as System clock source                */
    /******************************************************************************/
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    /* Enable HSE */
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);

    /* Wait till HSE is ready and if Time out is reached exit */
    do
    {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;
    }
    while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET)
    {
        HSEStatus = (uint32_t)0x01;
    }
    else
    {
        HSEStatus = (uint32_t)0x00;
    }

    if (HSEStatus == (uint32_t)0x01)
    {
        /* Enable high performance mode, System frequency up to 168 MHz */
        RCC->APB1ENR |= RCC_APB1ENR_PWREN;
        PWR->CR |= PWR_CR_PMODE;

        /* HCLK = SYSCLK / 1*/
        RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

        /* PCLK2 = HCLK / 2*/
        RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

        /* PCLK1 = HCLK / 4*/
        RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

        /* Configure the main PLL */
        RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
                       (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

        /* Enable the main PLL */
        RCC->CR |= RCC_CR_PLLON;

        /* Wait till the main PLL is ready */
        while((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
        }

        /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
        FLASH->ACR = FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

        /* Select the main PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= RCC_CFGR_SW_PLL;

        /* Wait till the main PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
        {
        }
    }
    else
    {
        /* If HSE fails to start-up, the application will have wrong clock
             configuration. User can add here some code to deal with this error */
    }

}

#ifdef DATA_IN_ExtSRAM
void SystemInit_ExtMemCtl(void)
{
    /* Enable GPIOD, GPIOE, GPIOF and GPIOG interface clock */
    RCC->AHB1ENR   = 0x00000078;

    /* Connect PDx pins to FSMC Alternate function */
    GPIOD->AFR[0]  = 0x00cc00cc;
    GPIOD->AFR[1]  = 0xcc0ccccc;
    /* Configure PDx pins in Alternate function mode */
    GPIOD->MODER   = 0xaaaa0a0a;
    /* Configure PDx pins speed to 100 MHz */
    GPIOD->OSPEEDR = 0xffff0f0f;
    /* Configure PDx pins Output type to push-pull */
    GPIOD->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PDx pins */
    GPIOD->PUPDR   = 0x00000000;

    /* Connect PEx pins to FSMC Alternate function */
    GPIOE->AFR[0]  = 0xc00cc0cc;
    GPIOE->AFR[1]  = 0xcccccccc;
    /* Configure PEx pins in Alternate function mode */
    GPIOE->MODER   = 0xaaaa828a;
    /* Configure PEx pins speed to 100 MHz */
    GPIOE->OSPEEDR = 0xffffc3cf;
    /* Configure PEx pins Output type to push-pull */
    GPIOE->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PEx pins */
    GPIOE->PUPDR   = 0x00000000;

    /* Connect PFx pins to FSMC Alternate function */
    GPIOF->AFR[0]  = 0x00cccccc;
    GPIOF->AFR[1]  = 0xcccc0000;
    /* Configure PFx pins in Alternate function mode */
    GPIOF->MODER   = 0xaa000aaa;
    /* Configure PFx pins speed to 100 MHz */
    GPIOF->OSPEEDR = 0xff000fff;
    /* Configure PFx pins Output type to push-pull */
    GPIOF->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PFx pins */
    GPIOF->PUPDR   = 0x00000000;

    /* Connect PGx pins to FSMC Alternate function */
    GPIOG->AFR[0]  = 0x00cccccc;
    GPIOG->AFR[1]  = 0x000000c0;
    /* Configure PGx pins in Alternate function mode */
    GPIOG->MODER   = 0x00080aaa;
    /* Configure PGx pins speed to 100 MHz */
    GPIOG->OSPEEDR = 0x000c0fff;
    /* Configure PGx pins Output type to push-pull */
    GPIOG->OTYPER  = 0x00000000;
    /* No pull-up, pull-down for PGx pins */
    GPIOG->PUPDR   = 0x00000000;

    /* Enable the FSMC interface clock */
    RCC->AHB3ENR         = 0x00000001;

    /* Configure and enable Bank1_SRAM2 */
    FSMC_Bank1->BTCR[2]  = 0x00001015;
    FSMC_Bank1->BTCR[3]  = 0x00010603;//0x00010400;
    FSMC_Bank1E->BWTR[2] = 0x0fffffff;

}
#endif /* DATA_IN_ExtSRAM */