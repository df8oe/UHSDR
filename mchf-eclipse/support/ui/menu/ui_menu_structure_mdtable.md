
[//]: # (                                                                              )
[//]: # ( WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !                         )
[//]: # (                                                                              )
[//]: # ( generated at  2017-03-18T07:47:43  by "./ui_menu_structure_mdtable.py" )
[//]: # (                                                                              )
[//]: # ( mcHF SDR TRX v2.0.0 - Menu Structure Diagram as MarkDown-Table )
[//]: # (                                                                              )
[//]: # ( see <https://help.github.com/categories/writing-on-github/>                  )
[//]: # (                                                                              )

# mcHF FW v2.0.0 - UI Menu Overview

generated at  2017-03-18T07:47:43  by "./ui_menu_structure_mdtable.py"



## Standard Menu (`MENU_BASE`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **LSB/USB Auto Select**       (                  MENU_SSB_AUTO_MODE_SELECT) | If enabled, the appropriate sideband mode for SSB and FreeDV is chosen as default for each band by its frequency. | 
| **Digital Modes**             (                          MENU_DIGI_DISABLE) | Disable appearance of digital modes when pressing Mode button | 
| **CW Mode**                   (                            MENU_CW_DISABLE) | Disable appearance of CW mode when pressing Mode button | 
| **AM Mode**                   (                            MENU_AM_DISABLE) | Disable appearance of AM mode when pressing Mode button | 
| **SyncAM Mode**               (                             MENU_DEMOD_SAM) | Disable appearance of SyncAM modes when pressing Mode button | 
| **SAM PLL locking range**     (                 MENU_SAM_PLL_LOCKING_RANGE) | SAM PLL Locking Range in Hz: this determines how far up and down from the carrier frequency of an AM station we can offtune the receiver, so that the PLL will still lock to the carrier. | 
| **SAM PLL step response**     (                 MENU_SAM_PLL_STEP_RESPONSE) | Step response = Zeta = damping factor of the SAM PLL. Sets the stability and transient response of the PLL. Larger values give faster lock even if you are offtune, but PLL is also more sensitive. | 
| **SAM PLL bandwidth in Hz**   (                     MENU_SAM_PLL_BANDWIDTH) | Bandwidth of the PLL loop = OmegaN in Hz: smaller bandwidth = more stable lock. FAST LOCK SAM PLL - set Step response and PLL bandwidth to large values [eg. 80 / 350]; DX (SLOW & STABLE) SAM PLL - set Step response and PLL bandwidth to small values [eg. 30 / 100]. | 
| **SAM Fade Leveler**          (                      MENU_SAM_FADE_LEVELER) | Fade leveler (in AM/SAM mode) ON/OFF. Fade leveler is helpful in situations with very fast QSB of the carrier ´flutter´. It is designed to remove the rapidly changing carrier and replace it with a more stable carrier. If there is no QSB on the carrier, there is no change. | 
| **FM Mode**                   (                        MENU_FM_MODE_ENABLE) | Disable appearance of FM mode when pressing Mode button | 
| **FM Sub Tone Gen**           (                MENU_FM_GEN_SUBAUDIBLE_TONE) | Enable generation of CTCSS tones during FM transmissions. | 
| **FM Sub Tone Det**           (                MENU_FM_DET_SUBAUDIBLE_TONE) | Enable detection of CTCSS tones during FM receive. RX is muted unless tone is detected. | 
| **FM Tone Burst**             (                    MENU_FM_TONE_BURST_MODE) | Enabled sending of short tone at beginning of each FM transmission. Used to open repeaters. Available frequencies are 1750 Hz and 2135 Hz. | 
| **FM Deviation**              (                           MENU_FM_DEV_MODE) | Select between normal and narrow deviation (5 and 2.5kHz) for FM RX/TX | 
| **RF Gain**                   (                           MENU_RF_GAIN_ADJ) | RF Receive Gain. This setting is also accessible via Encoder 2, RFG. | 
| **AGC Mode**                  (                              MENU_AGC_MODE) | Automatic Gain Control Mode setting. You may select preconfigured settings (SLOW,MED,FAST), define settings yourself (CUSTOM) or use MANUAL (no AGC, use RFG to control gain | 
| **Custom AGC (+=Slower)**     (                            MENU_CUSTOM_AGC) | If AGC is set to CUSTOM, this controls the speed setting of AGC | 
| **AGC WDSP switch**           (                       MENU_AGC_WDSP_SWITCH) | You can choose between two different AGC systems here: ´Standard AGC´ and ´WDSP AGC´. | 
| **AGC WDSP Mode**             (                         MENU_AGC_WDSP_MODE) | Choose a bundle of preset AGC parameters for the WDSP AGC: FAST / MED / SLOW / LONG / very LONG or switch OFF the AGC. | 
| **AGC WDSP Slope**            (                        MENU_AGC_WDSP_SLOPE) | Slope of the AGC is the difference between the loudest signal and the quietest signal after the AGC action has taken place. Given in dB. | 
| **AGC WDSP Decay**            (                    MENU_AGC_WDSP_TAU_DECAY) | Time constant for the AGC decay (speed of recovery of the AGC gain) in milliseconds. | 
| **AGC Threshold**             (                       MENU_AGC_WDSP_THRESH) | ´Threshold´ = ´Knee´ of the AGC: input signal level from which on the AGC action takes place. AGC threshold should be placed/adjusted just above the band noise for every particular RX situation to allow for optimal AGC action. The blue AGC box indicates when AGC action takes place and helps in adjusting this threshold. | 
| **AGC Hang enable**           (                  MENU_AGC_WDSP_HANG_ENABLE) | Enable/Disable Hang AGC function: If enabled: after the signal has decreased, the gain of the AGC is held constant for a certain time period (the hang time) in order to allow for speech pauses without disturbing noise because of fast acting AGC. | 
| **AGC Hang time**             (                    MENU_AGC_WDSP_HANG_TIME) | Hang AGC: hang time is the time period over which the AGC gain is held constant when in AGC Hang mode. After this period the gain is increased fast. | 
| **AGC Hang threshold**        (                  MENU_AGC_WDSP_HANG_THRESH) | ´Threshold´ for the Hang AGC: Hang AGC is useful for medium to strong signals. The Hang threshold determines the signal strength a signal has to exceed for Hang AGC to take place. | 
| **AGC Hang Decay**            (               MENU_AGC_WDSP_TAU_HANG_DECAY) | Time constant for the Hang AGC decay (speed of recovery of the AGC gain after hang time has expired) in milliseconds. | 
| **RX Codec Gain**             (                       MENU_CODEC_GAIN_MODE) | Sets the Codec IQ signal gain. Higher values represent higher gain. If set to AUTO the mcHF controls the gain so that the best dynamic range is used. | 
| **RX/TX Freq Xlate**          (                          MENU_RX_FREQ_CONV) | Controls offset of the receiver IQ signal base frequency from the dial frequency. Use of +/-12Khz is recommended. Switching it to OFF is not recommended as it disables certain features. | 
| **Mic/Line Select**           (                         MENU_MIC_LINE_MODE) | Select the required signal input for transmit (except in CW). Also changeable via long press on M3 | 
| **Mic Input Gain**            (                              MENU_MIC_GAIN) | Microphone gain. Also changeable via Encoder 3 if Microphone is selected as Input | 
| **Line Input Gain**           (                             MENU_LINE_GAIN) | LineIn gain. Also changeable via Encoder 3 if LineIn Left (L>L) or LineIn Right (L>R) is selected as Input | 
| **TX Audio Compress**         (                  MENU_TX_COMPRESSION_LEVEL) | Control the TX audio compressor. Higher values give more compression. Set to CUSTOM for user defined compression parameters. See below. Also changeable via Encoder 1 (CMP). | 
| **TX ALC Release Time**       (                           MENU_ALC_RELEASE) | If Audio Compressor Config is set to CUSTOM, sets the value of the Audio Compressor Release time. Otherwise shows predefined value of selected compression level. | 
| **TX ALC Input Gain**         (                     MENU_ALC_POSTFILT_GAIN) | If Audio Compressor Config is set to CUSTOM, sets the value of the ALC Input Gain. Otherwise shows predefined value of selected compression level. | 
| **RX NB Setting**             (                 MENU_NOISE_BLANKER_SETTING) | Set the Noise Blanker strength. Higher values mean more agressive blanking. Also changeable using Encoder 2 if Noise Blanker is active. | 
| **DSP NR Strength**           (                       MENU_DSP_NR_STRENGTH) | Set the Noise Reduction Strength. Higher values mean more agressive noise reduction but also higher CPU load. Use with extreme care. Also changeable using Encoder 2 if DSP is active. | 
| **TCXO Off/On/Stop**          (                             MENU_TCXO_MODE) | The software TCXO can be turned ON (set frequency is adjusted so that generated frequency matches the wanted frequency); OFF (no correction or measurement done); or STOP (no correction but measurement). | 
| **TCXO Temp. (C/F)**          (                              MENU_TCXO_C_F) | Show the measure TCXO temperature in Celsius or Fahrenheit. | 
| **Backup Config**             (                         MENU_BACKUP_CONFIG) | Backup your I2C Configuration to flash. If you don't have suitable I2C EEPROM installed this function is not available. | 
| **Restore Config**            (                        MENU_RESTORE_CONFIG) | Restore your I2C Configuration from flash. If you don't have suitable I2C EEPROM installed this function is not available. | 
| **Restart Codec**             (                         MENU_RESTART_CODEC) | Sometimes there is a problem with the I2S IQ signal stream from the Codec, resulting in mirrored signal reception. Restarting the CODEC Stream will cure that problem. Try more than once, if first call did not help. | 


## Configuration Menu (`MENU_CONF`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **Save Out-Of-Band Freq.**    (                CONFIG_FREQ_MEM_LIMIT_RELAX) | Select ON to save and restore frequencies which do not fit into the band during configuration saving (Power-Off or long press on Menu button) | 
| **TX on Out-Of-Band Freq.**   (                       CONFIG_TX_OUT_ENABLE) | Permit low power transmission even if the frequency is out of the official ham bands. DO NOT USE WITH CONNECTED ANTENNA! Use a dummy load! | 
| **Transmit Disable**          (                          CONFIG_TX_DISABLE) | Disable all transmissions unconditionally. In CW you will be able to hear a sidetone but no transmission is made. | 
| **Menu SW on TX disable**     (       CONFIG_AUDIO_MAIN_SCREEN_MENU_SWITCH) | Control if the screen automatically adapts Encoder value focus when switching between RX and TX. | 
| **TX Mute LineOut**           (                    CONFIG_MUTE_LINE_OUT_TX) | During transmission with frequency translation off, line out will carry one of the two signal channels. Good for CW but not very useful otherwise. You may switch this signal off here. | 
| **TX Initial Muting Time**    (              CONFIG_TXRX_SWITCH_AUDIO_MUTE) | When switching from RX to TX the audio and HF output will be muted for roughly VALUE ms. There are now several minimum times for muting defined in the firmware:<br/><br/> Input from Mic: 40ms<br/> Input from Line In: 40ms<br/> Digital Inputs (CW, USB): less than 1ms.<br/><br/> If the user defined 'TX Initial Muting Time' is set to more than zero, the maximum of both fixed input time and user defined time is used. Your microphone PTT switch is a potential source of noise if Mic is input! You need to increase the delay or change switches! | 
| **Max Volume**                (                          CONFIG_MAX_VOLUME) | Set maximum speaker&headphone volume.          | 
| **Max RX Gain (0=Max)**       (                         CONFIG_MAX_RX_GAIN) | Here you can set maximum gain for RX. A good choice is 3...5. If set to 0 RX is too sensitive in most working conditions. | 
| **Lineout Gain**              (                        CONFIG_LINEOUT_GAIN) | Set the constant gain level for the analog lineout jack | 
| **Key Beep**                  (                         CONFIG_BEEP_ENABLE) | If ON each keypress will generate a short beep | 
| **Beep Frequency**            (                           CONFIG_BEEP_FREQ) | Set beep frequency in Hz.                      | 
| **Beep Volume**               (                         CONFIG_BEEP_VOLUME) | Set beep volume.                               | 
| **CAT Mode**                  (                          CONFIG_CAT_ENABLE) | Enabled the FT 817 emulation via USB. See Wiki for more information. | 
| **CAT Running In Sandbox**    (                      CONFIG_CAT_IN_SANDBOX) | If On, frequency Changes made via CAT will not automatically switch bands and affect the manually selected frequencies. | 
| **CAT-DIQ-FREQ-XLAT**         (                            CONFIG_CAT_XLAT) | Select which frequency is reported via CAT Interface to the connected PC in Digital IQ Mode. If ON, it reports the displayed frequency. If OFF, it reports the center frequency, which is more useful with SDR programs. | 
| **XVTR Offs/Mult**            (                    CONFIG_XVTR_OFFSET_MULT) | When connecting to a transverter, set this to 1 and set the XVERTER Offset to the LO Frequency of it. The mcHF frequency is multiplied by this factor before the offset is added, so anything but 1 will result in each Hz in the mcHF being displayed as 2 to 10 Hz change on display. | 
| **XVTR Offset**               (               CONFIG_XVTR_FREQUENCY_OFFSET) | When transverter mode is enabled, this value is added to the mcHF frequency after being multiplied with the XVTR Offs/Mult. Use Step+ to set a good step width, much less turns with the dial knob if it is set to 1Mhz | 
| **Step Button Swap**          (               CONFIG_STEP_SIZE_BUTTON_SWAP) | If ON, Step- behaves like Step+ and vice versa. | 
| **Band+/- Button Swap**       (                    CONFIG_BAND_BUTTON_SWAP) | If ON, Band- behaves like Band+ and vice versa. | 
| **Reverse Touchscreen**       (                   MENU_REVERSE_TOUCHSCREEN) | Some touchscreens have the touch coordiantes reversed. In this case, select ON | 
| **Voltmeter Cal.**            (               CONFIG_VOLTMETER_CALIBRATION) | Adjusts the displayed value of the voltmeter.  | 
| **Freq. Calibrate**           (                 CONFIG_FREQUENCY_CALIBRATE) | Adjust the frequency correction of the local oscillator. Measure TX frequency and adjust until both match. Or use receive a know reference signal and zero-beat it and then adjust. More information in the Wiki. | 
| **Pwr. Display mW**           (                    CONFIG_FWD_REV_PWR_DISP) | Shows the forward and reverse power values in mW, can be used to calibrate the SWR meter. | 
| **Pwr. Det. Null**            (                     CONFIG_RF_FWD_PWR_NULL) |  Set the forward and reverse power sensors ADC zero power offset. This setting is enabled ONLY when Disp. Pwr (mW), is enabled. Needs SWR meter hardware modification to work. See Wiki Adjustment and Calibration. | 
| **SWR/PWR Meter FWD/REV Swap** (                  CONFIG_FWD_REV_SENSE_SWAP) | Exchange the assignment of the Power/SWR FWD and REV measurement ADC. Use if your power meter does not show anything during TX. | 
| **RX IQ Auto Correction**     (                  CONFIG_IQ_AUTO_CORRECTION) | Receive IQ phase and amplitude imbalance can be automatically adjusted by the mcHF. Switch ON/OFF here. If OFF, it takes the following menu values for compensating the imbalance. The automatic algorithm achieves up to 60dB mirror rejection. See Wiki Adjustments and Calibration. | 
| **TX IQ Balance (80m)**       (                  CONFIG_80M_TX_IQ_GAIN_BAL) | IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration. | 
| **TX IQ Phase   (80m)**       (                 CONFIG_80M_TX_IQ_PHASE_BAL) | IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration. | 
| **TX IQ Balance (10m)**       (                  CONFIG_10M_TX_IQ_GAIN_BAL) | IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration. | 
| **TX IQ Phase   (10m)**       (                 CONFIG_10M_TX_IQ_PHASE_BAL) | IQ Phase Adjust for all transmission if frequency translation is NOT OFF. Requires USB or LSB mode to be changeable. See Wiki Adjustments and Calibration. | 
| **TX IQ Balance (80m,CW)**    (        CONFIG_80M_TX_IQ_GAIN_BAL_TRANS_OFF) | IQ Balance Adjust for CW transmissions (and all transmission if frequency translation is OFF). See Wiki Adjustments and Calibration. | 
| **TX IQ Phase   (80m,CW)**    (       CONFIG_80M_TX_IQ_PHASE_BAL_TRANS_OFF) | IQ Phase Adjust for CW transmissions (and all transmission if frequency translation is OFF).See Wiki Adjustments and Calibration. | 
| **TX IQ Balance (10m,CW)**    (        CONFIG_10M_TX_IQ_GAIN_BAL_TRANS_OFF) | IQ Balance Adjust for CW transmissions (and all transmission if frequency translation is OFF).See Wiki Adjustments and Calibration. | 
| **TX IQ Phase   (10m,CW)**    (       CONFIG_10M_TX_IQ_PHASE_BAL_TRANS_OFF) | IQ Phase Adjust for CW transmissions (and all transmission if frequency translation is OFF).See Wiki Adjustments and Calibration. | 
| **DSP NR BufLen**             (   CONFIG_DSP_NR_DECORRELATOR_BUFFER_LENGTH) | DSP LMS noise reduction: length of the audio buffer that is used for simulation of a reference for the LMS algorithm. The longer the buffer, the better the performance, but this buffer length must always be larger than the number of taps in the FIR filter used. Thus, a larger buffer (and larger FIR filter) uses more MCU resources. | 
| **DSP NR FIR NumTaps**        (                  CONFIG_DSP_NR_FFT_NUMTAPS) | DSP LMS noise reduction: Number of taps in the DSP noise reduction FIR filter. The larger the number of taps in the filter, the better the performance, but the slower the performance of the filter and the mcHF. | 
| **DSP NR Post-AGC**           (              CONFIG_DSP_NR_POST_AGC_SELECT) | DSP LMS noise reduction: Perform the DSP LMS noise reduction BEFORE or AFTER the AGC. NO = before AGC, YES = after AGC. | 
| **DSP Notch ConvRate**        (             CONFIG_DSP_NOTCH_CONVERGE_RATE) | DSP LMS automatic notch filter:                | 
| **DSP Notch BufLen**          (CONFIG_DSP_NOTCH_DECORRELATOR_BUFFER_LENGTH) | DSP LMS automatic notch filter: length of the audio buffer that is used for simulation of a reference for the LMS algorithm. The longer the buffer, the better -and the slower- the performance, but this buffer length must always be larger than the number of taps in the FIR filter used. Thus, a larger buffer (and larger FIR filter) uses more MCU resources. | 
| **DSP Notch FIRNumTap**       (               CONFIG_DSP_NOTCH_FFT_NUMTAPS) | DSP LMS automatic notch filter: Number of taps in the DSP automatic notch FIR filter. The larger the number of taps in the filter, the better the performance, but the slower the performance of the filter and the mcHF. | 
| **NB AGC T/C (<=Slow)**       (                   CONFIG_AGC_TIME_CONSTANT) | Noise Blanker AGC time constant adjustment: Lower values are equivalent with slower Noise blanker AGC. While the menu is displayed, the noise blanker is switched OFF, so in order to test the effect of adjusting this parameter, leave the menu. | 
| **Reset Config EEPROM**       (                    CONFIG_RESET_SER_EEPROM) | Clear the EEPROM so that at next start all stored configuration data is reset to the values stored in Flash (see Backup/Restore). | 


## Display Menu (`MENU_DISPLAY`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **LCD Auto Blank**            (                   CONFIG_LCD_AUTO_OFF_MODE) | After x seconds LCD turns dark and LCD data sections stop. So power consumption is decreased and RX hum is decreased, too. LCD operation starts when using any button or the touchscreen. | 
| **Step Size Marker**          (               CONFIG_FREQ_STEP_MARKER_LINE) | If enabled, you'll see a line under the digit which is currently representing the selected tuning step size | 
| **Filter BW Display**         (               CONFIG_DISP_FILTER_BANDWIDTH) | Colour of the horizontal Filter Bandwidth indicator bar. | 
| **Spectrum Type**             (                         MENU_SPECTRUM_MODE) | Select if you want a scope-like or a waterfall-like (actually a fountain) display | 
| **Spectrum Magnify**          (                      MENU_SPECTRUM_MAGNIFY) | Select level of magnification (1x, 2x, 4x, 8x, 16x, 32x) of spectrum and waterfall display. Also changeable via touch screen. Refresh rate is much slower with high magnification settings. The dBm display has its maximum accuracy in magnify 1x setting. | 
| **Spectrum Size**             (                         MENU_SPECTRUM_SIZE) | Change height of spectrum display              | 
| **Spectrum Filter**           (              MENU_SPECTRUM_FILTER_STRENGTH) | Lowpass filter for the spectrum FFT. Low values: fast and nervous spectrum; High values: slow and calm spectrum. | 
| **Spec FreqScale Colour**     (             MENU_SPECTRUM_FREQSCALE_COLOUR) | Colour of the small frequency digits under the spectrum display. | 
| **Spec Line Colour**          (           MENU_SPECTRUM_CENTER_LINE_COLOUR) | Colour of the vertical line indicating the Receive frequency in the spectrum or waterdall display. | 
| **Spectrum FFT Wind.**        (            CONFIG_SPECTRUM_FFT_WINDOW_TYPE) | Selects the window algorithm for the spectrum FFT. For low spectral leakage, Hann, Hamming or Blackman window is recommended. | 
| **Scope Light**               (                    MENU_SCOPE_LIGHT_ENABLE) | The scope uses bars (NORMAL) or points (LIGHT) to represent data. LIGHT is a little less resource intensive. | 
| **Scope 1/Speed**             (                           MENU_SCOPE_SPEED) | Lower Values: Higher refresh rate              | 
| **Scope AGC Adj.**            (                      MENU_SCOPE_AGC_ADJUST) | Adjusting of scope / waterfall AGC for fitting graphs to screen | 
| **Scope Trace Colour**        (                    MENU_SCOPE_TRACE_COLOUR) | Set colour of scope                            | 
| **Scope Grid Colour**         (                     MENU_SCOPE_GRID_COLOUR) | Set colour of scope grid                       | 
| **Scope Div.**                (                     MENU_SCOPE_DB_DIVISION) | Set rf range for scope                         | 
| **Scope NoSig Adj.**          (                    MENU_SCOPE_NOSIG_ADJUST) | Set scope line corresponding to NO SIGNAL      | 
| **Wfall 1/Speed**             (                           MENU_WFALL_SPEED) | Lower Values: Higher refresh rate.             | 
| **Wfall Colours**             (                    MENU_WFALL_COLOR_SCHEME) | Select colour scheme for waterfall display.    | 
| **Wfall Step Size**           (                       MENU_WFALL_STEP_SIZE) | How many lines are moved in a single screen update | 
| **Wfall Brightness**          (                          MENU_WFALL_OFFSET) | Set to input level which waterfall uses for lowest level | 
| **Wfall Contrast**            (                        MENU_WFALL_CONTRAST) | Adjust to fit your personal input level range to displayable colour range for waterfall | 
| **Wfall NoSig Adj.**          (                    MENU_WFALL_NOSIG_ADJUST) | Set NO SIGNAL state for waterfall              | 
| **Upper Meter Colour**        (                       MENU_METER_COLOUR_UP) | Set the colour of the scale of combined S/Power-Meter | 
| **Lower Meter Colour**        (                     MENU_METER_COLOUR_DOWN) | Set the colour of the scale of combined SWR/AUD/ALC-Meter | 
| **dBm display**               (                           MENU_DBM_DISPLAY) | RX signal power (measured within the filter bandwidth) can be displayed in dBm or normalized as dBm/Hz. This value is supposed to be quite accurate to +-3dB. Preferably use low spectrum display magnify settings. Accuracy is lower for very very weak and very very strong signals. | 
| **dBm calibrate**             (                         MENU_DBM_CALIBRATE) | dBm display calibration. Just an offset (in dB) that is added to the internally calculated dBm or dBm/Hz value. | 
| **S-Meter**                   (                               MENU_S_METER) | Select the S-Meter measurement style. In old school mode, the RF Gain influences the displayed S-Meter value, higher RFG values increase the S-Meter value. In all other settings, the S-Meter is based on the dBm measurement and is thus a more accurate and objective reflection of the signal strength. | 


## CW Mode Settings (`MENU_CW`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **CW Keyer Mode**             (                            MENU_KEYER_MODE) | Select how the mcHF interprets the connected keyer signals. Supported modes: Iambic A and B Keyer (IAM A/B), Straight Key (STR_K), and Ultimatic Keyer (ULTIM) | 
| **CW Keyer Speed**            (                           MENU_KEYER_SPEED) | Keyer Speed for the automatic keyer modes in WpM. Also changeable via Encoder 3 if in CW Mode. | 
| **CW Sidetone Gain**          (                         MENU_SIDETONE_GAIN) | Audio volume for the monitor sidetone in CW TX. Also changeable via Encoder 1 if in CW Mode. | 
| **CW Side/Offset Freq**       (                    MENU_SIDETONE_FREQUENCY) | Sidetone Frequency (also Offset frequency, see CW Freq. Offset below) | 
| **CW Paddle Reverse**         (                        MENU_PADDLE_REVERSE) | Dit is Dah and Dah is Dit. Use if your keyer needs reverse meaning of the paddles. | 
| **CW TX->RX Delay**           (                        MENU_CW_TX_RX_DELAY) | How long to stay in CW TX mode after stop sending a signal. | 
| **CW Freq. Offset**           (                        MENU_CW_OFFSET_MODE) | TX: display is TX frequency if received frequency was zero-beated. DISP: display is RX frequency if received signal is matched to sidetone. SHIFT: LO shifts, display is RX frequency if signal is matched to sidetone. | 
| **CW LSB/USB Select**         (                   MENU_CW_AUTO_MODE_SELECT) | Set appropriate sideband mode for CW. If AUTO, sideband is chosen for bands by its frequency. A long press on Mode button gets the other sideband mode | 


## Filter Selection (`MENU_FILTER`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **SSB Filter 1**              (                             MENU_FP_SSB_01) | Filter bandwidth #1 when toggling with filter select button in LSB or USB. | 
| **SSB Filter 2**              (                             MENU_FP_SSB_02) | Filter bandwidth #2 when toggling with filter select button in LSB or USB. | 
| **SSB Filter 3**              (                             MENU_FP_SSB_03) | Filter bandwidth #3 when toggling with filter select button in LSB or USB. | 
| **SSB Filter 4**              (                             MENU_FP_SSB_04) | Filter bandwidth #4 when toggling with filter select button in LSB or USB. | 
| **CW Filter 1**               (                              MENU_FP_CW_01) | Filter bandwidth #1 when toggling with filter select button in CW. | 
| **CW Filter 2**               (                              MENU_FP_CW_02) | Filter bandwidth #2 when toggling with filter select button in CW. | 
| **CW Filter 3**               (                              MENU_FP_CW_03) | Filter bandwidth #3 when toggling with filter select button in CW. | 
| **CW Filter 4**               (                              MENU_FP_CW_04) | Filter bandwidth #4 when toggling with filter select button in CW. | 
| **AM/SAM Filter 1**           (                              MENU_FP_AM_01) | Filter bandwidth #1 when toggling with filter select button in AM & SAM. | 
| **AM/SAM Filter 2**           (                              MENU_FP_AM_02) | Filter bandwidth #2 when toggling with filter select button in AM & SAM. | 
| **AM/SAM Filter 3**           (                              MENU_FP_AM_03) | Filter bandwidth #3 when toggling with filter select button in AM & SAM. | 
| **AM/SAM Filter 4**           (                              MENU_FP_AM_04) | Filter bandwidth #4 when toggling with filter select button in AM & SAM. | 
| **AM  TX Audio Filter**       (                CONFIG_AM_TX_FILTER_DISABLE) | Select if AM-TX signal is filtered (strongly recommended to agree to regulations) | 
| **SSB TX Audio Filter2**      (                       CONFIG_SSB_TX_FILTER) | Select if SSB-TX signal is filtered (strongly recommended to agree to regulations) | 


## PA Configuration (`MENU_POW`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **Tune Power Level**          (                    CONFIG_TUNE_POWER_LEVEL) | Select the power level for TUNE operation. May be set using the selected power level or have a fixed power level. | 
| **Tune Tone (SSB)**           (                      CONFIG_TUNE_TONE_MODE) | Select if single tone or two tone is generated during TUNE operation. Not persistent. | 
| **CW PA Bias (If >0 )**       (                          CONFIG_CW_PA_BIAS) | If set to a value above 0, this BIAS is used during CW transmission; otherwise normal BIAS is used during CW | 
| **Reduce Power on Low Bands** (           CONFIG_REDUCE_POWER_ON_LOW_BANDS) | If set (recommended!)  frequencies below 8Mhz (40m or lower) require higher power adjust values (four times). This permits better control of generated power on these frequencies. | 
| **Reduce Power on High Bands** (          CONFIG_REDUCE_POWER_ON_HIGH_BANDS) | If set frequencies above 8Mhz (30m or higher) require higher power adjust values (four times). This permits better control of generated power on these frequencies. | 
| **PA Bias**                   (                             CONFIG_PA_BIAS) | Defines the BIAS value of the PA. See Adjustment and Calibration for more information. | 
| **160m  5W PWR Adjust**       (                      CONFIG_160M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **80m   5W PWR Adjust**       (                       CONFIG_80M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **60m   5W PWR Adjust**       (                       CONFIG_60M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **40m   5W PWR Adjust**       (                       CONFIG_40M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **30m   5W PWR Adjust**       (                       CONFIG_30M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **20m   5W PWR Adjust**       (                       CONFIG_20M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **17m   5W PWR Adjust**       (                       CONFIG_17M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **15m   5W PWR Adjust**       (                       CONFIG_15M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **12m   5W PWR Adjust**       (                       CONFIG_12M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **10m   5W PWR Adjust**       (                       CONFIG_10M_5W_ADJUST) | Defines the internal power adjustment factor to achieve 5W power on this band. See Adjustment and Calibration for more information. | 
| **160m  Full PWR Adjust**     (              CONFIG_160M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **80m   Full PWR Adjust**     (               CONFIG_80M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **60m   Full PWR Adjust**     (               CONFIG_60M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **40m   Full PWR Adjust**     (               CONFIG_40M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **30m   Full PWR Adjust**     (               CONFIG_30M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **20m   Full PWR Adjust**     (               CONFIG_20M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **17m   Full PWR Adjust**     (               CONFIG_17M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **15m   Full PWR Adjust**     (               CONFIG_15M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **12m   Full PWR Adjust**     (               CONFIG_12M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **10m   Full PWR Adjust**     (               CONFIG_10M_FULL_POWER_ADJUST) | Defines the internal power adjustment factor to achieve full power on this band. Check the output signal when adjusting for full power! See Adjustment and Calibration for more information. | 
| **160m Coupling Adj.**        (           CONFIG_FWD_REV_COUPLING_160M_ADJ) | Power Meter Adjustment factor for the 160m band power values. See Wiki. | 
| **80m  Coupling Adj.**        (            CONFIG_FWD_REV_COUPLING_80M_ADJ) | Power Meter Adjustment factor for the 80m band power values. See Wiki. | 
| **40m  Coupling Adj.**        (            CONFIG_FWD_REV_COUPLING_40M_ADJ) | Power Meter Adjustment factor for the 40m and 60m band power values. See Wiki. | 
| **20m  Coupling Adj.**        (            CONFIG_FWD_REV_COUPLING_20M_ADJ) | Power Meter Adjustment factor for the 20m and 30m band power values. See Wiki. | 
| **15m  Coupling Adj.**        (            CONFIG_FWD_REV_COUPLING_15M_ADJ) | Power Meter Adjustment factor for the 10m - 17m bands power values. See Wiki. | 


## System Info (`MENU_SYSINFO`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **Display**                   (                               INFO_DISPLAY) | Displays working mode (SPI/parallel            | 
| **Disp. Controller**          (                          INFO_DISPLAY_CTRL) | identified LCD controller chip                 | 
| **SI570**                     (                                 INFO_SI570) | startup frequency and I2C addrss of SI570      | 
| **EEPROM**                    (                                INFO_EEPROM) | type of serial EEPROM and its capacity         | 
| **Touchscreen**               (                                    INFO_TP) | touchscreen state                              | 
| **CPU**                       (                                   INFO_CPU) | identification of fitted MCU                   | 
| **Flash Size (kB)**           (                                 INFO_FLASH) | flash size of MCU                              | 
| **RAM Size (kB)**             (                                   INFO_RAM) | RAM size of MCU                                | 
| **Firmware**                  (                            INFO_FW_VERSION) | firmware version                               | 
| **Build**                     (                                 INFO_BUILD) | firmware: timestamp of building                | 
| **Bootloader**                (                            INFO_BL_VERSION) | bootloader version                             | 
| **RF Bands Mod**              (                                 INFO_RFMOD) | RF bands expansion PCB present?                | 
| **V/UHF Mod**                 (                             INFO_VHFUHFMOD) | VHF/UHF bands expansion PCB present?           | 
| **Backup RAM Battery**        (                                  INFO_VBAT) | Battery Support for Backup RAM present?        | 
| **Real Time Clock**           (                                   INFO_RTC) | Battery Supported Real Time Clock present?     | 
| **FW licensed under **        (                               INFO_LICENCE) | Display license of firmware                    | 


## Debug/Exper. Settings (`MENU_DEBUG`)
    
| LABEL                         (                                         NR) | DESCRIPTION                                    | 
| --------------------------------------------------------------------------- | ---------------------------------------------- | 
| **TX Audio via USB**          (                        MENU_DEBUG_TX_AUDIO) | If enabled, send generated audio to PC during TX. | 
| **I2C1 Bus Speed**            (                      MENU_DEBUG_I2C1_SPEED) | Sets speed of the I2C1 bus (Si570 oscillator and MCP9801 temperature sensor). Higher speeds provide quicker RX/TX switching but may also cause tuning issues (red digits). Be careful with speeds above 200 kHz. Stored permanently. Will be moved to Configuration menu in future. | 
| **I2C2 Bus Speed**            (                      MENU_DEBUG_I2C2_SPEED) | Sets operation speed of the I2C2 bus (Audio Codec and I2C EEPROM). Higher speeds provide quicker RX/TX switching, configuration save and power off. Many mcHF seem to run with 400kHz without problems. Be careful with speeds above 100 kHz. Stored permanently. Will be moved to Configuration menu in future. | 
| **FT817 Clone Transmit**      (                        MENU_DEBUG_CLONEOUT) | Will in future send out memory data to an FT817 Clone Info (to be used with CHIRP). | 
| **FT817 Clone Receive**       (                         MENU_DEBUG_CLONEIN) | Will in future get memory data from an FT817 Clone Info (to be used with CHIRP). | 


[//]: # ( EOFILE                                                                       )

