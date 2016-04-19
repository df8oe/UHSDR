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
**  Licence:        CC BY-NC-SA 3.0                                                **
************************************************************************************/
#include "ui_configuration.h"

#include "ui_driver.h"

#include "audio_driver.h"

#include "ui_spectrum.h"

#include "ui_rotary.h" // df

#include "ui.h" // bandInfo

// Virtual eeprom
#include "eeprom.h"
#include "mchf_hw_i2c2.h"

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



#define UI_C_EEPROM_BAND_5W_PF(bandNo,bandName1,bandName2) { ConfigEntry_UInt8, EEPROM_BAND##bandNo##_5W,&ts.pwr_adj[ADJ_5W][BAND_MODE_##bandName1],TX_POWER_FACTOR_##bandName1##_DEFAULT,0,TX_POWER_FACTOR_MAX },
#define UI_C_EEPROM_BAND_FULL_PF(bandNo,bandName1,bandName2) { ConfigEntry_UInt8, EEPROM_BAND##bandNo##_FULL,&ts.pwr_adj[ADJ_FULL_POWER][BAND_MODE_##bandName1],TX_POWER_FACTOR_##bandName1##_DEFAULT,0,TX_POWER_FACTOR_MAX },

// here all simple configuration values are defined
// in order to have a new configuration value being stored in nonvolatile memory
// just add an entry. It is important to specify the addres of a ram location from  where to read/write  the
// respective value. As these configuration descriptors are constant values, the address has to be the address
// of a global (or static variable) the compiler know about.
// It is also important to use the matching typeId otherwise data will not be (re)store in a number of cases
// Some configuration values are currently not in this structure since saving them is more complex. See LoadEeprom/SaveEeprom functions.

const ConfigEntryDescriptor ConfigEntryInfo[] = {
    { ConfigEntry_UInt8, EEPROM_SPEC_SCOPE_SPEED,&ts.scope_speed,SPECTRUM_SCOPE_SPEED_DEFAULT,0,SPECTRUM_SCOPE_SPEED_MAX},
    { ConfigEntry_UInt32_16, EEPROM_FREQ_STEP,&df.selected_idx,3,0,T_STEP_MAX_STEPS-2},
    { ConfigEntry_UInt8, EEPROM_TX_AUDIO_SRC,&ts.tx_audio_source,0,0,TX_AUDIO_MAX_ITEMS},
    { ConfigEntry_UInt8, EEPROM_TCXO_STATE,&df.temp_enabled,TCXO_ON,0,TCXO_TEMP_STATE_MAX},
    { ConfigEntry_UInt8, EEPROM_AUDIO_GAIN,&ts.rx_gain[RX_AUDIO_SPKR].value,DEFAULT_AUDIO_GAIN,0,MAX_AUDIO_GAIN},
    { ConfigEntry_UInt8, EEPROM_RX_CODEC_GAIN,&ts.rf_codec_gain,DEFAULT_RF_CODEC_GAIN_VAL,0,MAX_RF_CODEC_GAIN_VAL},
    { ConfigEntry_Int32_16, EEPROM_RX_GAIN,&ts.rf_gain,DEFAULT_RF_GAIN,0,MAX_RF_GAIN},
    { ConfigEntry_UInt8, EEPROM_NB_SETTING,&ts.nb_setting,0,0,MAX_RF_ATTEN},
    { ConfigEntry_UInt8, EEPROM_TX_POWER_LEVEL,&ts.power_level,PA_LEVEL_DEFAULT,0,PA_LEVEL_MAX_ENTRY},
    { ConfigEntry_UInt8, EEPROM_KEYER_SPEED,&ts.keyer_speed,DEFAULT_KEYER_SPEED,MIN_KEYER_SPEED, MAX_KEYER_SPEED},
    { ConfigEntry_UInt8, EEPROM_KEYER_MODE,&ts.keyer_mode,CW_MODE_IAM_B, 0, CW_MAX_MODE},
    { ConfigEntry_UInt8, EEPROM_SIDETONE_GAIN,&ts.st_gain,DEFAULT_SIDETONE_GAIN,0, SIDETONE_MAX_GAIN},
    { ConfigEntry_Int32_16, EEPROM_FREQ_CAL,&ts.freq_cal,0,MIN_FREQ_CAL,MAX_FREQ_CAL},
    { ConfigEntry_UInt8, EEPROM_AGC_MODE,&ts.agc_mode,AGC_DEFAULT,0,AGC_MAX_MODE},
    { ConfigEntry_UInt8, EEPROM_MIC_GAIN,&ts.tx_gain[TX_AUDIO_MIC],MIC_GAIN_DEFAULT,MIC_GAIN_MIN,MIC_GAIN_MAX},
    { ConfigEntry_UInt8, EEPROM_LINE_GAIN,&ts.tx_gain[TX_AUDIO_LINEIN_L],LINE_GAIN_DEFAULT,LINE_GAIN_MIN,LINE_GAIN_MAX},
    { ConfigEntry_UInt32_16, EEPROM_SIDETONE_FREQ,&ts.sidetone_freq,CW_SIDETONE_FREQ_DEFAULT,CW_SIDETONE_FREQ_MIN,CW_SIDETONE_FREQ_MAX},
    { ConfigEntry_UInt8, EEPROM_SPEC_SCOPE_FILTER,&ts.scope_filter,SPECTRUM_SCOPE_FILTER_DEFAULT,SPECTRUM_SCOPE_FILTER_MIN,SPECTRUM_SCOPE_FILTER_MAX},
    { ConfigEntry_UInt8, EEPROM_AGC_CUSTOM_DECAY,&ts.agc_custom_decay,AGC_CUSTOM_DEFAULT,0,AGC_CUSTOM_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_TRACE_COLOUR,&ts.scope_trace_colour,SPEC_COLOUR_TRACE_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_GRID_COLOUR,&ts.scope_grid_colour,SPEC_COLOUR_GRID_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_CENTRE_GRID_COLOUR,&ts.scope_centre_grid_colour,SPEC_COLOUR_GRID_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_SCALE_COLOUR,&ts.scope_scale_colour,SPEC_COLOUR_SCALE_DEFAULT, 0, SPEC_MAX_COLOUR},
    { ConfigEntry_UInt8, EEPROM_PADDLE_REVERSE,&ts.paddle_reverse,0,0,1},
    { ConfigEntry_UInt8, EEPROM_CW_RX_DELAY,&ts.cw_rx_delay,CW_RX_DELAY_DEFAULT,0,CW_RX_DELAY_MAX},
    { ConfigEntry_UInt8, EEPROM_MAX_VOLUME,&ts.rx_gain[RX_AUDIO_SPKR].max,MAX_VOLUME_DEFAULT,MAX_VOLUME_MIN,MAX_VOLUME_MAX},
    { ConfigEntry_UInt8, EEPROM_PA_BIAS,&ts.pa_bias,DEFAULT_PA_BIAS,0,MAX_PA_BIAS},
    { ConfigEntry_UInt8, EEPROM_PA_CW_BIAS,&ts.pa_cw_bias,DEFAULT_PA_BIAS,0,MAX_PA_BIAS},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_LSB_GAIN_BALANCE,&ts.tx_iq_lsb_gain_balance,0, MIN_TX_IQ_GAIN_BALANCE, MAX_TX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_USB_GAIN_BALANCE,&ts.tx_iq_usb_gain_balance,0, MIN_TX_IQ_GAIN_BALANCE, MAX_TX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_LSB_PHASE_BALANCE,&ts.tx_iq_lsb_phase_balance,0, MIN_TX_IQ_PHASE_BALANCE, MAX_TX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_USB_PHASE_BALANCE,&ts.tx_iq_usb_phase_balance,0, MIN_TX_IQ_GAIN_BALANCE, MAX_TX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_LSB_GAIN_BALANCE,&ts.rx_iq_lsb_gain_balance,0, MIN_RX_IQ_GAIN_BALANCE, MAX_RX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_USB_GAIN_BALANCE,&ts.rx_iq_usb_gain_balance,0,  MIN_RX_IQ_GAIN_BALANCE, MAX_RX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_LSB_PHASE_BALANCE,&ts.rx_iq_lsb_phase_balance,0,  MIN_RX_IQ_PHASE_BALANCE, MAX_RX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_USB_PHASE_BALANCE,&ts.rx_iq_usb_phase_balance,0,  MIN_RX_IQ_PHASE_BALANCE, MAX_RX_IQ_PHASE_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_AM_GAIN_BALANCE,&ts.rx_iq_am_gain_balance,0,  MIN_RX_IQ_GAIN_BALANCE, MAX_RX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_RX_IQ_FM_GAIN_BALANCE,&ts.rx_iq_fm_gain_balance,0,  MIN_RX_IQ_GAIN_BALANCE, MAX_RX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_AM_GAIN_BALANCE,&ts.tx_iq_am_gain_balance,0, MIN_TX_IQ_GAIN_BALANCE, MAX_TX_IQ_GAIN_BALANCE},
    { ConfigEntry_Int32_16, EEPROM_TX_IQ_FM_GAIN_BALANCE,&ts.tx_iq_fm_gain_balance,0, MIN_TX_IQ_PHASE_BALANCE, MAX_TX_IQ_GAIN_BALANCE},
    { ConfigEntry_UInt8, EEPROM_SENSOR_NULL,&swrm.sensor_null,SENSOR_NULL_DEFAULT,SENSOR_NULL_MIN,SENSOR_NULL_MAX},
    { ConfigEntry_UInt8, EEPROM_XVERTER_DISP,&ts.xverter_mode,0,0,XVERTER_MULT_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_MAGNIFY,&sd.magnify,0,0,1},
    { ConfigEntry_UInt8, EEPROM_WIDE_FILT_CW_DISABLE,&ts.filter_cw_wide_disable,1,0,1},
    { ConfigEntry_UInt8, EEPROM_NARROW_FILT_SSB_DISABLE,&ts.filter_ssb_narrow_disable,1,0,1},
    { ConfigEntry_UInt8, EEPROM_AM_MODE_DISABLE,&ts.am_mode_disable,1,0,1},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_DB_DIV,&ts.spectrum_db_scale,DB_DIV_ADJUST_DEFAULT,DB_DIV_ADJUST_MIN, DB_DIV_ADJUST_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_AGC_RATE,&ts.scope_agc_rate,SPECTRUM_SCOPE_AGC_DEFAULT,SPECTRUM_SCOPE_AGC_MIN, SPECTRUM_SCOPE_AGC_MAX},
    { ConfigEntry_UInt8, EEPROM_METER_MODE,&ts.tx_meter_mode,METER_SWR,0,METER_MAX},
    { ConfigEntry_UInt32_16, EEPROM_ALC_DECAY_TIME,&ts.alc_decay,ALC_DECAY_DEFAULT,0,ALC_DECAY_MAX },
    { ConfigEntry_UInt32_16, EEPROM_ALC_POSTFILT_TX_GAIN,&ts.alc_tx_postfilt_gain,ALC_POSTFILT_GAIN_DEFAULT, ALC_POSTFILT_GAIN_MIN, ALC_POSTFILT_GAIN_MAX},
    { ConfigEntry_UInt8, EEPROM_STEP_SIZE_CONFIG,&ts.freq_step_config,0,0,255},
    { ConfigEntry_UInt8, EEPROM_DSP_MODE,&ts.dsp_active,0,0,255},
    { ConfigEntry_UInt8, EEPROM_DSP_NR_STRENGTH,&ts.dsp_nr_strength,DSP_NR_STRENGTH_DEFAULT,0, DSP_NR_STRENGTH_MAX},
    { ConfigEntry_UInt32_16, EEPROM_DSP_NR_DECOR_BUFLEN,&ts.dsp_nr_delaybuf_len,DSP_NR_BUFLEN_DEFAULT, DSP_NR_BUFLEN_MIN, DSP_NR_BUFLEN_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NR_FFT_NUMTAPS,&ts.dsp_nr_numtaps,DSP_NR_NUMTAPS_DEFAULT, DSP_NR_NUMTAPS_MIN, DSP_NOTCH_NUMTAPS_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NOTCH_DECOR_BUFLEN,&ts.dsp_notch_delaybuf_len,DSP_NOTCH_DELAYBUF_DEFAULT,DSP_NOTCH_BUFLEN_MIN,DSP_NOTCH_BUFLEN_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NOTCH_FFT_NUMTAPS,&ts.dsp_notch_numtaps,DSP_NOTCH_NUMTAPS_DEFAULT, DSP_NOTCH_NUMTAPS_MIN,DSP_NOTCH_NUMTAPS_MAX},
    { ConfigEntry_UInt8, EEPROM_DSP_NOTCH_CONV_RATE,&ts.dsp_notch_mu,DSP_NOTCH_MU_DEFAULT,0,DSP_NOTCH_MU_MAX},
    { ConfigEntry_UInt8, EEPROM_MAX_RX_GAIN,&ts.max_rf_gain,MAX_RF_GAIN_DEFAULT,0,MAX_RF_GAIN_MAX},
    { ConfigEntry_UInt8, EEPROM_TX_AUDIO_COMPRESS,&ts.tx_comp_level,TX_AUDIO_COMPRESSION_DEFAULT,0,TX_AUDIO_COMPRESSION_MAX},
    { ConfigEntry_UInt8, EEPROM_TX_DISABLE,&ts.tx_disable,0,0,1},
    { ConfigEntry_UInt8, EEPROM_MISC_FLAGS1,&ts.misc_flags1,0,0,255},
    { ConfigEntry_UInt8, EEPROM_MISC_FLAGS2,&ts.misc_flags2,0,0,255},
    { ConfigEntry_UInt16, EEPROM_VERSION_MINOR,&ts.version_number_minor,0,0,255},
    { ConfigEntry_UInt16, EEPROM_VERSION_NUMBER,&ts.version_number_release,0,0,255},
    { ConfigEntry_UInt16, EEPROM_VERSION_BUILD,&ts.version_number_build,0,0,255},
    { ConfigEntry_UInt8, EEPROM_NB_AGC_TIME_CONST,&ts.nb_agc_time_const,NB_AGC_DEFAULT,0,NB_MAX_AGC_SETTING},
    { ConfigEntry_UInt8, EEPROM_CW_OFFSET_MODE,&ts.cw_offset_mode,CW_OFFSET_MODE_DEFAULT,0,CW_OFFSET_MAX},
    { ConfigEntry_UInt8, EEPROM_FREQ_CONV_MODE,&ts.iq_freq_mode,FREQ_IQ_CONV_MODE_DEFAULT,0,FREQ_IQ_CONV_MODE_MAX},
    { ConfigEntry_UInt8, EEPROM_LSB_USB_AUTO_SELECT,&ts.lsb_usb_auto_select,AUTO_LSB_USB_DEFAULT,0,AUTO_LSB_USB_MAX},
    { ConfigEntry_UInt8, EEPROM_LCD_BLANKING_CONFIG,&ts.lcd_backlight_blanking,0,0,255},
    { ConfigEntry_UInt32_16, EEPROM_VFO_MEM_MODE,&ts.vfo_mem_mode,0,0,255},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_2200M,&swrm.coupling_calc[COUPLING_2200M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_630M,&swrm.coupling_calc[COUPLING_630M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_160M,&swrm.coupling_calc[COUPLING_160M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_80M,&swrm.coupling_calc[COUPLING_80M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_40M,&swrm.coupling_calc[COUPLING_40M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_20M,&swrm.coupling_calc[COUPLING_20M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_15M,&swrm.coupling_calc[COUPLING_15M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt8, EEPROM_DETECTOR_COUPLING_COEFF_6M,&swrm.coupling_calc[COUPLING_6M],SWR_COUPLING_DEFAULT,SWR_COUPLING_MIN,SWR_COUPLING_MAX},
    { ConfigEntry_UInt32_16, EEPROM_VOLTMETER_CALIBRATE,&ts.voltmeter_calibrate,POWER_VOLTMETER_CALIBRATE_DEFAULT,POWER_VOLTMETER_CALIBRATE_MIN,POWER_VOLTMETER_CALIBRATE_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_COLOR_SCHEME,&ts.waterfall_color_scheme,WATERFALL_COLOR_DEFAULT,WATERFALL_COLOR_MIN,WATERFALL_COLOR_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_VERTICAL_STEP_SIZE,&ts.waterfall_vert_step_size,WATERFALL_STEP_SIZE_DEFAULT,WATERFALL_STEP_SIZE_MIN,WATERFALL_STEP_SIZE_MAX},
    { ConfigEntry_UInt32_16, EEPROM_WATERFALL_OFFSET,&ts.waterfall_offset,WATERFALL_OFFSET_DEFAULT,WATERFALL_OFFSET_MIN,WATERFALL_OFFSET_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_SIZE,&ts.waterfall_size,WATERFALL_SIZE_DEFAULT,0,WATERFALL_BIG},
    { ConfigEntry_UInt32_16, EEPROM_WATERFALL_CONTRAST,&ts.waterfall_contrast,WATERFALL_CONTRAST_DEFAULT,WATERFALL_CONTRAST_MIN,WATERFALL_CONTRAST_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_SPEED,&ts.waterfall_speed,WATERFALL_SPEED_DEFAULT_PARALLEL,0,WATERFALL_SPEED_MAX},
    { ConfigEntry_UInt8, EEPROM_SPECTRUM_SCOPE_NOSIG_ADJUST,&ts.spectrum_scope_nosig_adjust,SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT,SPECTRUM_SCOPE_NOSIG_ADJUST_MIN,SPECTRUM_SCOPE_NOSIG_ADJUST_MAX},
    { ConfigEntry_UInt8, EEPROM_WATERFALL_NOSIG_ADJUST,&ts.waterfall_nosig_adjust,SPECTRUM_SCOPE_NOSIG_ADJUST_DEFAULT,WATERFALL_NOSIG_ADJUST_MIN,WATERFALL_NOSIG_ADJUST_MAX},
    { ConfigEntry_UInt8, EEPROM_FFT_WINDOW,&ts.fft_window_type,FFT_WINDOW_DEFAULT,0,FFT_WINDOW_MAX},
    { ConfigEntry_UInt8, EEPROM_TX_PTT_AUDIO_MUTE,&ts.tx_audio_muting_timing,0,0,TX_PTT_AUDIO_MUTE_DELAY_MAX},
    { ConfigEntry_UInt8, EEPROM_FILTER_DISP_COLOUR,&ts.filter_disp_colour,0,0,SPEC_MAX_COLOUR},
    { ConfigEntry_UInt32_16, EEPROM_FM_SUBAUDIBLE_TONE_GEN,&ts.fm_subaudible_tone_gen_select,FM_SUBAUDIBLE_TONE_OFF,0,NUM_SUBAUDIBLE_TONES},
    { ConfigEntry_UInt32_16, EEPROM_FM_SUBAUDIBLE_TONE_DET,&ts.fm_subaudible_tone_det_select,FM_SUBAUDIBLE_TONE_OFF,0,NUM_SUBAUDIBLE_TONES},
    { ConfigEntry_UInt8, EEPROM_FM_TONE_BURST_MODE,&ts.fm_tone_burst_mode,FM_TONE_BURST_OFF,0,FM_TONE_BURST_MAX},
    { ConfigEntry_UInt8, EEPROM_FM_SQUELCH_SETTING,&ts.fm_sql_threshold,FM_SQUELCH_DEFAULT,0,FM_SQUELCH_MAX},
//    { ConfigEntry_UInt8, EEPROM_FM_RX_BANDWIDTH,&ts.fm_rx_bandwidth,FM_BANDWIDTH_DEFAULT,0,FM_RX_BANDWIDTH_MAX},
    { ConfigEntry_UInt32_16, EEPROM_KEYBOARD_BEEP_FREQ,&ts.beep_frequency,DEFAULT_BEEP_FREQUENCY,MIN_BEEP_FREQUENCY,MAX_BEEP_FREQUENCY},
    { ConfigEntry_UInt8, EEPROM_BEEP_LOUDNESS,&ts.beep_loudness,DEFAULT_BEEP_LOUDNESS,0,MAX_BEEP_LOUDNESS},
    { ConfigEntry_Bool, EEPROM_CAT_MODE_ACTIVE,&ts.cat_mode_active,0,0,1},
    { ConfigEntry_UInt8, EEPROM_TUNE_POWER_LEVEL,&ts.tune_power_level,PA_LEVEL_MAX_ENTRY,PA_LEVEL_FULL,PA_LEVEL_MAX_ENTRY},
    { ConfigEntry_UInt8, EEPROM_CAT_XLAT,&ts.xlat,1,0,1},
    { ConfigEntry_Bool, EEPROM_DYNAMIC_TUNING,&ts.dynamic_tuning_active,0,0,1},
    { ConfigEntry_Bool, EEPROM_SAM_ENABLE,&ts.sam_enabled,0,0,1},
    { ConfigEntry_Bool, EEPROM_CAT_IN_SANDBOX,&ts.cat_in_sandbox,0,0,1},
    { ConfigEntry_Bool, EEPROM_SPECTRUM_LIGHT_ENABLE,&ts.spectrum_light ,0,0,1},
	UI_C_EEPROM_BAND_5W_PF( 0,80,m)
    UI_C_EEPROM_BAND_5W_PF(1,60,m)
    UI_C_EEPROM_BAND_5W_PF(2,40,m)
    UI_C_EEPROM_BAND_5W_PF(3,30,m)
    UI_C_EEPROM_BAND_5W_PF(4,20,m)
    UI_C_EEPROM_BAND_5W_PF(5,17,m)
    UI_C_EEPROM_BAND_5W_PF(6,15,m)
    UI_C_EEPROM_BAND_5W_PF(7,12,m)
    UI_C_EEPROM_BAND_5W_PF(8,10,m)
    UI_C_EEPROM_BAND_5W_PF(9,6,m)
    UI_C_EEPROM_BAND_5W_PF(10,4,m)
    UI_C_EEPROM_BAND_5W_PF(11,2,m)
    UI_C_EEPROM_BAND_5W_PF(12,70,cm)
    UI_C_EEPROM_BAND_5W_PF(13,23,cm)
    UI_C_EEPROM_BAND_5W_PF(14,2200,m)
    UI_C_EEPROM_BAND_5W_PF(15,630,m)
    UI_C_EEPROM_BAND_5W_PF(16,160,m)
    UI_C_EEPROM_BAND_FULL_PF(0,80,m)
    UI_C_EEPROM_BAND_FULL_PF(1,60,m)
    UI_C_EEPROM_BAND_FULL_PF(2,40,m)
    UI_C_EEPROM_BAND_FULL_PF(3,30,m)
    UI_C_EEPROM_BAND_FULL_PF(4,20,m)
    UI_C_EEPROM_BAND_FULL_PF(5,17,m)
    UI_C_EEPROM_BAND_FULL_PF(6,15,m)
    UI_C_EEPROM_BAND_FULL_PF(7,12,m)
    UI_C_EEPROM_BAND_FULL_PF(8,10,m)
    UI_C_EEPROM_BAND_FULL_PF(9,6,m)
    UI_C_EEPROM_BAND_FULL_PF(10,4,m)
    UI_C_EEPROM_BAND_FULL_PF(11,2,m)
    UI_C_EEPROM_BAND_FULL_PF(12,70,cm)
    UI_C_EEPROM_BAND_FULL_PF(13,23,cm)
    UI_C_EEPROM_BAND_FULL_PF(14,2200,m)
    UI_C_EEPROM_BAND_FULL_PF(15,630,m)
    UI_C_EEPROM_BAND_FULL_PF(16,160,m)
    // the entry below MUST be the last entry, and only at the last position Stop is allowed
    {ConfigEntry_Stop}
};

// TODO: LINEAR SEARCH IS VERY BAD FOR LARGER SETS, replace with more clever strategy.
// for use case in menu this acceptable, however
const ConfigEntryDescriptor* UiConfiguration_GetEntry(uint16_t id) {
  int idx;
  const ConfigEntryDescriptor* retval = NULL;
    for (idx = 0; ConfigEntryInfo[idx].typeId != ConfigEntry_Stop; idx++) {
      if (ConfigEntryInfo[idx].id == id) {
        retval = &ConfigEntryInfo[idx];
        break;
      }
    }
    return retval;
}



static void __attribute__ ((noinline)) UiReadSettingEEPROM_Bool(uint16_t addr, volatile bool* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val ) {
    uint16_t value;
    if(Read_EEPROM(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiReadSettingEEPROM_UInt8(uint16_t addr, volatile uint8_t* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val ) {
    uint16_t value;
    if(Read_EEPROM(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiReadSettingEEPROM_UInt16(uint16_t addr, volatile uint16_t* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val ) {
    uint16_t value;
    if(Read_EEPROM(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}


static void __attribute__ ((noinline)) UiReadSettingEEPROM_UInt32_16(uint16_t addr, volatile uint32_t* val_ptr, uint16_t default_val, uint16_t min_val, uint16_t max_val ) {
    uint16_t value;
    if(Read_EEPROM(addr, &value) == 0)
    {
        *val_ptr = value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiReadSettingEEPROM_Int32_16(uint16_t addr, volatile int32_t* val_ptr, int default_val, int min_val, int max_val ) {
    uint16_t value;
    if(Read_EEPROM(addr, &value) == 0)
    {
        *val_ptr = (int16_t)value;
        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}

static void UiReadSettingEEPROM_UInt32(uint16_t addrH, uint16_t addrL, volatile uint32_t* val_ptr, uint32_t default_val, uint32_t min_val, uint32_t max_val) {
    uint16_t valueH,valueL;
    if(Read_EEPROM(addrH, &valueH) == 0 && Read_EEPROM(addrL, &valueL) == 0)
    {

        *val_ptr = valueH;
        *val_ptr <<=16;
        *val_ptr |= valueL;

        if (*val_ptr < min_val || *val_ptr > max_val || ts.load_eeprom_defaults) {
            *val_ptr = default_val;
        }
    }
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_UInt16(uint16_t addr, uint16_t set_val, uint16_t default_val ) {
    Write_EEPROM(addr, set_val);
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_UInt32(uint16_t addrH, uint16_t addrL, uint32_t set_val, uint32_t default_val ) {
    Write_EEPROM(addrH, (uint16_t)(set_val >> 16));
    Write_EEPROM(addrL, (uint16_t)(set_val));
}

void UiReadSettingsBandMode(const uint8_t i, const uint16_t band_mode, const uint16_t band_freq_high, const uint16_t  band_freq_low,__IO VfoReg* vforeg) {
    uint32_t value32;
    uint16_t value16;

    UiReadSettingEEPROM_UInt16(band_mode + i,&value16,0,0,0xffff);
    {
        // Note that ts.band will, by definition, be equal to index "i"
        vforeg->decod_mode = (value16 >> 8) & 0x0F;     // demodulator mode might not be right for saved band!
        if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)       // valid mode value from EEPROM? or defaults loaded?
            vforeg->decod_mode = DEMOD_LSB;         // no - set to LSB
        //
    }

    // Try to read Freq saved values
    UiReadSettingEEPROM_UInt32(band_freq_high + i, band_freq_low + i,&value32,bandInfo[i].tune + DEFAULT_FREQ_OFFSET,0,0xffffffff);
    {
        //
        // We have loaded from eeprom the last used band, but can't just
        // load saved frequency, as it could be out of band, so do a
        // boundary check first (also check to see if defaults should be loaded)
        //
        if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (value32 >= bandInfo[i].tune) && (value32 <= (bandInfo[i].tune + bandInfo[i].size)))
        {
            vforeg->dial_value = value32;
        }
        else if((ts.misc_flags2 & MISC_FLAGS2_FREQ_MEM_LIMIT_RELAX) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))
        {   // xxxx relax memory-save frequency restrictions and is it within the allowed range?
            vforeg->dial_value = value32;
        }
        else
        {
            // Load default for this band
            vforeg->dial_value = bandInfo[i].tune + DEFAULT_FREQ_OFFSET;
        }
    }

}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_Bool(uint16_t addr, bool set_val, bool default_val ) {
    UiWriteSettingEEPROM_UInt16(addr,(uint16_t)set_val,(uint16_t)default_val);
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_UInt32_16(uint16_t addr, uint32_t set_val, uint16_t default_val ) {
    UiWriteSettingEEPROM_UInt16(addr,set_val,default_val);
}

static void __attribute__ ((noinline)) UiWriteSettingEEPROM_Int32_16(uint16_t addr, int32_t set_val, int32_t default_val ) {
    UiWriteSettingEEPROM_UInt16(addr,(uint16_t)(int16_t)set_val,default_val);
}


static void UiWriteSettingsBandMode(const uint16_t i,const uint16_t band_mode, const uint16_t band_freq_high, const uint16_t band_freq_low, __IO VfoReg* vforeg) {

    // ------------------------------------------------------------------------------------
    // Read Band and Mode saved values - update if changed
    UiWriteSettingEEPROM_UInt16(band_mode + i,
            (vforeg->decod_mode << 8),
            ((vforeg->decod_mode & 0x0f) << 8)
    );
    // Try to read Freq saved values - update if changed
    UiWriteSettingEEPROM_UInt32(band_freq_high+i,band_freq_low+i, vforeg->dial_value, vforeg->dial_value);
}

void UiWriteSettingEEPROM_Filter() {
  int idx, mem_idx;
  for (idx = 0; idx < FILTER_MODE_MAX;idx++) {
    for (mem_idx = 0; mem_idx < FILTER_PATH_MEM_MAX;mem_idx++) {
      UiWriteSettingEEPROM_UInt16(EEPROM_FILTER_PATH_MAP_BASE+idx*FILTER_PATH_MEM_MAX+mem_idx,ts.filter_path_mem[idx][mem_idx],0);
    }
  }
}

void UiReadSettingEEPROM_Filter()
{
  int idx, mem_idx;
  for (idx = 0; idx < FILTER_MODE_MAX;idx++) {
    for (mem_idx = 0; mem_idx < FILTER_PATH_MEM_MAX;mem_idx++) {
      UiReadSettingEEPROM_UInt16(EEPROM_FILTER_PATH_MAP_BASE+idx*FILTER_PATH_MEM_MAX+mem_idx,&(ts.filter_path_mem[idx][mem_idx]),0,0,AUDIO_FILTER_PATH_NUM-1);
    }
  }
}

void UiConfiguration_ReadConfigEntryData(const ConfigEntryDescriptor* ced_ptr) {
  switch(ced_ptr->typeId) {

  case ConfigEntry_UInt8:
    UiReadSettingEEPROM_UInt8(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
    break;
  case ConfigEntry_UInt16:
    UiReadSettingEEPROM_UInt16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
    break;
  case ConfigEntry_UInt32_16:
    UiReadSettingEEPROM_UInt32_16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
    break;
  case ConfigEntry_Int32_16:
    UiReadSettingEEPROM_Int32_16(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
    break;
  case ConfigEntry_Bool:
    UiReadSettingEEPROM_Bool(ced_ptr->id,ced_ptr->val_ptr,ced_ptr->val_default,ced_ptr->val_min,ced_ptr->val_max);
    break;

  }
}
void UiConfiguration_WriteConfigEntryData(const ConfigEntryDescriptor* ced_ptr) {
  switch(ced_ptr->typeId) {

  case ConfigEntry_UInt8:
    UiWriteSettingEEPROM_UInt16(ced_ptr->id,*(uint8_t*)ced_ptr->val_ptr,ced_ptr->val_default);
    break;
  case ConfigEntry_UInt16:
    UiWriteSettingEEPROM_UInt16(ced_ptr->id,*(uint16_t*)ced_ptr->val_ptr,ced_ptr->val_default);
    break;
  case ConfigEntry_UInt32_16:
    UiWriteSettingEEPROM_UInt32_16(ced_ptr->id,*(uint32_t*)ced_ptr->val_ptr,ced_ptr->val_default);
    break;
  case ConfigEntry_Int32_16:
    UiWriteSettingEEPROM_Int32_16(ced_ptr->id,*(int32_t*)ced_ptr->val_ptr,ced_ptr->val_default);
    break;
  case ConfigEntry_Bool:
    UiWriteSettingEEPROM_Bool(ced_ptr->id,*(bool*)ced_ptr->val_ptr,ced_ptr->val_default);
    break;
  }
}

void UiConfiguration_WriteConfigEntries() {
  int idx;
  for (idx = 0; ConfigEntryInfo[idx].typeId != ConfigEntry_Stop ;idx++) {
    UiConfiguration_WriteConfigEntryData(&ConfigEntryInfo[idx]);
  }
}

static void __attribute__ ((noinline)) UiConfiguration_ReadConfigEntries() {
  int idx;
  for (idx = 0; ConfigEntryInfo[idx].typeId != ConfigEntry_Stop ;idx++) {
    UiConfiguration_ReadConfigEntryData(&ConfigEntryInfo[idx]);
  }
}


//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverLoadEepromValues
//* Object              : load saved values on driver start
//* Input Parameters    : Indirect:  If "ts.load_eeprom_defaults" is TRUE, default values will be loaded instead of EEPROM values.
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void UiConfiguration_LoadEepromValues(void)
{
    bool dspmode = ts.dsp_inhibit;
    ts.dsp_inhibit = 1;     // disable dsp while loading EEPROM data

    uint16_t value16;
    uint32_t value32;
    // Do a sample reads to "prime the pump" before we start...
    // This is to make the function work reliabily after boot-up
    //
    UiReadSettingEEPROM_UInt16(EEPROM_ZERO_LOC_UNRELIABLE,&value16,0,0,0xffff);
    // Let's use location zero - which may not work reliably, anyway!
    //
    UiReadSettingEEPROM_UInt8(EEPROM_MISC_FLAGS2,&ts.misc_flags2,0,0,255);
    // ------------------------------------------------------------------------------------
    // Try to read Band and Mode saved values, but read freq-limit-settings before
    UiReadSettingEEPROM_UInt16(EEPROM_BAND_MODE,&value16,0,0,0xffff);
    {
        ts.band = value16 & 0x00FF;
        if(ts.band > MAX_BANDS-1)           // did we get an insane value from EEPROM?
            ts.band = BAND_MODE_80;     //  yes - set to 80 meters
        //
        ts.dmod_mode = (value16 >> 8) & 0x0F;       // demodulator mode might not be right for saved band!
        if((ts.dmod_mode > DEMOD_MAX_MODE)  || ts.load_eeprom_defaults || ts.load_freq_mode_defaults)       // valid mode value from EEPROM? or defaults loaded?
            ts.dmod_mode = DEMOD_LSB;           // no - set to LSB

    }
    // ------------------------------------------------------------------------------------
    // Try to read Freq saved values
    UiReadSettingEEPROM_UInt32(EEPROM_FREQ_HIGH,EEPROM_FREQ_LOW,&value32,0,0,0xffffffff);
    {

        // We have loaded from eeprom the last used band, but can't just
        // load saved frequency, as it could be out of band, so do a
        // boundary check first (also check to see if defaults should be loaded)
        if((!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults) && (value32 >= bandInfo[ts.band].tune) && (value32 <= (bandInfo[ts.band].tune + bandInfo[ts.band].size)))
        {
            df.tune_new = value32;
        }
        else if((ts.misc_flags2 & MISC_FLAGS2_FREQ_MEM_LIMIT_RELAX) && (!ts.load_eeprom_defaults) && (!ts.load_freq_mode_defaults))   {   // xxxx relax memory-save frequency restrictions and is it within the allowed range?
            df.tune_new = value32;
        }
        else
        {
            // Load default for this band
            df.tune_new = bandInfo[ts.band].tune;
        }
    }
    //
    // Try to read saved per-band values for frequency, mode and filter
    //


    uint8_t i;
    //
    for(i = 0; i < MAX_BANDS; i++)
    {   // read from stored bands
        // UiReadSettingsBandMode(i,EEPROM_BAND0_MODE,EEPROM_BAND0_FREQ_HIGH,EEPROM_BAND0_FREQ_LOW, &vfo[VFO_WORK].band[i]);
        UiReadSettingsBandMode(i,EEPROM_BAND0_MODE_A,EEPROM_BAND0_FREQ_HIGH_A,EEPROM_BAND0_FREQ_LOW_A, &vfo[VFO_A].band[i]);
        UiReadSettingsBandMode(i,EEPROM_BAND0_MODE_B,EEPROM_BAND0_FREQ_HIGH_B,EEPROM_BAND0_FREQ_LOW_B, &vfo[VFO_B].band[i]);
    }

    UiConfiguration_ReadConfigEntries();

    UiReadSettingEEPROM_UInt32( EEPROM_XVERTER_OFFSET_HIGH,EEPROM_XVERTER_OFFSET_LOW,&ts.xverter_offset,0,0,XVERTER_OFFSET_MAX);

    UiReadSettingEEPROM_Filter();

    // post configuration loading actions below
    df.tuning_step  = tune_steps[df.selected_idx];
    ts.tx_gain[TX_AUDIO_LINEIN_R] = ts.tx_gain[TX_AUDIO_LINEIN_L];
    // TODO: Right and Left Settings stored

    {
        ulong bias_val;
        bias_val = BIAS_OFFSET + (ts.pa_bias * 2);
        if(bias_val > 255)
            bias_val = 255;

        // Set DAC Channel1 DHR12L register with bias value
        DAC_SetChannel2Data(DAC_Align_8b_R,bias_val);
        //printf("-->PA BIAS loaded: %d\n\r",ts.pa_bias);
    }

    ts.alc_decay_var = ts.alc_decay;

    ts.alc_tx_postfilt_gain_var =  ts.alc_tx_postfilt_gain; // "working" copy of variable

    if(!ts.version_number_release) {			// set xlate to -12KHz at first start
      ts.iq_freq_mode = FREQ_IQ_CONV_MODE_DEFAULT;
    }

    ts.dsp_inhibit = dspmode;       // restore setting
}

// ********************************************************************************************************************
//
//*----------------------------------------------------------------------------
//* Function Name       : UiDriverSaveEepromValues
//* Object              : save all values to EEPROM - called on power-down.  Does not check to see if they have changed
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//

uint16_t UiConfiguration_SaveEepromValues(void)
{
    uint16_t i, retval = 0x0;
    bool dspmode;
    uchar demodmode;

    if(ts.txrx_mode != TRX_MODE_RX) {
      retval = 0xFF00;
    } else {
      // disable DSP during write because it decreases speed tremendous
      dspmode = ts.dsp_inhibit;
      ts.dsp_inhibit = 1;
      //  ts.dsp_active &= 0xfa;  // turn off DSP

      // switch to SSB during write when in FM because it decreases speed tremendous
      demodmode = ts.dmod_mode;
      if(ts.dmod_mode == DEMOD_FM)
        ts.dmod_mode = DEMOD_USB;   // if FM switch to USB during write

      if(ts.ser_eeprom_in_use == 0)
      {
        static uint8_t p[MAX_VAR_ADDR*2+2];
        ts.eeprombuf = p;

        uint16_t i, data;

        ts.eeprombuf[0] = ts.ser_eeprom_type;
        ts.eeprombuf[1] = ts.ser_eeprom_in_use;
        for(i=1; i <= MAX_VAR_ADDR; i++)
        {
          Read_SerEEPROM(i, &data);
          ts.eeprombuf[i*2+1] = (uint8_t)((0x00FF)&data);
          data = data>>8;
          ts.eeprombuf[i*2] = (uint8_t)((0x00FF)&data);
        }
        ts.ser_eeprom_in_use = 0xAA;
      }

      // TODO: move value to a static variable, so that it can be read/written with standard approach
      UiWriteSettingEEPROM_UInt16(EEPROM_BAND_MODE,
          (uint16_t)((uint16_t)ts.band| ((uint16_t)ts.dmod_mode << 8)),
          (uint16_t)((uint16_t)ts.band |((uint16_t)demodmode & 0x0f << 8) ));

      // TODO: move value to a static variable, so that it can be read/written with standard approach
      UiWriteSettingEEPROM_UInt32(EEPROM_FREQ_HIGH,EEPROM_FREQ_LOW, df.tune_new, df.tune_new);

	if(ts.cat_band_index == 255)			// not in a sandbox
	    {
    	    // save current band/frequency/mode settings
    	    vfo[is_vfo_b()?VFO_B:VFO_A].band[ts.band].dial_value = df.tune_new;
    	    // Save decode mode
    	    vfo[is_vfo_b()?VFO_B:VFO_A].band[ts.band].decod_mode = ts.dmod_mode;
	    }

      // Save stored band/mode/frequency memory from RAM
      for(i = 0; i < MAX_BANDS; i++)  {   // scan through each band's frequency/mode data
        // UiWriteSettingsBandMode(i,EEPROM_BAND0_MODE,EEPROM_BAND0_FREQ_HIGH,EEPROM_BAND0_FREQ_LOW,  &vfo[VFO_WORK].band[i]);
        UiWriteSettingsBandMode(i,EEPROM_BAND0_MODE_A,EEPROM_BAND0_FREQ_HIGH_A,EEPROM_BAND0_FREQ_LOW_A, &vfo[VFO_A].band[i]);
        UiWriteSettingsBandMode(i,EEPROM_BAND0_MODE_B,EEPROM_BAND0_FREQ_HIGH_B,EEPROM_BAND0_FREQ_LOW_B, &vfo[VFO_B].band[i]);
      }

      UiConfiguration_WriteConfigEntries();

      UiWriteSettingEEPROM_UInt32(EEPROM_XVERTER_OFFSET_HIGH,EEPROM_XVERTER_OFFSET_LOW,ts.xverter_offset,0);

      UiWriteSettingEEPROM_Filter();

      if(ts.ser_eeprom_in_use == 0xAA) {
        Write_24Cxxseq(0, ts.eeprombuf, MAX_VAR_ADDR*2+2, ts.ser_eeprom_type);
        ts.ser_eeprom_in_use = 0;
      }

      ts.dsp_inhibit = dspmode;   // restore DSP mode
      ts.dmod_mode = demodmode;   // restore active mode
    }
    return retval;
}
