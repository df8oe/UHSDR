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
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/
#include "ui_driver.h"
#include "freedv_uhsdr.h"
#include "ui_lcd_layouts.h"
#include "profiling.h"
#include "ui_lcd_hy28.h"
#include "radio_management.h"

#if defined(USE_FREEDV) || defined(USE_ALTERNATE_NR)

MultiModeBuffer_t __MCHF_SPECIALMEM mmb;
#endif

#ifdef USE_FREEDV

#include "freedv_api.h"

freedv_conf_t freedv_conf;

struct freedv *f_FREEDV;


RingBuffer_DefineExtMem(fdv_demod_rb,sizeof(mmb.fdv_demod_buff)/sizeof(fdv_demod_rb_item_t), mmb.fdv_demod_buff)
RingBuffer_DefineExtMem(fdv_iq_rb,sizeof(mmb.fdv_iq_buff)/sizeof(fdv_iq_rb_item_t), mmb.fdv_iq_buff)

#define FDV_AUDIO_MEM_SIZE ((FDV_BUFFER_SIZE*2)+IQ_BLOCK_SIZE)
__MCHF_SPECIALMEM fdv_audio_rb_item_t fdv_audio_rb_mem[FDV_AUDIO_MEM_SIZE];
RingBuffer_DefineExtMem(fdv_audio_rb, FDV_AUDIO_MEM_SIZE, fdv_audio_rb_mem)

typedef struct {
    int32_t start;
    int32_t offset;
    int32_t count;
} flex_buffer;

static uint16_t freedv_display_x_offset;

/**
 * Returns the internal UHSDR  configuration value in the value range for display and use with the freedv_api
 * @param freedv_conf_p
 * @return SNR in the range of -100 to 99
 */

int32_t FreeDV_Get_Squelch_SNR(freedv_conf_t* freedv_conf_p)
{
    return (int32_t)(freedv_conf_p->squelch_snr_thresh) + FDV_SQUELCH_OFFSET;
}

/**
 * Sets the internal UHSDR  configuration value in the permitted value range
 * @param freedv_conf_p
 * @param squelch_snr  SNR in the range of -100 to 99 , -100 represents OFF
 */
void FreeDV_Set_Squelch_SNR(freedv_conf_t* freedv_conf_p, int8_t squelch_snr)
{
    int32_t internal_snr = squelch_snr - FDV_SQUELCH_OFFSET;
    if (internal_snr > FDV_SQUELCH_MAX)
    {
        internal_snr = FDV_SQUELCH_MAX;
    }
    else if (internal_snr < FDV_SQUELCH_OFF)
    {
        internal_snr = FDV_SQUELCH_OFF;
    }

    freedv_conf_p->squelch_snr_thresh = internal_snr;
}

int FreeDV_Is_Squelch_Enable(freedv_conf_t* freedv_conf_p)
{
    return freedv_conf_p->squelch_snr_thresh != FDV_SQUELCH_OFF;
}

void FreeDV_Squelch_Update(freedv_conf_t* freedv_conf_p)
{
    UNUSED(freedv_conf_p);
    freedv_set_squelch_en(f_FREEDV, FreeDV_Is_Squelch_Enable(&freedv_conf));
    freedv_set_snr_squelch_thresh(f_FREEDV, FreeDV_Get_Squelch_SNR(&freedv_conf));
}

static void FreeDv_DisplayBer()
{
    int ber = 0;
    char ber_string[12];



    ber = 1000*freedv_get_total_bit_errors(f_FREEDV)/freedv_get_total_bits(f_FREEDV);
    snprintf(ber_string,12,"0.%03d",ber);  //calculate and display the bit error rate
    UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x + freedv_display_x_offset,ts.Layout->FREEDV_BER.y, ber_string,Yellow,Black, ts.Layout->FREEDV_FONT);

}

static void FreeDv_DisplaySnr()
{
    static float SNR = 1;
    float SNR_est;
    char SNR_string[12];
    int sync;

    freedv_get_modem_stats(f_FREEDV,&sync,&SNR_est);

    SNR = 0.95*SNR + 0.05 * SNR_est; //some averaging to keep it more calm
    int32_t SNR_Int = roundf(SNR);

    if (SNR_Int > 99)
    {
        SNR_Int = 99;
    }
    else if (SNR_Int < -99)
    {
        SNR_Int = -99;
    }

    uint32_t clr_fg;
    if (FreeDV_Is_Squelch_Enable(&freedv_conf))
    {
        clr_fg = SNR_Int >= FreeDV_Get_Squelch_SNR(&freedv_conf)? Green:Red;
    }
    else
    {
        clr_fg = Yellow;
    }

    snprintf(SNR_string,12,"%-2ld",SNR_Int);  //Display the current SNR and round it up to the next int
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x + freedv_display_x_offset, ts.Layout->FREEDV_SNR.y ,SNR_string,clr_fg,Black, ts.Layout->FREEDV_FONT);
}

void FreeDv_DisplayClear()
{
//    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x, ts.Layout->FREEDV_SNR.y,"            ",Yellow,Black,ts.Layout->FREEDV_FONT);
//   UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x, ts.Layout->FREEDV_BER.y,"            ",Yellow,Black,ts.Layout->FREEDV_FONT);
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x, ts.Layout->FREEDV_SNR.y,"         ",Yellow,Black,ts.Layout->FREEDV_FONT);		//SNR=00
    UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x, ts.Layout->FREEDV_BER.y,"         ",Yellow,Black,ts.Layout->FREEDV_FONT);		//BER=0.000	(max 9 chars)
    UiDriver_TextMsgClear();
}

void FreeDv_DisplayPrepare()
{
	UiDriver_TextMsgClear();
	freedv_display_x_offset = UiLcdHy28_TextWidth("SNR=", ts.Layout->FREEDV_FONT);
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x, ts.Layout->FREEDV_SNR.y,"SNR=",Yellow,Black, ts.Layout->FREEDV_FONT);
    UiLcdHy28_PrintText(ts.Layout->FREEDV_BER.x, ts.Layout->FREEDV_BER.y,"BER=",Yellow,Black, ts.Layout->FREEDV_FONT);
}

void FreeDv_DisplayUpdate()
{
    FreeDv_DisplayBer();
    FreeDv_DisplaySnr();
    UiDriver_TextMsgDisplay();
}


void FreeDv_HandleFreeDv()
{

    // Freedv DL2FW
    // static FDV_Out_Buffer FDV_TX_out_im_buff;
    static bool tx_was_here = false;
    static bool rx_was_here = true;


     // we are always called, so check if there is FreeDV active
    if (ts.digital_mode == DigitalMode_FreeDV)
    {
        if ((ts.txrx_mode == TRX_MODE_TX)
                && RingBuffer_GetData(&fdv_audio_rb) >= freedv_get_n_speech_samples(f_FREEDV)
                && RingBuffer_GetRoom(&fdv_iq_rb) >= freedv_get_n_nom_modem_samples(f_FREEDV))
        {
            // ...and if we are transmitting and samples from dv_tx_processor are ready
            if (tx_was_here == false)
            {
                RingBuffer_ClearGetTail(&fdv_audio_rb);
                RingBuffer_ClearPutHead(&fdv_iq_rb);
                tx_was_here = true;
                rx_was_here = false;
            }
            COMP    iq_buffer[freedv_get_n_nom_modem_samples(f_FREEDV)];
            int16_t audio_buffer[freedv_get_n_speech_samples(f_FREEDV)];

            RingBuffer_GetSamples(&fdv_audio_rb, &audio_buffer, freedv_get_n_speech_samples(f_FREEDV));

            profileTimedEventStart(7);
            freedv_comptx(f_FREEDV,
                    iq_buffer,
                    audio_buffer); // start the encoding process
            profileTimedEventStop(7);

            for (int idx = 0; idx < freedv_get_n_nom_modem_samples(f_FREEDV); idx++)
            {
                fdv_iq_rb_item_t sample;
                sample.real = iq_buffer[idx].real;
                sample.imag = iq_buffer[idx].imag;

                RingBuffer_PutSamples(&fdv_iq_rb, &sample, 1);
            }

        }
        else if ((ts.txrx_mode == TRX_MODE_RX))
        {


            if (rx_was_here == false)
            {
                RingBuffer_ClearGetTail(&fdv_demod_rb);
                RingBuffer_ClearPutHead(&fdv_audio_rb);

                freedv_set_total_bit_errors(f_FREEDV,0);  //reset ber calculation after coming from TX
                freedv_set_total_bits(f_FREEDV,0);

                rx_was_here = true; // this is used to clear buffers when going into TX
                tx_was_here = false;
            }

            // these buffers are large enough to hold the requested/provided amount of data for freedv_comprx
            // these are larger than the FDV_BUFFER_SIZE since some more bytes may be asked for.
#ifdef USE_SIMPLE_FREEDV_FILTERS
            #define     input_rb fdv_iq_rb
            #define     input_rb_item_t fdv_iq_rb_item_t
            #define     FREEDV_RX_FUNC freedv_comprx
            #define     FREEDV_RX_INPUT_T COMP
#else
            #define     input_rb fdv_demod_rb
            #define     input_rb_item_t fdv_demod_rb_item_t
            #define     FREEDV_RX_FUNC freedv_rx
            #define     FREEDV_RX_INPUT_T int16_t
#endif
            FREEDV_RX_INPUT_T input_buffer[freedv_get_n_max_modem_samples(f_FREEDV)];
            int16_t audio_buffer[freedv_get_n_max_modem_samples(f_FREEDV)];

            // while makes this highest prio
            // if may give more responsiveness but can cause interrupted reception
            while (RingBuffer_GetData(&input_rb) >= freedv_nin(f_FREEDV)
                    && RingBuffer_GetRoom(&fdv_audio_rb) >= freedv_nin(f_FREEDV))
            {
                // MchfBoard_GreenLed(LED_STATE_OFF);
                 // if we arrive here the rx_buffer is full enough and will be consumed now.
#ifdef USE_SIMPLE_FREEDV_FILTERS
                for (int idx = 0; idx < freedv_nin(f_FREEDV); idx++ )
                {
                    input_rb_item_t sample;
                    RingBuffer_GetSamples(&input_rb, &sample, 1);
                    input_buffer[idx].real = sample.real;
                    input_buffer[idx].imag = sample.imag;
                }
#else
                RingBuffer_GetSamples(&input_rb, &input_buffer, freedv_nin(f_FREEDV));
#endif

                int count = FREEDV_RX_FUNC(f_FREEDV, audio_buffer, input_buffer); // run the decoding process

                if (freedv_get_sync(f_FREEDV) != 0)
                {
                    RingBuffer_PutSamples(&fdv_audio_rb, &audio_buffer, count);
                }
            }
        }
    }
    else
    {
        if (tx_was_here == true || rx_was_here == true)
        {
            tx_was_here = false;
            rx_was_here = false;
            RingBuffer_ClearGetTail(&fdv_iq_rb);
            RingBuffer_ClearGetTail(&fdv_audio_rb);
        }
    }
}


// FreeDV txt test - will be out of here
typedef struct {
    char  tx_str[80];
    char *ptx_str;
} my_callback_state_t;

static my_callback_state_t  my_cb_state;

static char my_get_next_tx_char(void *callback_state) {
    my_callback_state_t* pstate = (my_callback_state_t*)callback_state;

    char  c = *pstate->ptx_str++;

    if (*pstate->ptx_str == 0) {
        pstate->ptx_str = pstate->tx_str;
    }

    return c;
}


static void my_put_next_rx_char(void *callback_state, char ch)
{
    UNUSED(callback_state);
    UiDriver_TextMsgPutChar(ch);
}

freedv_mode_desc_t freedv_modes[] =
{
        { "1600", "FD1600", FREEDV_MODE_1600 },
#ifdef USE_FREEDV_700D
        { "700D", "FD700D", FREEDV_MODE_700D },
#endif
};

const uint8_t freedv_modes_num = sizeof(freedv_modes)/sizeof(freedv_modes[0]);


// FreeDV txt test - will be out of here
void  FreeDV_Init()
{
    // Freedv Test DL2FW

     // move this to ui_configuration;
     FreeDV_Set_Squelch_SNR(&freedv_conf,-2);
     freedv_conf.mode = 0; // 0 = 1600, 1 = 700D

     FreeDV_SetMode(freedv_conf.mode, true);

}

/**
 *
 * @return true if mode was activated, false if old mode is still active
 */
int32_t FreeDV_SetMode(uint8_t fdv_mode, int32_t firstTime)
{
    bool retval = true;

    // turn off rx audio processing to be able to disable and reenable FreeDV processing
    ads.af_disabled++;

    // normal init starts
    if (fdv_mode >= freedv_modes_num)
    {
        fdv_mode = 0;
    }

    // we don't switch if TX is currently active
    if (firstTime == false)
    {
        retval = (ts.txrx_mode == TRX_MODE_RX);
        if (retval == true && f_FREEDV != NULL && freedv_conf.mode != fdv_mode)
        {
            freedv_close(f_FREEDV);
            f_FREEDV = NULL;
        }
    }

    // only if all went well AND we have not yet initialized the mode,
    // we are going to run the mode, so if same mode is being set, nothing happens
    // really
    if (retval && f_FREEDV == NULL)
    {
        f_FREEDV = freedv_open(freedv_modes[fdv_mode].freedv_id);

        retval = f_FREEDV != NULL;
        if (retval)
        {
            sprintf(my_cb_state.tx_str, ts.special_functions_enabled == 1 ? FREEDV_TX_DF8OE_MESSAGE : FREEDV_TX_MESSAGE);

            my_cb_state.ptx_str = my_cb_state.tx_str;
            freedv_set_callback_txt(f_FREEDV, &my_put_next_rx_char, &my_get_next_tx_char, &my_cb_state);

            if (FreeDV_Is_Squelch_Enable(&freedv_conf))
            {
                freedv_set_squelch_en(f_FREEDV,1);
                freedv_set_snr_squelch_thresh(f_FREEDV, FreeDV_Get_Squelch_SNR(&freedv_conf));
            }
            else
            {
                freedv_set_squelch_en(f_FREEDV,0);
            }

            freedv_set_tx_bpf(f_FREEDV, 0);
        }
    }

    if (retval)
    {
        freedv_conf.mode = fdv_mode;
    }

    // turn on rx audio processing, FreeDV is ready for work
    ads.af_disabled--;

    return retval;
}
int32_t FreeDV_Iq_Get_FrameLen()
{
    return freedv_get_n_nom_modem_samples(f_FREEDV);
}

int32_t FreeDV_Audio_Get_FrameLen()
{
    return freedv_get_n_speech_samples(f_FREEDV);
}


#ifdef DEBUG_FREEDV
void FreeDV_Test()
{
    ts.digital_mode = DigitalMode_FreeDV;
    ts.txrx_mode = TRX_MODE_RX;
    while (1)
    {
        for (int idx = 0; idx < FREEDV_TEST_BUFFER_FRAME_SIZE*FREEDV_TEST_BUFFER_FRAME_COUNT; idx++)
        {
            int16_t sample[2];
            sample[0] = test_buffer[idx].real;
            sample[1] = test_buffer[idx].imag;

            RingBuffer_PutSamples(&fdv_iq_rb,&sample,2);
            FreeDv_HandleFreeDv();
            RingBuffer_GetSamples(&fdv_audio_rb,sample,1);
        }
    }

}
#endif

#endif
