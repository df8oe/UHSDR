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
#include "codec.h"
//
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "mchf_hw_i2c.h"
#include "ui_si570.h"

const uchar 	hs_div[6]	= {11, 9, 7, 6, 5, 4};
const float 	fdco_max 	= FDCO_MAX;
const float 	fdco_min 	= FDCO_MIN;

// unsigned short si570_address;

// All publics as struct, so eventually could be malloc-ed and in CCM for faster access!!
__IO OscillatorState os;

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_setbits
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_setbits(unsigned char original, unsigned char reset_mask, unsigned char new_val)
{
   	return ((original & reset_mask) | new_val);
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_verify_frequency
//* Object              : read regs and verify freq change
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_verify_frequency(void)
{
	uchar 	i,res;
	uchar	regs[10];

	// Read all regs
	for(i = 0; i < 6; i++)
	{
		res = mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + i) ,&regs[i]);
		if(res != 0)
			return(0);
	}

	// Not working - need fix
	//memset(regs,0,6);
	//trx4m_hw_i2c_ReadData(os.si570_address,7,regs,5);

	//printf("-----------------------------------\n\r");
	//printf("write %02x %02x %02x %02x %02x %02x\n\r",os.regs[0],os.regs[1],os.regs[2],os.regs[3],os.regs[4],os.regs[5]);
	//printf("read  %02x %02x %02x %02x %02x %02x\n\r",regs[0],regs[1],regs[2],regs[3],regs[4],regs[5]);

	if(memcmp(regs,os.regs,6) != 0)
		return 1;

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_small_frequency_change
//* Object              : small frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_small_frequency_change(void)
{
	uchar ret,reg_135;
	uchar unfreeze = 0;

	//printf("small\n\r");

	// Read current
	ret = mchf_hw_i2c_ReadRegister(os.si570_address,SI570_REG_135,&reg_135);
	if(ret)
		goto critical;

	// Write to freeze M bit
	ret = mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_135,(reg_135|SI570_FREEZE_M));
	if(ret)
		goto critical;

	// Write as block, registers 7-12
	ret = mchf_hw_i2c_WriteBlock(os.si570_address,SI570_REG_7,os.regs,6);
	if(ret)
	{
		unfreeze = 1;
		goto critical;
	}

	// Verify
	ret = ui_si570_verify_frequency();
	if(ret)
	{
		unfreeze = 1;
		goto critical;
	}

	// Write to unfreeze M
	mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_135,(reg_135 & ~SI570_FREEZE_M));

	return 0;

critical:
	//CriticalError(100);
	if(unfreeze) mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_135,(reg_135 & ~SI570_FREEZE_M));
	return 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_large_frequency_change
//* Object              : large frequency changes handling
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_large_frequency_change(void)
{
	uchar ret,reg_135,reg_137;
	uchar unfreeze = 0;

	//printf("large\n\r");

	// Read the current state of Register 137
	ret = mchf_hw_i2c_ReadRegister(os.si570_address,SI570_REG_137,&reg_137);
	if(ret)
		goto critical;

	// Set the Freeze DCO bit
	ret = mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_137, (reg_137|SI570_FREEZE_DCO));
	if(ret)
		goto critical;

	// Write as block, registers 7-12
	ret = mchf_hw_i2c_WriteBlock(os.si570_address,SI570_REG_7,os.regs,6);
	if(ret)
	{
		unfreeze = 1;
		goto critical;
	}

	// Verify
	ret = ui_si570_verify_frequency();
	if(ret)
	{
		unfreeze = 1;
		goto critical;
	}

	// Clear the Freeze DCO bit
	ret = mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_137,(reg_137 & ~SI570_FREEZE_DCO));
	if(ret)
		goto critical;

	// Read current
	ret = mchf_hw_i2c_ReadRegister(os.si570_address,SI570_REG_135,&reg_135);
	if(ret)
		goto critical;

	// Set the NewFreq bit
	ret = mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_135,(reg_135|SI570_NEW_FREQ));
	if(ret)
		goto critical;

	// Wait for action completed
	reg_135 = SI570_NEW_FREQ;
	while(reg_135 & SI570_NEW_FREQ)
	{
		ret = mchf_hw_i2c_ReadRegister(os.si570_address,SI570_REG_135,&reg_135);
		if(ret)
			goto critical;
	}

	return 0;

critical:
	//CriticalError(101);
	if(unfreeze) mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_137,(reg_137 & ~SI570_FREEZE_DCO));
	return 1;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_is_large_change
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_is_large_change(void)
{
	long double	delta_rfreq;

	if(os.rfreq_old == os.rfreq)
		return 0;

	if(os.rfreq_old < os.rfreq)
		delta_rfreq = os.rfreq - os.rfreq_old;
	else
		delta_rfreq = os.rfreq_old - os.rfreq;

	if((delta_rfreq / os.rfreq_old) <= 0.0035)
		return 0;

	return 1;
}
	
//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_get_configuration
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_get_configuration(void)
{
   	int 	i;
   	uchar 	ret;
   	short	res;

   	ulong 	rfreq_frac;
   	ulong 	rfreq_int;

   	uchar 	hsdiv_curr;
   	uchar 	n1_curr;
   	//uchar	sig[10];

   	// Reset publics
   	os.rfreq_old 	= 0.0;
   	os.fxtal 		= FACTORY_FXTAL;

   	res = mchf_hw_i2c_WriteRegister(os.si570_address,SI570_REG_135,SI570_RECALL);
   	if(res != 0)
   	{
   		//printf("cmd1 err: %d\n\r",res);
   		return 1;
   	}

	ret = SI570_RECALL;
	i = 0;
	while(ret & SI570_RECALL)
	{
		res = mchf_hw_i2c_ReadRegister(os.si570_address,SI570_REG_135,&ret);
		if(res != 0)
		{
			//printf("read1 err: %d\n\r",res);
			return 2;
		}

		i++;
		if(i == 30)
		{
			//printf("read timeout\n\r");
			return 3;
		}
	}

   	for(i = 0; i < 6; i++)
   	{   		
   		res = mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + i) ,&(os.regs[i]));
   		if(res != 0)
   		{
			//printf("read2 err: %d, i = %d\n\r",res,i);
			return 4;
		}
   	}
   	//printf("startup %02x %02x %02x %02x %02x %02x\n\r",os.regs[0],os.regs[1],os.regs[2],os.regs[3],os.regs[4],os.regs[5]);

   	hsdiv_curr = ((os.regs[0] & 0xE0) >> 5) + 4;

#ifdef LOWER_PRECISION
   	os.init_hsdiv = hsdiv_curr;
   	//printf("init hsdiv: %d\n\r",hsdiv_curr);
#endif

   	n1_curr = ((os.regs[0] & 0x1F ) << 2 ) + ((os.regs[1] & 0xC0 ) >> 6 );
   	if(n1_curr == 0)
   		n1_curr = 1;
   	else if((n1_curr & 1) != 0)
   		n1_curr = n1_curr + 1;

#ifdef LOWER_PRECISION
   	os.init_n1 = n1_curr;
   	//printf("init n1: %d\n\r",n1_curr);
#endif

 	rfreq_int =	(os.regs[1] & 0x3F);
  	rfreq_int =	(rfreq_int << 4) + ((os.regs[2] & 0xF0) >> 4);

	rfreq_frac = (os.regs[2] & 0x0F);
  	rfreq_frac = (rfreq_frac << 8) + os.regs[3];
  	rfreq_frac = (rfreq_frac << 8) + os.regs[4];
  	rfreq_frac = (rfreq_frac << 8) + os.regs[5];

#ifdef LOWER_PRECISION
  	os.init_rfreq = (os.regs[1] & 0x3F );
  	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[2] );
  	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[3] );
  	os.init_rfreq = (os.init_rfreq << 8) + (os.regs[4] );
  	os.init_rfreq = (os.init_rfreq << 6) + (os.regs[5] >> 2 );
#endif

  	os.rfreq = rfreq_int + rfreq_frac / POW_2_28;
  	os.fxtal = (os.fout * n1_curr * hsdiv_curr) / os.rfreq;
	
  	// Read signature
  	/*for(i = 0; i < 6; i++)
  	{
   		res = mchf_hw_i2c_ReadRegister(os.si570_address,(i + 13) ,&sig[i]);
   		if(res != 0)
   		{
			printf("read sig err: %d, i = %d\n\r",res,i);
			return 4;
		}
   	}
   	printf("sig %02x %02x %02x %02x %02x %02x\n\r",sig[0],sig[1],sig[2],sig[3],sig[4],sig[5]);*/

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_change_frequency
//* Object              :
//* Input Parameters    : input frequency (float), test: 0 = tune, 1 = calculate, but do not actually tune to see if a large tuning step will occur
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
static uchar ui_si570_change_frequency(float new_freq, uchar test)
{
	uchar 	i,res = 0;
   	ushort 	divider_max,curr_div,whole;

   	float 	curr_n1;
   	float 	n1_tmp;

   	ulong 	frac_bits;

#ifdef LOWER_PRECISION
   	float 	ratio = 0;
   	ulong 	final_rfreq_long;
#endif

   	uchar 	n1;
   	uchar 	hsdiv;

   	divider_max = (ushort)floorf(fdco_max / new_freq);
   	curr_div 	= (ushort)ceilf (fdco_min / new_freq);
   	//printf("%d-%d -> ",curr_div,divider_max);

   	while (curr_div <= divider_max)
	{
    	for(i = 0; i < 6; i++)
      	{
    		hsdiv = hs_div[i];
      		curr_n1 = (float)(curr_div) / (float)(hsdiv);

      		n1_tmp = floorf(curr_n1);
      		n1_tmp = curr_n1 - n1_tmp;

      		if(n1_tmp == 0.0)
        	{
         		n1 = (uchar)curr_n1;
				if((n1 == 1) || ((n1 & 1) == 0))
					goto found;
         	}
      	}

      	curr_div++;
   	}

   	CriticalError(102);

found:

	//printf("(%d) %d %d\n\r",curr_div,n1,hsdiv);

 	// New RFREQ calculation
	os.rfreq = ((long double)new_freq * (long double)(n1 * hsdiv)) / os.fxtal;
   	//printf("%d\n\r",(int)os.rfreq);

   	// Debug print calc freq
   	//printf("%d\n\r",(int)((os.fxtal*os.rfreq)/(n1*hsdiv)));

#ifdef LOWER_PRECISION
   	ratio = new_freq / fout0;
   	ratio = ratio * (((float)n1)/((float)os.init_n1));
   	ratio = ratio * (((float)hsdiv)/((float)os.init_hsdiv));
   	final_rfreq_long = ratio * os.init_rfreq;
#endif

   	for(i = 0; i < 6; i++)
   		os.regs[i] = 0;

   	hsdiv = hsdiv - 4;
   	os.regs[0] = (hsdiv << 5);

   	if(n1 == 1)
     	n1 = 0;
   	else if((n1 & 1) == 0)
      	n1 = n1 - 1;

   	os.regs[0] = ui_si570_setbits(os.regs[0], 0xE0, (n1 >> 2));
   	os.regs[1] = (n1 & 3) << 6;

#ifdef LOWER_PRECISION
   	os.regs[1] = os.regs[1] | (final_rfreq_long >> 30);
   	os.regs[2] = final_rfreq_long >> 22;
   	os.regs[3] = final_rfreq_long >> 14;
   	os.regs[4] = final_rfreq_long >> 6;
   	os.regs[5] = final_rfreq_long << 2;
#endif

#ifdef HIGHER_PRECISION
   	whole = floorf(os.rfreq);
   	frac_bits = floorf((os.rfreq - whole) * POW_2_28);

   	for(i = 5; i >= 3; i--)
   	{
   		os.regs[i] = frac_bits & 0xFF;
   		frac_bits = frac_bits >> 8;
   	}

   	os.regs[2] = ui_si570_setbits(os.regs[2], 0xF0, (frac_bits & 0xF));
   	os.regs[2] = ui_si570_setbits(os.regs[2], 0x0F, (whole & 0xF) << 4);
   	os.regs[1] = ui_si570_setbits(os.regs[1], 0xC0, (whole >> 4) & 0x3F);
#endif

   	//printf("write %02x %02x %02x %02x %02x %02x\n\r",os.regs[0],os.regs[1],os.regs[2],os.regs[3],os.regs[4],os.regs[5]);

   	// check to see if this tuning will result in a "large" tuning step, without setting the frequency
   	if(test)
   		return(ui_si570_is_large_change());

   	//
	if(ui_si570_is_large_change())
	{
		res = ui_si570_large_frequency_change();
	}
	else
	{
		res = ui_si570_small_frequency_change();

		// Maybe large change was needed, let's try it
		if(res)	{
			res = ui_si570_large_frequency_change();
		}
	}

	if(res)
		return res;

	// Verify second time - we might be transmitting, so
	// it is absolutely unacceptable to be on startup
	// SI570 frequency if any I2C error or chip reset occurs!
	res = ui_si570_verify_frequency();
	if(res == 0)
		os.rfreq_old = os.rfreq;

	//if(res)
	//	printf("---- error ----\n\r");

	return res;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_set_frequency
//* Object              :
//* Input Parameters    : freq = 32 bit frequency in HZ, temp_factor = temperature calibration factor ref to 14.000 MHz, test: 0= set freq, 1= calculate, but do not change freq
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_set_frequency(ulong freq,int calib,int temp_factor, uchar test)
{
	long double d;
	float		si_freq, freq_calc, freq_scale, temp_scale, temp;

	freq_scale = (float)freq;		// get frequency
	freq_calc = freq_scale;		// copy frequency
	freq_scale /= 14000000;		// get scaling factor since our calibrations are referenced to 14.000 MHz
	//
	temp = (float)calib;			// get calibration factor
	temp *= (freq_scale);		// scale calibration for operating frequency but double magnitude of calibration factor
	freq_calc -= temp;				// subtract calibration factor
	//
	//
	temp_scale = (float)temp_factor;	// get temperature factor
	temp_scale /= 14000000;		// calculate scaling factor for the temperature correction (referenced to 14.000 MHz)
	//
	freq_calc *= (1 + temp_scale);	// rescale by temperature correction factor
	//
	freq = (ulong)freq_calc;


	// -----------------------------
	// freq = freq + calib*4 + temp_factor;		// This is the old version - it does NOT scale calibration/temperature with operating frequency.
	// -----------------------------

	d = freq;								// convert to float
	d = d / 1000000.0;						// Si570 set value = decimal MHz
	si_freq = d;							// convert to float

	//printf("set si750 freq to: %d\n\r",freq);


	return ui_si570_change_frequency(si_freq, test);
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_init_temp_sensor
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_init_temp_sensor(void)
{
	uchar config,res;

	// Read config reg
	res = mchf_hw_i2c_ReadRegister(MCP_ADDR,MCP_CONFIG,&config);
	if(res != 0)
		return 1;

	//printf("chip conf: %02x\n\r",config);

	// Modify resolution
	config &= ~(3 << MCP_ADC_RES);
	config |= (MCP_ADC_RES_12 << MCP_ADC_RES);

	// Modify power mode
	config &= ~(1 << MCP_SHUTDOWN);
	config |= (MCP_POWER_UP << MCP_SHUTDOWN);

	//printf("moded conf: %02x\n\r",config);

	// Write config reg
	res = mchf_hw_i2c_WriteRegister(MCP_ADDR,MCP_CONFIG,config);
	if(res != 0)
		return 2;

	// Verify
	//res = mchf_hw_i2c_ReadRegister(MCP_ADDR,MCP_CONFIG,&config);
	//if(res != 0)
	//	return;

	//printf("updated conf: %02x\n\r",config);
	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_read_temp
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
uchar ui_si570_read_temp(int *temp)
{
	uchar 	res;
	uchar	data[10];

	if(data == NULL)
		return 1;

	// Read temperature
	res = mchf_hw_i2c_ReadData(MCP_ADDR,MCP_TEMP,data,2);
	if(res != 0)
		return 2;

	//printf("temp reg %02x%02x\n\r",data[0],data[1]);

	// Convert to decimal
	ui_si570_conv_temp(data,temp);

	return 0;
}

//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_conv_temp
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void ui_si570_conv_temp(uchar *temp,int *dtemp)
{
	ushort 	ts;
	int		t = 0,d = 0;

	if(dtemp == NULL)
		return;

	ts = *temp << 8;
	temp++;
	ts |= *temp;

	// Full part
	t = (ts >> 8) & 0xFF;

	// Decimal part
	if(ts & 0x80) d += 5000;
	if(ts & 0x40) d += 2500;
	if(ts & 0x20) d += 1250;
	if(ts & 0x10) d += 625;

	//printf("%i.%dC\n\r",t,d);

	// Return int temperature (sign ok after this ?)
	*dtemp = (t * 10000) + d;
}

//
// by DF8OE
//
//*----------------------------------------------------------------------------
//* Function Name       : ui_si570_calc_startupfrequency
//* Object              :
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
//
void ui_si570_calc_startupfrequency(void)
{
if (os.fout < 5)
	{
	uchar si_regs[7];
	int hs_div;
	int n1;
	float rsfreq;

	// test for hardware address of SI570
	os.si570_address = (0x50 << 1);
	if( mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7) ,&si_regs[0]) != 0)
	    {
	    os.si570_address = (0x55 << 1);
	    mchf_hw_i2c_reset();
	    }

	// read configuration
	mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7) ,&si_regs[0]);
	mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + 1) ,&si_regs[1]);
	mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + 2) ,&si_regs[2]);
	mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + 3) ,&si_regs[3]);
	mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + 4) ,&si_regs[4]);
	mchf_hw_i2c_ReadRegister(os.si570_address,(SI570_REG_7 + 5) ,&si_regs[5]);

	// calculate startup frequency
	rsfreq = (float)((si_regs[5] + (si_regs[4] * 0x100) + (si_regs[3] * 0x10000) + (double)((double)si_regs[2] * (double)0x1000000) + (double)((double)(si_regs[1] & 0x3F) * (double)0x100000000)) / (double)POW_2_28);
	hs_div = (si_regs[0] & 0xE0) / 32 + 4;
	n1 = (si_regs[1] & 0xC0) / 64 + (si_regs[0] & 0x1F) *4 + 1;
	if (n1 %2 != 0 && n1 != 1)
		n1++;
	os.fout = roundf((1142850 * rsfreq) / (hs_div * n1)) / 10000;

	int i;
	// all known startup frequencies
	float suf_table[] = {10,10.356,14.05,14.1,15,16.0915,22.5792,34.285,56.32,63,76.8,100,125,156.25,0};

	// test if startup frequency is known
	for (i = 0; suf_table[i] != 0; i++)
	    {
	    float test = os.fout - suf_table[i];
	    if( test < 0)
		test = -test;
	    if(test < 0.2)
		{
		os.fout = suf_table[i];
		break;
		}
	    }
	}
}
