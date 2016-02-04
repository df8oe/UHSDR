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
**  Licence:		For radio amateurs experimentation, non-commercial use only!   **
************************************************************************************/

// Common
#include "mchf_board.h"

#include <stdio.h>

// serial EEPROM driver
#include "mchf_hw_i2c2.h"

// Audio Driver
#include "audio_driver.h"
#include "cw_gen.h"

// UI Driver
#include "ui_driver.h"
#include "ui_rotary.h"
#include "ui_lcd_hy28.h"
#include "ui_menu.h"

// Keyboard Driver
// #include "keyb_driver.h"

// Misc
#include "softdds.h"

// Eeprom
#include "eeprom.h"
//
//
//
#include "cat_driver.h"
// Transceiver state public structure
__IO TransceiverState ts;

// ----------------------------------------------------
// Create a time reference incremented by 1 mS and 10mS
//__IO uint32_t LocalTime_1MS  = 0;
//__IO uint32_t LocalTime_10MS = 0;
//__IO uint32_t LocalTime_Over = 0;
// ----------------------------------------------------

// USB Host
//extern USB_OTG_CORE_HANDLE          USB_OTG_Core_dev;

// TIM5 publics
//extern __IO uint32_t PeriodValue;
//extern __IO uint32_t CaptureNumber;
//uint16_t tmpCC4[2] = {0, 0};
//
// If more EEPROM variables are added, make sure that you add to this table - and the index to it in "eeprom.h"
// and correct MAX_VAR_ADDR in mchf_board.h


const uint16_t VirtAddVarTab[NB_OF_VAR] =
{
		VAR_ADDR_1,
		VAR_ADDR_2,
		VAR_ADDR_3,
		VAR_ADDR_4,
		VAR_ADDR_5,
		VAR_ADDR_6,
		VAR_ADDR_7,
		VAR_ADDR_8,
		VAR_ADDR_9,
		VAR_ADDR_10,
		VAR_ADDR_11,
		VAR_ADDR_12,
		VAR_ADDR_13,
		VAR_ADDR_14,
		VAR_ADDR_15,
		VAR_ADDR_16,
		VAR_ADDR_17,
		VAR_ADDR_18,
		VAR_ADDR_19,
		VAR_ADDR_20,
		VAR_ADDR_21,
		VAR_ADDR_22,
		VAR_ADDR_23,
		VAR_ADDR_24,
		VAR_ADDR_25,
		VAR_ADDR_26,
		VAR_ADDR_27,
		VAR_ADDR_28,
		VAR_ADDR_29,
		VAR_ADDR_30,
		VAR_ADDR_31,
		VAR_ADDR_32,
		VAR_ADDR_33,
		VAR_ADDR_34,
		VAR_ADDR_35,
		VAR_ADDR_36,
		VAR_ADDR_37,
		VAR_ADDR_38,
		VAR_ADDR_39,
		VAR_ADDR_40,
		VAR_ADDR_41,
		VAR_ADDR_42,
		VAR_ADDR_43,
		VAR_ADDR_44,
		VAR_ADDR_45,
		VAR_ADDR_46,
		VAR_ADDR_47,
		VAR_ADDR_48,
		VAR_ADDR_49,
		VAR_ADDR_50,
		VAR_ADDR_51,
		VAR_ADDR_52,
		VAR_ADDR_53,
		VAR_ADDR_54,
		VAR_ADDR_55,
		VAR_ADDR_56,
		VAR_ADDR_57,
		VAR_ADDR_58,
		VAR_ADDR_59,
		VAR_ADDR_60,
		VAR_ADDR_61,
		VAR_ADDR_62,
		VAR_ADDR_63,
		VAR_ADDR_64,
		VAR_ADDR_65,
		VAR_ADDR_66,
		VAR_ADDR_67,
		VAR_ADDR_68,
		VAR_ADDR_69,
		VAR_ADDR_70,
		VAR_ADDR_71,
		VAR_ADDR_72,
		VAR_ADDR_73,
		VAR_ADDR_74,
		VAR_ADDR_75,
		VAR_ADDR_76,
		VAR_ADDR_77,
		VAR_ADDR_78,
		VAR_ADDR_79,
		VAR_ADDR_80,
		VAR_ADDR_81,
		VAR_ADDR_82,
		VAR_ADDR_83,
		VAR_ADDR_84,
		VAR_ADDR_85,
		VAR_ADDR_86,
		VAR_ADDR_87,
		VAR_ADDR_88,
		VAR_ADDR_89,
		VAR_ADDR_90,
		VAR_ADDR_91,
		VAR_ADDR_92,
		VAR_ADDR_93,
		VAR_ADDR_94,
		VAR_ADDR_95,
		VAR_ADDR_96,
		VAR_ADDR_97,
		VAR_ADDR_98,
		VAR_ADDR_99,
		VAR_ADDR_100,
		VAR_ADDR_101,
		VAR_ADDR_102,
		VAR_ADDR_103,
		VAR_ADDR_104,
		VAR_ADDR_105,
		VAR_ADDR_106,
		VAR_ADDR_107,
		VAR_ADDR_108,
		VAR_ADDR_109,
		VAR_ADDR_110,
		VAR_ADDR_111,
		VAR_ADDR_112,
		VAR_ADDR_113,
		VAR_ADDR_114,
		VAR_ADDR_115,
		VAR_ADDR_116,
		VAR_ADDR_117,
		VAR_ADDR_118,
		VAR_ADDR_119,
		VAR_ADDR_120,
		VAR_ADDR_121,
		VAR_ADDR_122,
		VAR_ADDR_123,
		VAR_ADDR_124,
		VAR_ADDR_125,
		VAR_ADDR_126,
		VAR_ADDR_127,
		VAR_ADDR_128,
		VAR_ADDR_129,
		VAR_ADDR_130,
		VAR_ADDR_131,
		VAR_ADDR_132,
		VAR_ADDR_133,
		VAR_ADDR_134,
		VAR_ADDR_135,
		VAR_ADDR_136,
		VAR_ADDR_137,
		VAR_ADDR_138,
		VAR_ADDR_139,
		VAR_ADDR_140,
		VAR_ADDR_141,
		VAR_ADDR_142,
		VAR_ADDR_143,
		VAR_ADDR_144,
		VAR_ADDR_145,
		VAR_ADDR_146,
		VAR_ADDR_147,
		VAR_ADDR_148,
		VAR_ADDR_149,
		VAR_ADDR_150,
		VAR_ADDR_151,
		VAR_ADDR_152,
		VAR_ADDR_153,
		VAR_ADDR_154,
		VAR_ADDR_155,
		VAR_ADDR_156,
		VAR_ADDR_157,
		VAR_ADDR_158,
		VAR_ADDR_159,
		VAR_ADDR_160,
		VAR_ADDR_161,
		VAR_ADDR_162,
		VAR_ADDR_163,
		VAR_ADDR_164,
		VAR_ADDR_165,
		VAR_ADDR_166,
		VAR_ADDR_167,
		VAR_ADDR_168,
		VAR_ADDR_169,
		VAR_ADDR_170,
		VAR_ADDR_171,
		VAR_ADDR_172,
		VAR_ADDR_173,
		VAR_ADDR_174,
		VAR_ADDR_175,
		VAR_ADDR_176,
		VAR_ADDR_177,
		VAR_ADDR_178,
		VAR_ADDR_179,
		VAR_ADDR_180,
		VAR_ADDR_181,
		VAR_ADDR_182,
		VAR_ADDR_183,
		VAR_ADDR_184,
		VAR_ADDR_185,
		VAR_ADDR_186,
		VAR_ADDR_187,
		VAR_ADDR_188,
		VAR_ADDR_189,
		VAR_ADDR_190,
		VAR_ADDR_191,
		VAR_ADDR_192,
		VAR_ADDR_193,
		VAR_ADDR_194,
		VAR_ADDR_195,
		VAR_ADDR_196,
		VAR_ADDR_197,
		VAR_ADDR_198,
		VAR_ADDR_199,
		VAR_ADDR_200,
		VAR_ADDR_201,
		VAR_ADDR_202,
		VAR_ADDR_203,
		VAR_ADDR_204,
		VAR_ADDR_205,
		VAR_ADDR_206,
		VAR_ADDR_207,
		VAR_ADDR_208,
		VAR_ADDR_209,
		VAR_ADDR_210,
		VAR_ADDR_211,
		VAR_ADDR_212,
		VAR_ADDR_213,
		VAR_ADDR_214,
		VAR_ADDR_215,
		VAR_ADDR_216,
		VAR_ADDR_217,
		VAR_ADDR_218,
		VAR_ADDR_219,
		VAR_ADDR_220,
		VAR_ADDR_221,
		VAR_ADDR_222,
		VAR_ADDR_223,
		VAR_ADDR_224,
		VAR_ADDR_225,
		VAR_ADDR_226,
		VAR_ADDR_227,
		VAR_ADDR_228,
		VAR_ADDR_229,
		VAR_ADDR_230,
		VAR_ADDR_231,
		VAR_ADDR_232,
		VAR_ADDR_233,
		VAR_ADDR_234,
		VAR_ADDR_235,
		VAR_ADDR_236,
		VAR_ADDR_237,
		VAR_ADDR_238,
		VAR_ADDR_239,
		VAR_ADDR_240,
		VAR_ADDR_241,
		VAR_ADDR_242,
		VAR_ADDR_243,
		VAR_ADDR_244,
		VAR_ADDR_245,
		VAR_ADDR_246,
		VAR_ADDR_247,
		VAR_ADDR_248,
		VAR_ADDR_249,
		VAR_ADDR_250,
		VAR_ADDR_251,
		VAR_ADDR_252,
		VAR_ADDR_253,
		VAR_ADDR_254,
		VAR_ADDR_255,
		VAR_ADDR_256,
		VAR_ADDR_257,
		VAR_ADDR_258,
		VAR_ADDR_259,
		VAR_ADDR_260,
		VAR_ADDR_261,
		VAR_ADDR_262,
		VAR_ADDR_263,
		VAR_ADDR_264,
		VAR_ADDR_265,
		VAR_ADDR_266,
		VAR_ADDR_267,
		VAR_ADDR_268,
		VAR_ADDR_269,
		VAR_ADDR_270,
		VAR_ADDR_271,
		VAR_ADDR_272,
		VAR_ADDR_273,
		VAR_ADDR_274,
		VAR_ADDR_275,
		VAR_ADDR_276,
		VAR_ADDR_277,
		VAR_ADDR_278,
		VAR_ADDR_279,
		VAR_ADDR_280,
		VAR_ADDR_281,
		VAR_ADDR_282,
		VAR_ADDR_283,
		VAR_ADDR_284,
		VAR_ADDR_285,
		VAR_ADDR_286,
		VAR_ADDR_287,
		VAR_ADDR_288,
		VAR_ADDR_289,
		VAR_ADDR_290,
		VAR_ADDR_291,
		VAR_ADDR_292,
		VAR_ADDR_293,
		VAR_ADDR_294,
		VAR_ADDR_295,
		VAR_ADDR_296,
		VAR_ADDR_297,
		VAR_ADDR_298,
		VAR_ADDR_299,
		VAR_ADDR_300,
		VAR_ADDR_301,
		VAR_ADDR_302,
		VAR_ADDR_303,
		VAR_ADDR_304,
		VAR_ADDR_305,
		VAR_ADDR_306,
		VAR_ADDR_307,
		VAR_ADDR_308,
		VAR_ADDR_309,
		VAR_ADDR_310,
		VAR_ADDR_311,
		VAR_ADDR_312,
		VAR_ADDR_313,
		VAR_ADDR_314,
		VAR_ADDR_315,
		VAR_ADDR_316,
		VAR_ADDR_317,
		VAR_ADDR_318,
		VAR_ADDR_319,
		VAR_ADDR_320,
		VAR_ADDR_321,
		VAR_ADDR_322,
		VAR_ADDR_323,
		VAR_ADDR_324,
		VAR_ADDR_325,
		VAR_ADDR_326,
		VAR_ADDR_327,
		VAR_ADDR_328,
		VAR_ADDR_329,
		VAR_ADDR_330,
		VAR_ADDR_331,
		VAR_ADDR_332,
		VAR_ADDR_333,
		VAR_ADDR_334,
		VAR_ADDR_335,
		VAR_ADDR_336,
		VAR_ADDR_337,
		VAR_ADDR_338,
		VAR_ADDR_339,
		VAR_ADDR_340,
		VAR_ADDR_341,
		VAR_ADDR_342,
		VAR_ADDR_343,
		VAR_ADDR_344,
		VAR_ADDR_345,
		VAR_ADDR_346,
		VAR_ADDR_347,
		VAR_ADDR_348,
		VAR_ADDR_349,
		VAR_ADDR_350,
		VAR_ADDR_351,
		VAR_ADDR_352,
		VAR_ADDR_353,
		VAR_ADDR_354,
		VAR_ADDR_355,
		VAR_ADDR_356,
		VAR_ADDR_357,
		VAR_ADDR_358,
		VAR_ADDR_359,
		VAR_ADDR_360,
		VAR_ADDR_361,
		VAR_ADDR_362,
		VAR_ADDR_363,
		VAR_ADDR_364,
		VAR_ADDR_365,
		VAR_ADDR_366,
		VAR_ADDR_367,
		VAR_ADDR_368,
		VAR_ADDR_369,
		VAR_ADDR_370,
		VAR_ADDR_371,
		VAR_ADDR_372,
		VAR_ADDR_373,
		VAR_ADDR_374,
		VAR_ADDR_375,
		VAR_ADDR_376,
		VAR_ADDR_377,
		VAR_ADDR_378,
		VAR_ADDR_379,
		VAR_ADDR_380,
		VAR_ADDR_381,
		VAR_ADDR_382,
		VAR_ADDR_383
};

// System tick if needed
__IO uint32_t TimingDelay = 0;

uchar wd_init_enabled = 0;

//*----------------------------------------------------------------------------
//* Function Name       : CriticalError
//* Object              : should never be here, really
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void CriticalError(ulong error)
{
	NVIC_SystemReset();
}

//*----------------------------------------------------------------------------
//* Function Name       : NMI_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void NMI_Handler(void)
{
	printf("NMI_Handler called\n\r");
	CriticalError(1);
}

//*----------------------------------------------------------------------------
//* Function Name       : HardFault_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void HardFault_Handler(void)
{
	printf("HardFault_Handler called\n\r");
	CriticalError(2);
}

//*----------------------------------------------------------------------------
//* Function Name       : MemManage_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void MemManage_Handler(void)
{
	printf("MemManage_Handler called\n\r");
	CriticalError(3);
}

//*----------------------------------------------------------------------------
//* Function Name       : BusFault_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void BusFault_Handler(void)
{
	printf("BusFault_Handler called\n\r");
	CriticalError(4);
}

//*----------------------------------------------------------------------------
//* Function Name       : UsageFault_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UsageFault_Handler(void)
{
	printf("UsageFault_Handler called\n\r");
	CriticalError(5);
}

//*----------------------------------------------------------------------------
//* Function Name       : SVC_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void SVC_Handler(void)
{
	printf("SVC_Handler called\n\r");
	CriticalError(6);
}

//*----------------------------------------------------------------------------
//* Function Name       : DebugMon_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void DebugMon_Handler(void)
{
	printf("DebugMon_Handler called\n\r");
	CriticalError(7);
}

//*----------------------------------------------------------------------------
//* Function Name       : SysTick_Handler
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void SysTick_Handler(void)
{
//!	TimingDelay++;

	// Process UI refresh
//	ui_driver_irq();
}

//*----------------------------------------------------------------------------
//* Function Name       : EXTI0_IRQHandler
//* Object              : paddles dah
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void EXTI0_IRQHandler(void)
{
	// Checks whether the User Button EXTI line is asserted
	//
	// WARNING:
	// Due to an apparent HARDWARE bug in the MCU this interrupt seems to be occasionally triggered by transitions
	// of lines OTHER than the PADDLE_DAH (PE0) line, specifically the PC4 and PC5 (Step- and Step+) lines.
	//
	if (EXTI_GetITStatus(EXTI_Line0) != RESET)
	{
		// Call handler
		if(ts.dmod_mode == DEMOD_CW)	{
			if(!GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH))	{	// was DAH line low?
				cw_gen_dah_IRQ();		// Yes - go to CW state machine
			}
		}
		//
		// PTT activate
		else if((ts.dmod_mode == DEMOD_USB)||(ts.dmod_mode == DEMOD_LSB) || (ts.dmod_mode == DEMOD_AM) || (ts.dmod_mode == DEMOD_FM))
		{
			if(!GPIO_ReadInputDataBit(PADDLE_DAH_PIO,PADDLE_DAH))	{	// was PTT line low?
				ts.ptt_req = 1;		// yes - ONLY then do we activate PTT!  (e.g. prevent hardware bug from keying PTT!)
			}
		}
	}

	// Clears the EXTI's line pending bit
	EXTI_ClearITPendingBit(EXTI_Line0);
}

//*----------------------------------------------------------------------------
//* Function Name       : EXTI1_IRQHandler
//* Object              : paddles dit
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void EXTI1_IRQHandler(void)
{
	// Checks whether the User Button EXTI line (paddle DIT) is asserted
	if (EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
		// Call handler
		if(ts.dmod_mode == DEMOD_CW)
			if(!GPIO_ReadInputDataBit(PADDLE_DIT_PIO,PADDLE_DIT))	// was Dit line low?  (Validate to prevent extraneous interrupts)
				cw_gen_dit_IRQ();
	}	// do nothing if not in CW mode!

	// Clears the EXTI's line pending bit
	EXTI_ClearITPendingBit(EXTI_Line1);
}

//*----------------------------------------------------------------------------
//* Function Name       : EXTI15_10_IRQHandler
//* Object              : power button irq here
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void EXTI15_10_IRQHandler(void)
{
	// power button interrupt
	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
	{
//		if(!GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13))	// Signal power off

	}
	// Clear interrupt pending bit
	EXTI_ClearITPendingBit(EXTI_Line13);
}

/*void TIM5_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET)
  {
    // Get the Input Capture value
    tmpCC4[CaptureNumber++] = TIM_GetCapture4(TIM5);

    // Clear CC4 Interrupt pending bit
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);

    if (CaptureNumber >= 2)
    {
      // Compute the period length
      PeriodValue = (uint16_t)(0xFFFF - tmpCC4[0] + tmpCC4[1] + 1);
    }
  }
}*/

//*----------------------------------------------------------------------------
//* Function Name       : TransceiverStateInit
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void TransceiverStateInit(void)
{
	// Defaults always
	ts.txrx_mode 		= TRX_MODE_RX;				// start in RX
	ts.samp_rate		= I2S_AudioFreq_48k;		// set sampling rate

	ts.enc_one_mode 	= ENC_ONE_MODE_AUDIO_GAIN;
	ts.enc_two_mode 	= ENC_TWO_MODE_RF_GAIN;
	ts.enc_thr_mode		= ENC_THREE_MODE_RIT;

	ts.band		  		= BAND_MODE_20;				// band from eeprom
	ts.band_change		= 0;						// used in muting audio during band change
	ts.filter_band		= 0;						// used to indicate the bpf filter selection for power detector coefficient selection
	ts.dmod_mode 		= DEMOD_USB;				// demodulator mode
	ts.audio_gain		= DEFAULT_AUDIO_GAIN;		// Set initial volume
	ts.audio_gain		= MAX_VOLUME_DEFAULT;		// Set max volume default
	ts.audio_gain_active = 1;						// this variable is used in the active RX audio processing function
	ts.rf_gain			= DEFAULT_RF_GAIN;			//
	ts.max_rf_gain		= MAX_RF_GAIN_DEFAULT;		// setting for maximum gain (e.g. minimum S-meter reading)
	ts.rf_codec_gain	= DEFAULT_RF_CODEC_GAIN_VAL;	// Set default RF gain (0 = lowest, 8 = highest, 9 = "Auto")
	ts.rit_value		= 0;						// RIT value
	ts.agc_mode			= AGC_DEFAULT;				// AGC setting
	ts.agc_custom_decay	= AGC_CUSTOM_DEFAULT;		// Default setting for AGC custom setting - higher = slower
	ts.filter_id		= AUDIO_DEFAULT_FILTER;		// startup audio filter
	ts.filter_300Hz_select	= FILTER_300HZ_DEFAULT;	// Select 750 Hz center filter as default
	ts.filter_500Hz_select	= FILTER_500HZ_DEFAULT;	// Select 750 Hz center filter as default
	ts.filter_1k8_select	= FILTER_1K8_DEFAULT;	// Select 1425 Hz center filter as default
	ts.filter_2k3_select	= FILTER_2K3_DEFAULT;	// Select 1412 Hz center filter as default
	ts.filter_3k6_select	= FILTER_3K6_DEFAULT;	// This is enabled by default
	ts.filter_wide_select	= FILTER_WIDE_DEFAULT;	// This is enabled by default
	//
	ts.st_gain			= DEFAULT_SIDETONE_GAIN;	// Sidetone gain
	ts.keyer_mode		= CW_MODE_IAM_B;			// CW keyer mode
	ts.keyer_speed		= DEFAULT_KEYER_SPEED;		// CW keyer speed
	ts.sidetone_freq	= CW_SIDETONE_FREQ_DEFAULT;	// CW sidetone and TX offset frequency
	ts.paddle_reverse	= 0;						// Paddle defaults to NOT reversed
	ts.cw_rx_delay		= CW_RX_DELAY_DEFAULT;		// Delay of TX->RX turnaround
	ts.unmute_delay_count		= SSB_RX_DELAY;		// Used to time TX->RX delay turnaround
	//
	ts.nb_setting		= 0;						// Noise Blanker setting
	//
	ts.tx_iq_lsb_gain_balance 	= 0;				// Default settings for RX and TX gain and phase balance
	ts.tx_iq_usb_gain_balance 	= 0;
	ts.tx_iq_lsb_gain_balance 	= 0;
	ts.tx_iq_usb_gain_balance 	= 0;
	ts.rx_iq_lsb_gain_balance = 0;
	ts.rx_iq_usb_gain_balance = 0;
	ts.rx_iq_lsb_phase_balance = 0;
	ts.rx_iq_usb_phase_balance = 0;
	ts.rx_iq_am_gain_balance = 0;
	ts.rx_iq_fm_gain_balance = 0;
	ts.tx_iq_am_gain_balance = 0;
	ts.tx_iq_fm_gain_balance = 0;
	//
	ts.tune_freq		= 0;
	ts.tune_freq_old	= 0;
	//
//	ts.calib_mode		= 0;						// calibrate mode
	ts.menu_mode		= 0;						// menu mode
	ts.menu_item		= 0;						// menu item selection
	ts.menu_var			= 0;						// menu item change variable
	ts.menu_var_changed	= 0;						// TRUE if a menu variable was changed and that an EEPROM save should be done

	//ts.txrx_lock		= 0;						// unlocked on start
	ts.audio_unmute		= 0;						// delayed un-mute not needed
	ts.buffer_clear		= 0;						// used on return from TX to purge the audio buffers

	ts.tx_audio_source	= TX_AUDIO_MIC;				// default source is microphone
	ts.tx_mic_gain		= MIC_GAIN_DEFAULT;			// default microphone gain
	ts.tx_mic_gain_mult	= ts.tx_mic_gain;			// actual operating value for microphone gain
	ts.mic_boost		= 0;
	ts.tx_line_gain		= LINE_GAIN_DEFAULT;		// default line gain

	ts.tune				= 0;						// reset tuning flag

	ts.tx_power_factor	= 0.50;						// TX power factor

	ts.pa_bias			= DEFAULT_PA_BIAS;			// Use lowest possible voltage as default
	ts.pa_cw_bias		= DEFAULT_PA_BIAS;			// Use lowest possible voltage as default (nonzero sets separate bias for CW mode)
	ts.freq_cal			= 0;							// Initial setting for frequency calibration
	ts.power_level		= PA_LEVEL_DEFAULT;			// See mchf_board.h for setting
	//
//	ts.codec_vol		= 0;						// Holder for codec volume
//	ts.codec_mute_state	= 0;						// Holder for codec mute state
//	ts.codec_was_muted = 0;							// Indicator that codec *was* muted
	//
	ts.powering_down	= 0;						// TRUE if powering down
	//
	ts.scope_speed		= SPECTRUM_SCOPE_SPEED_DEFAULT;	// default rate of spectrum scope update

	ts.waterfall_speed	= WATERFALL_SPEED_DEFAULT_SPI;		// default speed of update of the waterfall for parallel displays
	//
	ts.scope_filter		= SPECTRUM_SCOPE_FILTER_DEFAULT;	// default filter strength for spectrum scope
	ts.scope_trace_colour	= SPEC_COLOUR_TRACE_DEFAULT;	// default colour for the spectrum scope trace
	ts.scope_grid_colour	= SPEC_COLOUR_GRID_DEFAULT;		// default colour for the spectrum scope grid
	ts.scope_grid_colour_active = Grid;
	ts.scope_centre_grid_colour = SPEC_COLOUR_GRID_DEFAULT;		// color of center line of scope grid
	ts.scope_centre_grid_colour_active = Grid;
	ts.scope_scale_colour	= SPEC_COLOUR_SCALE_DEFAULT;	// default colour for the spectrum scope frequency scale at the bottom
	ts.scope_agc_rate	= SPECTRUM_SCOPE_AGC_DEFAULT;		// load default spectrum scope AGC rate
	ts.spectrum_db_scale = DB_DIV_10;					// default to 10dB/division
	//
	ts.menu_item		= 0;						// start out with a reasonable menu item
	//
	ts.radio_config_menu_enable = 0;				// TRUE if radio configuration menu is to be enabled
	//
	ts.cat_mode_active	= 0;						// TRUE if CAT mode is active
	//
	ts.xverter_mode		= 0;						// TRUE if transverter mode is active (e.g. offset of display)
	ts.xverter_offset	= 0;						// Frequency offset in transverter mode (added to frequency display)
	//
	ts.refresh_freq_disp	= 1;					// TRUE if frequency/color display is to be refreshed when next called - NORMALLY LEFT AT 0 (FALSE)!!!
													// This is NOT reset by the LCD function, but must be enabled/disabled externally
	//
	ts.pwr_2200m_5w_adj	= 1;
	ts.pwr_630m_5w_adj	= 1;
	ts.pwr_160m_5w_adj	= 1;
	ts.pwr_80m_5w_adj	= 1;
	ts.pwr_60m_5w_adj	= 1;
	ts.pwr_40m_5w_adj	= 1;
	ts.pwr_30m_5w_adj	= 1;
	ts.pwr_20m_5w_adj	= 1;
	ts.pwr_17m_5w_adj	= 1;
	ts.pwr_15m_5w_adj	= 1;
	ts.pwr_12m_5w_adj	= 1;
	ts.pwr_10m_5w_adj	= 1;
	ts.pwr_6m_5w_adj	= 1;
	ts.pwr_4m_5w_adj	= 1;
	ts.pwr_2m_5w_adj	= 1;
	ts.pwr_70cm_5w_adj	= 1;
	ts.pwr_23cm_5w_adj	= 1;
	//
	ts.filter_cw_wide_disable		= 0;			// TRUE if wide filters are to be disabled in CW mode
	ts.filter_ssb_narrow_disable	= 0;			// TRUE if narrow (CW) filters are to be disabled in SSB mdoe
	ts.am_mode_disable				= 0;			// TRUE if AM mode is to be disabled
	//
	ts.tx_meter_mode	= METER_SWR;
	//
	ts.alc_decay		= ALC_DECAY_DEFAULT;		// ALC Decay (release) default value
	ts.alc_decay_var	= ALC_DECAY_DEFAULT;		// ALC Decay (release) default value
	ts.alc_tx_postfilt_gain		= ALC_POSTFILT_GAIN_DEFAULT;	// Post-filter added gain default (used for speech processor/ALC)
	ts.alc_tx_postfilt_gain_var	= ALC_POSTFILT_GAIN_DEFAULT;	// Post-filter added gain default (used for speech processor/ALC)
	ts.tx_comp_level	= 0;		// 0=Release Time/Pre-ALC gain manually adjusted, >=1:  Value calculated by this parameter
	//
	ts.freq_step_config		= 0;			// disabled both marker line under frequency and swapping of STEP buttons
	//
	ts.nb_disable		= 0;				// TRUE if noise blanker is to be disabled
	//
	ts.dsp_active		= 0;				// TRUE if DSP noise reduction is to be enabled
	ts.digital_mode		= 0;				// digital modes OFF by default
	ts.dsp_active_toggle	= 0xff;			// used to hold the button G2 "toggle" setting.
	ts.dsp_nr_delaybuf_len = DSP_NR_BUFLEN_DEFAULT;
	ts.dsp_nr_strength	= 0;				// "Strength" of DSP noise reduction (0 = weak)
	ts.dsp_nr_numtaps 	= DSP_NR_NUMTAPS_DEFAULT;		// default for number of FFT taps for noise reduction
	ts.dsp_notch_numtaps = DSP_NOTCH_NUMTAPS_DEFAULT;	// default for number of FFT taps for notch filter
	ts.dsp_notch_delaybuf_len =	DSP_NOTCH_DELAYBUF_DEFAULT;
	ts.dsp_inhibit		= 1;				// TRUE if DSP is to be inhibited - power up with DSP disabled
	ts.dsp_inhibit_mute = 0;				// holder for "dsp_inhibit" during muting operations to allow restoration of previous state
	ts.dsp_timed_mute	= 0;				// TRUE if DSP is to be muted for a timed amount
	ts.dsp_inhibit_timing = 0;				// used to time inhibiting of DSP when it must be turned off for some reason
	ts.reset_dsp_nr		= 0;				// TRUE if DSP NR coefficients are to be reset when "audio_driver_set_rx_audio_filter()" is called
	ts.lcd_backlight_brightness = 0;		// = 0 full brightness
	ts.lcd_backlight_blanking = 0;			// MSB = 1 for auto-off of backlight, lower nybble holds time for auto-off in seconds
	//
	ts.tune_step		= 0;				// Used for press-and-hold step size changing mode
	ts.frequency_lock	= 0;				// TRUE if frequency knob is locked
	//
	ts.tx_disable		= 0;				// TRUE if transmitter is to be disabled
	ts.misc_flags1		= 0;				// Used to hold individual status flags, stored in EEPROM location "EEPROM_MISC_FLAGS1"
	ts.misc_flags2		= 0;				// Used to hold individual status flags, stored in EEPROM location "EEPROM_MISC_FLAGS2"
	ts.sysclock			= 0;				// This counts up from zero when the unit is powered up at precisely 100 Hz over the long term.  This
											// is NEVER reset and is used for timing certain events.
	ts.version_number_release	= 0;		// version release - used to detect firmware change
	ts.version_number_build = 0;			// version build - used to detect firmware change
	ts.nb_agc_time_const	= 0;			// used to calculate the AGC time constant
	ts.cw_offset_mode	= 0;				// CW offset mode (USB, LSB, etc.)
	ts.cw_lsb			= 0;				// Flag that indicates CW operates in LSB mode when TRUE
	ts.iq_freq_mode		= 0;				// used to set/configure the I/Q frequency/conversion mode
	ts.conv_sine_flag	= 0;				// FALSE until the sine tables for the frequency conversion have been built (normally zero, force 0 to rebuild)
	ts.lsb_usb_auto_select	= 0;			// holds setting of LSB/USB auto-select above/below 10 MHz
	ts.hold_off_spectrum_scope	= 0;		// this is a timer used to hold off updates of the spectrum scope when an SPI LCD display interface is used
	ts.lcd_blanking_time = 0;				// this holds the system time after which the LCD is blanked - if blanking is enabled
	ts.lcd_blanking_flag = 0;				// if TRUE, the LCD is blanked completely (e.g. backlight is off)
	ts.freq_cal_adjust_flag = 0;			// set TRUE if frequency calibration is in process
	ts.xvtr_adjust_flag = 0;				// set TRUE if transverter offset adjustment is in process
	ts.rx_muting = 0;						// set TRUE if audio output is to be muted
	ts.rx_blanking_time = 0;				// this is a timer used to delay the un-blanking of the audio after a large synthesizer tuning step
	ts.vfo_mem_mode = 0;					// this is used to record the VFO/memory mode (0 = VFO "A" = backwards compatibility)
											// LSB+6 (0x40) = 0:  VFO A,  1 = VFO B
											// LSB+7 (0x80) = 0:  Normal mode, 1 = SPLIT mode
											// Other bits are currently reserved
	ts.voltmeter_calibrate	= POWER_VOLTMETER_CALIBRATE_DEFAULT;	// Voltmeter calibration constant
	ts.thread_timer = 0;					// used to time thread
	ts.waterfall_color_scheme = WATERFALL_COLOR_DEFAULT;	// color scheme for waterfall display
	ts.waterfall_vert_step_size = WATERFALL_STEP_SIZE_DEFAULT;		// step size in waterfall display
	ts.waterfall_offset = WATERFALL_OFFSET_DEFAULT;		// Offset for waterfall display (brightness)
	ts.waterfall_contrast = WATERFALL_CONTRAST_DEFAULT;	// contrast setting for waterfall display
	ts.spectrum_scope_scheduler = 0;		// timer for scheduling the next update of the spectrum scope update
	ts.spectrum_scope_nosig_adjust = SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT;	// Adjustment for no signal adjustment conditions for spectrum scope
	ts.waterfall_nosig_adjust = WATERFALL_NOSIG_ADJUST_DEFAULT;		// Adjustment for no signal adjustment conditions for waterfall
	ts.waterfall_size	= WATERFALL_SIZE_DEFAULT;		// adjustment for waterfall size
	ts.fft_window_type = FFT_WINDOW_DEFAULT;		// FFT Windowing type
	ts.dvmode = 0;						// disable "DV" mode RX/TX functions by default
	ts.tx_audio_muting_timing = 0;			// timing value used for muting TX audio when keying PTT to suppress "click" or "thump"
	ts.tx_audio_muting_timer = 0;			// timer used for muting TX audio when keying PTT to suppress "click" or "thump"
	ts.tx_audio_muting_flag = 0;			// when TRUE, audio is to be muted after PTT/keyup
	ts.filter_disp_colour = FILTER_DISP_COLOUR_DEFAULT;	//
	ts.vfo_mem_flag = 0;				// when TRUE, memory mode is enabled
	ts.mem_disp = 0;					// when TRUE, memory display is enabled
	ts.load_eeprom_defaults = 0;			// when TRUE, defaults are loaded when "UiDriverLoadEepromValues()" is called - must be saved by user w/power-down to be permanent!
	ts.fm_subaudible_tone_gen_select = 0;		// lookup ("tone number") used to index the table generation (0 corresponds to "tone disabled")
	ts.fm_tone_burst_mode = 0;			// this is the setting for the tone burst generator
	ts.fm_tone_burst_timing = 0;			// used to time the duration of the tone burst
	ts.fm_sql_threshold = FM_SQUELCH_DEFAULT;	// squelch threshold
	ts.fm_rx_bandwidth = FM_BANDWIDTH_DEFAULT;	// bandwidth setting for FM reception
	ts.fm_subaudible_tone_det_select = 0;		// lookup ("tone number") used to index the table for tone detection (0 corresponds to "tone disabled")
	ts.beep_active = 1;				// TRUE if beep is active
	ts.beep_frequency = DEFAULT_BEEP_FREQUENCY;	// beep frequency, in Hz
	ts.beep_loudness = DEFAULT_BEEP_LOUDNESS;	// loudness of keyboard/CW sidetone test beep
	ts.load_freq_mode_defaults = 0;			// when TRUE, load frequency defaults into RAM when "UiDriverLoadEepromValues()" is called - MUST be saved by user IF these are to take effect!
	ts.boot_halt_flag = 0;				// when TRUE, boot-up is halted - used to allow various test functions
	ts.mic_bias = 0;				// mic bias off
	ts.ser_eeprom_type = 0;				// serial eeprom not present
	ts.ser_eeprom_in_use = 0xFF;			// serial eeprom not in use
	ts.eeprombuf = 0x00;				// pointer to RAM - dynamically loaded
	ts.tp_present = 0;				// default no touchscreen
	ts.tp_x = 0xFF;					// invalid position
	ts.tp_y = 0xFF;					// invalid position
	ts.show_tp_coordinates = 0;			// dont show coordinates on LCD
	ts.rfmod_present = 0;				// rfmod not present
	ts.vhfuhfmod_present = 0;			// VHF/UHF mod not present
}

//*----------------------------------------------------------------------------
//* Function Name       : MiscInit
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void MiscInit(void)
{
	//printf("misc init...\n\r");

	// Init Soft DDS
	softdds_setfreq(0.0,ts.samp_rate,0);
	//softdds_setfreq(500.0,ts.samp_rate,0);
	//softdds_setfreq(1000.0,ts.samp_rate,0);
	//softdds_setfreq(2000.0,ts.samp_rate,0);
	//softdds_setfreq(3000.0,ts.samp_rate,0);
	//softdds_setfreq(4000.0,ts.samp_rate,0);

	//printf("misc init ok\n\r");
}

/*
static void wd_reset(void)
{
	// Init WD
	if(!wd_init_enabled)
	{
		// Start watchdog
		WWDG_Enable(WD_REFRESH_COUNTER);

		// Reset
		wd_init_enabled = 1;
		TimingDelay 	= 0;

		return;
	}

	// 40mS flag for WD reset
	if(TimingDelay > 40)
	{
		TimingDelay = 0;
		//GREEN_LED_PIO->ODR ^= RED_LED;

		// Update WWDG counter
		WWDG_SetCounter(WD_REFRESH_COUNTER);
	}
}
*/
//*----------------------------------------------------------------------------
//* Function Name       : main
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
int main(void)
{
*(__IO uint32_t*)(SRAM2_BASE) = 0x0;	// clearing delay prevent for bootloader

	// Set unbuffered mode for stdout (newlib)
	//setvbuf( stdout, 0, _IONBF, 0 );

//	SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

	// HW init
	mchf_board_init();


	// Power on
	mchf_board_green_led(1);

	// Set default transceiver state
	TransceiverStateInit();

	// virtual Eeprom init
	ts.ee_init_stat = EE_Init();	// get status of EEPROM initialization

	ts.ser_eeprom_in_use = 0xFF;				// serial EEPROM not in use yet

	// serial EEPROM init
//	Write_24Cxx(0,0xFF,16);		//enable to reset EEPROM and force new copyvirt2ser
	if(Read_24Cxx(0,8) == 0xFE00)
	    ts.ser_eeprom_type = 0;				// no serial EEPROM availbale
	else
	    {
	    if(Read_24Cxx(0,16) != 0xFF)
		{
		if(Read_24Cxx(0,8) > 6 && Read_24Cxx(0,8) < 9 && Read_24Cxx(1,8) == 0x10)
		    {
		    ts.ser_eeprom_type = Read_24Cxx(0,8);
		    ts.ser_eeprom_in_use = Read_24Cxx(1,ts.ser_eeprom_type);
		    }
		else
		    {
		    ts.ser_eeprom_type = Read_24Cxx(0,16);
		    ts.ser_eeprom_in_use = Read_24Cxx(1,ts.ser_eeprom_type);
		    }
		}
	    else 
		{
		    {
		    Write_24Cxx(10,0xdd,8);
		    if(Read_24Cxx(10,8) == 0xdd)
			{						// 8 bit addressing
		    	Write_24Cxx(3,0x99,8);				// write testsignature
			ts.ser_eeprom_type = 7;				// smallest possible 8 bit EEPROM
			if(Read_24Cxx(0x83,8) != 0x99)
			    ts.ser_eeprom_type = 8;
			Write_24Cxx(0,ts.ser_eeprom_type,8);
			Write_24Cxx(1,0x10,8);
			}
		    else
			{						// 16 bit addressing
			if(Read_24Cxx(0x10000,17) != 0xFE00)
			    {
			    ts.ser_eeprom_type = 17;			// 24LC1025
			    Write_24Cxx(0,17,16);
			    }
			if(Read_24Cxx(0x10000,18) != 0xFE00)
			    {
			    ts.ser_eeprom_type = 18;			// 24LC1026
			    Write_24Cxx(0,18,16);
			    }
			if(Read_24Cxx(0x10000,19) != 0xFE00)
			    {
			    ts.ser_eeprom_type = 19;			// 24CM02
			    Write_24Cxx(0,19,16);
			    }
			if(ts.ser_eeprom_type < 17)
			    {
			    Write_24Cxx(3,0x66,16);			// write testsignature 1
			    Write_24Cxx(0x103,0x77,16);			// write testsignature 2
			    if(Read_24Cxx(3,16) == 0x66 && Read_24Cxx(0x103,16) == 0x77)
				{					// 16 bit addressing
				ts.ser_eeprom_type = 9;			// smallest possible 16 bit EEPROM
				if(Read_24Cxx(0x803,16) != 0x66)
				    ts.ser_eeprom_type = 12;
				if(Read_24Cxx(0x1003,16) != 0x66)
				    ts.ser_eeprom_type = 13;
				if(Read_24Cxx(0x2003,16) != 0x66)
				    ts.ser_eeprom_type = 14;
				if(Read_24Cxx(0x4003,16) != 0x66)
				    ts.ser_eeprom_type = 15;
				if(Read_24Cxx(0x8003,16) != 0x66)
				    ts.ser_eeprom_type = 16;
				Write_24Cxx(0,ts.ser_eeprom_type,16);
				}
			    }
			}
		    }
		}
	    if(ts.ser_eeprom_type < 16)				// incompatible EEPROMs
		ts.ser_eeprom_in_use = 0x10;			// serial EEPROM too small

	    if(ts.ser_eeprom_in_use == 0xFF)
		{
		copy_virt2ser();				// copy data from virtual to serial EEPROM
		verify_servirt();				// test if copy is corrupt
		Write_24Cxx(1, 0, ts.ser_eeprom_type);		// serial EEPROM in use now
		}
//	    ts.ser_eeprom_in_use = 0xFF;			// serial EEPROM use disable 4 debug
	}

	// test if touchscreen is present
	get_touchscreen_coordinates();				// initial reading of XPT2046
	if(ts.tp_x != 0xff && ts.tp_y != 0xff && ts.tp_x != 0 && ts.tp_y != 0) // touchscreen data valid?
	    ts.tp_present = 1;					// yes - touchscreen present!
	else
	    ts.tp_x = ts.tp_y = 0xff;

	// Show logo
	UiLcdHy28_ShowStartUpScreen(100);

	// Extra init
	MiscInit();

	// Init the RX Hilbert transform/filter prior
	// to initializing the audio!
	//
	UiCalcRxPhaseAdj();
	//
	// Init TX Hilbert transform/filter
	//
	UiCalcTxPhaseAdj();	//

	UiDriverLoadFilterValue();	// Get filter value so we can init audio with it

	// Audio HW init
	audio_driver_init();

	// Usb Host driver init
	//keyb_driver_init();

	// UI HW init
	ui_driver_init();

	// Audio HW init - again, using EEPROM-loaded values
	audio_driver_init();
	//
	//
	UiCalcSubaudibleGenFreq();		// load/set current FM subaudible tone settings for generation
	//
	UiCalcSubaudibleDetFreq();		// load/set current FM subaudible tone settings	for detection
	//
	UiLoadToneBurstMode();	// load/set tone burst frequency
	//
	UiLoadBeepFreq();		// load/set beep frequency
	//
	ts.audio_gain_change = 99;		// Force update of volume control
	uiCodecMute(0);					// make cure codec is un-muted

	UiCheckForEEPROMLoadDefaultRequest();	// check - and act on - request for loading of EEPROM defaults, if any
	//
	UiCheckForEEPROMLoadFreqModeDefaultRequest();	// check - and act on - request for loading frequency/mode defaults, if any
	//
	UiCheckForPressedKey();

	if (ts.cat_mode_active) cat_driver_init();

#ifdef DEBUG_BUILD
	printf("== main loop starting ==\n\r");
#endif

	// Transceiver main loop
	for(;;)
	{
		// UI events processing
		ui_driver_thread();
		CatDriverFT817CheckAndExecute();
		// Audio driver processing
		//audio_driver_thread();

		// USB Host driver processing
		//usbh_driver_thread();

		// Reset WD - not working
		//wd_reset();
	}
}
