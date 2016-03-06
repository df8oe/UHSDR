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

#ifndef DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_
#define DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_

void    AudioManagement_CalcRxIqGainAdj(void);
void    AudioManagement_CalcTxIqGainAdj(void);
void    AudioManagement_CalcRxIqGainAdj(void);
void    AudioManagement_CalcTxIqGainAdj(void);

void    AudioManagement_CalcTxCompLevel(void);
void    AudioManagement_CalcNB_AGC(void);
void    AudioManagement_CalcAGCVals(void);
void    AudioManagement_CalcRFGain(void);
void    AudioManagement_CalcALCDecay(void);
void    AudioManagement_CalcAGCDecay(void);

void    AudioManagement_LoadToneBurstMode(void);
void    AudioManagement_CalcSubaudibleGenFreq(void);        // load/set current FM subaudible tone settings for generation
void    AudioManagement_CalcSubaudibleDetFreq();
void    AudioManagement_KeyBeep(void);
void    AudioManagement_LoadBeepFreq(void);



#endif /* DRIVERS_AUDIO_AUDIO_MANAGEMENT_H_ */
