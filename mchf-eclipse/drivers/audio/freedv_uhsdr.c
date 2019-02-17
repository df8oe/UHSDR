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

struct freedv *f_FREEDV;

RingBuffer_DefineExtMem(fdv_demod_rb,int16_t,sizeof(mmb.fdv_demod_buff)/sizeof(int16_t), mmb.fdv_demod_buff)
//RingBuffer_DefineExtMem(fdv_demod_rb,COMP,sizeof(mmb.fdv_demod_buff)/sizeof(COMP), mmb.fdv_demod_buff)
RingBuffer_DefineExtMem(fdv_iq_rb,COMP,sizeof(mmb.fdv_iq_buff)/sizeof(COMP), mmb.fdv_iq_buff)
RingBuffer_Define(fdv_audio_rb,int16_t,((FDV_BUFFER_SIZE*2)+IQ_BLOCK_SIZE))

typedef struct {
    int32_t start;
    int32_t offset;
    int32_t count;
} flex_buffer;

static uint16_t freedv_display_x_offset;

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
    if (SNR<0) SNR=0.0;
    snprintf(SNR_string,12,"%02d",(int)(SNR+0.5));  //Display the current SNR and round it up to the next int
    UiLcdHy28_PrintText(ts.Layout->FREEDV_SNR.x + freedv_display_x_offset, ts.Layout->FREEDV_SNR.y ,SNR_string,Yellow,Black, ts.Layout->FREEDV_FONT);
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
    static bool rx_was_here = false;


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
            RingBuffer_PutSamples(&fdv_iq_rb, &iq_buffer, freedv_get_n_nom_modem_samples(f_FREEDV));

        }
        else if ((ts.txrx_mode == TRX_MODE_RX))
        {

            int16_t    demod_buffer[freedv_get_n_max_modem_samples(f_FREEDV)];
            //COMP    demod_buffer[freedv_get_n_max_modem_samples(f_FREEDV)];

            int16_t audio_buffer[freedv_get_n_max_modem_samples(f_FREEDV)];

            // these buffers are large enough to hold the requested/provided amount of data for freedv_comprx
            // these are larger than the FDV_BUFFER_SIZE since some more bytes may be asked for.

            if (rx_was_here == false)
            {
                RingBuffer_ClearGetTail(&fdv_demod_rb);
                RingBuffer_ClearPutHead(&fdv_audio_rb);

                freedv_set_total_bit_errors(f_FREEDV,0);  //reset ber calculation after coming from TX
                freedv_set_total_bits(f_FREEDV,0);
                FreeDv_DisplayClear();

                rx_was_here = true; // this is used to clear buffers when going into TX
                tx_was_here = false;
            }


            // while makes this highest prio
            // if may give more responsiveness but can cause interrupted reception
            while (RingBuffer_GetData(&fdv_demod_rb) >= freedv_nin(f_FREEDV)
                    && RingBuffer_GetRoom(&fdv_audio_rb) >= freedv_nin(f_FREEDV))
            {
                // MchfBoard_GreenLed(LED_STATE_OFF);
                 // if we arrive here the rx_buffer for comprx is full and will be consumed now.
                // profileTimedEventStart(7);
                RingBuffer_GetSamples(&fdv_demod_rb, &demod_buffer, freedv_nin(f_FREEDV));

                int count = freedv_rx(f_FREEDV, audio_buffer, demod_buffer); // run the decoding process
                //int count = freedv_comprx(f_FREEDV, audio_buffer, demod_buffer); // run the decoding process

                if (freedv_get_sync(f_FREEDV) != 0)
                {
                    RingBuffer_PutSamples(&fdv_audio_rb, &audio_buffer, count);
                }
                // profileTimedEventStop(7);
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


static void my_put_next_rx_char(void *callback_state, char ch) {
    UiDriver_TextMsgPutChar(ch);
}

// FreeDV txt test - will be out of here
void  FreeDV_Init()
{
    // Freedv Test DL2FW

    f_FREEDV = freedv_open(FREEDV_MODE_UHSDR);

    sprintf(my_cb_state.tx_str, ts.special_functions_enabled == 1 ? FREEDV_TX_DF8OE_MESSAGE : FREEDV_TX_MESSAGE);

    my_cb_state.ptx_str = my_cb_state.tx_str;
    freedv_set_callback_txt(f_FREEDV, &my_put_next_rx_char, &my_get_next_tx_char, &my_cb_state);
    // freedv_set_squelch_en(f_FREEDV,0);
    // freedv_set_snr_squelch_thresh(f_FREEDV,-100.0);
    freedv_set_tx_bpf(f_FREEDV, 0);


}

int32_t FreeDV_Iq_Get_FrameLen()
{
    return freedv_get_n_nom_modem_samples(f_FREEDV);
}

int32_t FreeDV_Audio_Get_FrameLen()
{
    return freedv_get_n_speech_samples(f_FREEDV);
}

#endif
