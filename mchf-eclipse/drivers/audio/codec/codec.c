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

// 218b
// Common
#include "mchf_board.h"
#include "audio_driver.h"

#include <stdio.h>

//#include "mchf_sw_i2c.h"
#include "mchf_hw_i2c2.h"
#include "codec.h"

// Demodulator mode public flag
extern __IO ulong demod_mode;

// Transceiver state public structure
extern __IO TransceiverState ts;

// Public Audio
extern __IO		AudioDriverState	ads;


//*----------------------------------------------------------------------------
//* Function Name       : Codec_Init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uint32_t Codec_Init(uint32_t AudioFreq,ulong word_size)
{
	// Configure the Codec related IOs
	Codec_GPIO_Init();   

	// Configure the I2S peripheral
	Codec_AudioInterface_Init(AudioFreq);

	// Reset the Codec Registers
	Codec_Reset(AudioFreq,word_size);

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_Reset
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_Reset(uint32_t AudioFreq,ulong word_size)
{
	//printf("codec init, freq = %d\n\r",AudioFreq);

	// Reset register
	if(Codec_WriteRegister(W8731_RESET, 0) != 0)
		return;

	// Reg 00: Left Line In (0dB, mute off)
	Codec_WriteRegister(W8731_LEFT_LINE_IN,0x001F);

	// Reg 01: Right Line In (0dB, mute off)
	Codec_WriteRegister(W8731_RIGHT_LINE_IN,0x001F);

	// Reg 02: Left Headphone out (0dB)
	//Codec_WriteRegister(0x02,0x0079);
	// Reg 03: Right Headphone out (0dB)
	//Codec_WriteRegister(0x03,0x0079);

	Codec_Volume(0);

	// Reg 04: Analog Audio Path Control (DAC sel, ADC line, Mute Mic)
	Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0012);

	// Reg 05: Digital Audio Path Control(all filters disabled)
	// De-emphasis control, bx11x - 48kHz
	//                      bx00x - off
	// DAC soft mute		b1xxx - mute on
	//						b0xxx - mute off
	//
	Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,W8731_DEEMPH_CNTR);

	// Reg 06: Power Down Control (Clk off, Osc off, Mic Off)
	Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0062);

	// Reg 07: Digital Audio Interface Format (i2s, 16/32 bit, slave)
	if(word_size == WORD_SIZE_16)
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x0002);
	else
		Codec_WriteRegister(W8731_DIGI_AU_INTF_FORMAT,0x000E);

	// Reg 08: Sampling Control (Normal, 256x, 48k ADC/DAC)
	// master clock: 12.5 Mhz
	if(AudioFreq == I2S_AudioFreq_48k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0000);
	if(AudioFreq == I2S_AudioFreq_32k) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x0018);
	if(AudioFreq == I2S_AudioFreq_8k ) Codec_WriteRegister(W8731_SAMPLING_CNTR,0x000C);

	// Reg 09: Active Control
	Codec_WriteRegister(W8731_ACTIVE_CNTR,0x0001);
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_RX_TX
//* Object              : switch codec mode
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

void Codec_RX_TX(void)
{

	uchar mute_count;

	if(ts.txrx_mode == TRX_MODE_RX)
	{
		// First step - mute sound
		Codec_Volume(0);

		// Mute line input
		Codec_Line_Gain_Adj(0);

		// Reg 04: Analog Audio Path Control (DAC sel, ADC line, Mute Mic)
		Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0012);

		// Reg 06: Power Down Control (Clk off, Osc off, Mic Off if LINE IN, Mic On if MIC IN)
		//
		// COMMENT:  It would be tempting to set bit 1 "MICPD" of "W8731_POWER_DOWN_CTR" to zero to disable mic power down
		// and maintain microphone bias during receive, but this seems to cause problems on receive (e.g. deafness) even
		// if the microphone is muted and "mic boost" is disabled.  (KA7OEI 20151030)
		//
		Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0062);	// turn off mic bias, etc.
		//
		// --------------------------------------------------------------
		// Test - route mic to headphones
		// Reg 04: Analog Audio Path Control (DAC sel, ADC Mic, Mic on)
		//Codec_WriteRegister(0x04,0x0014);
		// Reg 06: Power Down Control (Clk off, Osc off, Mic On)
		//Codec_WriteRegister(0x06,0x0061);
		// --------------------------------------------------------------
		//
		ts.audio_unmute = 1;

	}
	else		// It is transmit
	{
		Codec_Volume(0);	// Mute sound
		//
		ads.agc_holder = ads.agc_val;		// store AGC value at instant we went to TX for recovery when we return to RX
		//
		if((ts.dmod_mode == DEMOD_CW) || ((ts.dmod_mode == DEMOD_CW) && ts.tune))	{	// Turn sidetone on for CW or TUNE mode in CW mode
			//
			Codec_SidetoneSetgain();	// set sidetone level
			//
		}
		else if(ts.tune)	{	// Not in CW mode - but in TUNE mode
			if(!ts.iq_freq_mode)	// Is translate mode *NOT* active?
				Codec_SidetoneSetgain();	// yes, turn on sidetone in SSB-TUNE mode
		}
		else	{	// Not CW or TUNE mode
			//
			for(mute_count = 0; mute_count < 8; mute_count++)		// Doing this seems to suppress the loud CLICK
				Codec_Volume(0);	// that occurs when going from RX to TX in modes other than CW
				// This is probably because of the delay between the mute command, above, and the
			//
			non_os_delay();
			//
			// Select source or leave it as it is
			// PHONE out is muted, normal exit routed to TX modulator
			// input audio is routed via 4066 switch
			if(ts.tx_audio_source == TX_AUDIO_MIC)
			{
				// Set up microphone gain and adjust mic boost accordingly
				if(ts.tx_mic_gain > 50)	{
					ts.mic_boost = 1;
					ts.tx_mic_gain_mult = (ts.tx_mic_gain - 35)/3;
				}
				else	{
					ts.mic_boost = 0;
					ts.tx_mic_gain_mult = ts.tx_mic_gain;
				}
				//
				if(!ts.mic_boost)
					Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0014);	// mic boost off
				else
					Codec_WriteRegister(W8731_ANLG_AU_PATH_CNTR,0x0015);	// mic boost on
				//
				// Reg 04: Analog Audio Path Control (DAC sel, ADC Mic, Mic on)
				// Reg 06: Power Down Control (Clk off, Osc off, Mic On)
				if(ts.mic_bias)
				    Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0061);	// turn on mic bias
				else
				    Codec_WriteRegister(W8731_POWER_DOWN_CNTR,0x0062);	// turn off mic bias
			}
			else
				Codec_Line_Gain_Adj(ts.tx_line_gain);	// set LINE input gain if in LINE in mode
			//
		}
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_SidetoneSetgain
//* Object              : calculates and sets sidetone gain based on tx power factor
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_SidetoneSetgain(void)
{
float vcalc, vcalc1;

// This calculates the relative level of the sidetone and sets the headphone gain appropriately
// to keep the sidetone level more or less the same.
// This seems to be slightly "off", particularly at the extremes of high and low
// transmit power levels - this needs to be looked into...
//
// Note that this function is called from places OTHER than Codec_RX_TX(), above!

	// bail out if not in transmit mode
	if(ts.txrx_mode != TRX_MODE_TX)		// bail out if not in transmit mode
		return;
	//
	if(ts.st_gain)	{	// calculate if the sidetone gain is non-zero
		vcalc = (float)ts.tx_power_factor;	// get TX scaling power factor
		vcalc *= vcalc;
		vcalc = 1/vcalc;		// invert it since we are calculating attenuation of the original signal (assuming normalization to 1.0)
		vcalc = log10f(vcalc);	// get the log
		vcalc *= 10;			// convert to deciBels and calibrate for the per-step value of the codec
		vcalc1 = (float)ts.st_gain;		// get the sidetone gain (level) setting
		vcalc1 *= 6;			// offset by # of dB the desired sidetone gain
		vcalc += vcalc1;		// add the calculated gain to the desired sidetone gain
		if(vcalc > 127)			// enforce limits of calculation to range of attenuator
			vcalc = 127;
		else if	(vcalc < 0)
			vcalc = 0;
	}
	else						// mute if zero value
		vcalc = 0;
	//
	Codec_Volume((uchar)vcalc);		// set the calculated sidetone volume
	//
}



//*----------------------------------------------------------------------------
//* Function Name       : Codec_Volume
//* Object              : audio vol control in RX mode
//* Object              : input: 0 - 80
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------

void Codec_Volume(uchar vol)
{
//	ts.codec_vol = vol;		// copy codec volume for global use
	ulong lv = vol;

//	if(vol == 0)
//		ts.codec_was_muted = 1;

	lv += 0x2F;

	if(lv < 0x2F) lv = 0x2F;	// limit min value
	if(lv > 0x7F) lv = 0x7F; 	// limit max value

	//printf("codec reg: 0x%02x\n\r",lv);

	// Reg 03: LINE OUT - const level
//	Codec_WriteRegister(W8731_RIGHT_HEADPH_OUT,0x65);

	//
	// Selectively mute "Right Headphone" output (LINE OUT) depending on transceiver configuration
	//
	if(ts.txrx_mode == TRX_MODE_TX)	{	// in transmit mode?
		if((ts.iq_freq_mode) || ((!ts.iq_freq_mode) && (ts.misc_flags1 & 4)))	// is translate mode active OR translate mode OFF but LINE OUT to be muted during transmit
			Codec_WriteRegister(W8731_RIGHT_HEADPH_OUT,0);	// yes - mute LINE OUT during transmit
		else							// audio is NOT to be muted during transmit
			Codec_WriteRegister(W8731_RIGHT_HEADPH_OUT,0x78);	// value selected for 0.5VRMS at AGC setting
	}
	else	// receive mode - LINE OUT always enabled
		Codec_WriteRegister(W8731_RIGHT_HEADPH_OUT,0x78);	// value selected for 0.5VRMS at AGC setting

	// Reg 02: Speaker - variable volume
	Codec_WriteRegister(W8731_LEFT_HEADPH_OUT,lv);
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_Mute
//* Object              : new method of mute via soft mute of the DAC
//* Object              :
//* Input Parameters    : 0 = Unmuted  1 = Muted
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_Mute(uchar state)
{
//	ts.codec_mute_state = state;
	//
	// Reg 05: Digital Audio Path Control(all filters disabled)
	// De-emphasis control, bx11x - 48kHz
	//                      bx00x - off
	// DAC soft mute		b1xxx - mute on
	//						b0xxx - mute off
	//
	if(state)	{
		Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,(W8731_DEEMPH_CNTR|0x08));	// mute
//		ts.codec_was_muted = 1;
	}
	else
		Codec_WriteRegister(W8731_DIGI_AU_PATH_CNTR,(W8731_DEEMPH_CNTR));		// mute off
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_WriteRegister
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint16_t RegisterValue)
{
	uchar 	res;
	//ushort 	msg;

	// Assemble 2-byte data in WM8731 format
	uint8_t Byte1 = ((RegisterAddr<<1)&0xFE) | ((RegisterValue>>8)&0x01);
	uint8_t Byte2 = RegisterValue&0xFF;
	
	// Combine spi msg
	//msg = (Byte1 << 8) | Byte2;

	//printf("codec write, reg = %02x,val = %02x\n\r",Byte1,Byte2);

	res = mchf_hw_i2c2_WriteRegister(CODEC_ADDRESS,Byte1,Byte2);
	if(res)
	{
#ifdef DEBUG_BUILD
		printf("err codec i2c: %d\n\r",res);
#endif
	}

	return res;
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_AudioInterface_Init
//* Object              : init I2S
//* Object              : I2S PLL already enabled in startup file!
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
	I2S_InitTypeDef I2S_InitStructure;

	// Enable the CODEC_I2S peripheral clock
	RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);

	// CODEC_I2S peripheral configuration for master TX
	SPI_I2S_DeInit(CODEC_I2S);
	I2S_InitStructure.I2S_AudioFreq = AudioFreq;
	I2S_InitStructure.I2S_Standard = I2S_STANDARD;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
	I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
	I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;	// using MCO2

	// Initialise the I2S main channel for TX
	I2S_Init(CODEC_I2S, &I2S_InitStructure);
	
	// Initialise the I2S extended channel for RX
	I2S_FullDuplexConfig(CODEC_I2S_EXT, &I2S_InitStructure);
}

//*----------------------------------------------------------------------------
//* Function Name       : Codec_GPIO_Init
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// CODEC_I2S output pins configuration: WS, SCK SD0 and SDI pins
	GPIO_InitStructure.GPIO_Pin 	= CODEC_I2S_SCK | CODEC_I2S_SDO | CODEC_I2S_SDI;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(CODEC_I2S_SDO_PIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(CODEC_I2S_WS_PIO, &GPIO_InitStructure);

	// Configure MCO2 (PC9)
	GPIO_InitStructure.GPIO_Pin = CODEC_CLOCK;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(CODEC_CLOCK_PIO, &GPIO_InitStructure);

	// Output I2S PLL via MCO2 pin - 12.288 Mhz
	RCC_MCO2Config(RCC_MCO2Source_PLLI2SCLK, RCC_MCO2Div_3);

	// Connect pins to I2S peripheral
	GPIO_PinAFConfig(CODEC_I2S_WS_PIO,	CODEC_I2S_WS_SOURCE,  CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_SDO_PIO, CODEC_I2S_SCK_SOURCE, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_SDO_PIO,	CODEC_I2S_SDO_SOURCE, CODEC_I2S_GPIO_AF);
	GPIO_PinAFConfig(CODEC_I2S_SDO_PIO, CODEC_I2S_SDI_SOURCE, CODEC_I2S_GPIO_AF);
}
//
//*----------------------------------------------------------------------------
//* Function Name       : Codec_Line_Gain_Adj
//* Object              :
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void Codec_Line_Gain_Adj(uchar gain)
{
	uint16_t l_gain;
	//printf("codec line gain adjust = %d\n\r",gain);

	l_gain = (uint16_t)gain;
	//
	// Use Reg 00: Left Line In, set MSB to adjust gain of both channels simultaneously
	//
	l_gain |= 0x100;	// set MSB of control word for "LRINBOTH" flag
	//
	Codec_WriteRegister(W8731_LEFT_LINE_IN,l_gain);


}
