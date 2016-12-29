
[//]: # (                                                                              )
[//]: # ( WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !                         )
[//]: # (                                                                              )
[//]: # ( generated from  <${BUILD_ID}>  at  2016-12-29T21:11:30  by  ui_menu_structure_mdtable.py )
[//]: # (                                                                              )
[//]: # ( mcHF SDR TRX - Menu Structure Diagram as MarkDown-Table                      )
[//]: # (                                                                              )
[//]: # ( see <https://help.github.com/categories/writing-on-github/>                  )
[//]: # (                                                                              )
    

## Standard Menu (STD, `MENU_BASE`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **LSB/USB Auto Select**       (031) | If enabled, the appropriate sideband mode for SSB and FreeDV is chosen as default for bands by its frequency. | 
| **Digital Modes**             (030) | Disable appearance of digital modes when pressing Mode button | 
| **CW Mode**                   (030) | Disable appearance of CW mode when pressing Mode button | 
| **AM Mode**                   (030) | Disable appearance of AM mode when pressing Mode button | 
| **SyncAM Mode**               (SAM) | Disable appearance of SyncAM modeswhen pressing Mode button | 
| **FM Mode**                   (040) | Disable appearance of FM mode when pressing Mode button | 
| **FM Sub Tone Gen**           (041) | Enable generation of CTCSS tones during FM transmissions. | 
| **FM Sub Tone Det**           (042) | Enable detection of CTCSS tones during FM receive. RX is muted unless tone is detected. | 
| **FM Tone Burst**             (043) | Enabled sending of short tone at begin of each FM transmission. Used to open repeaters. Available frequencies are 1750 Hz and 2135 Hz. | 
| **FM Deviation**              (045) | Select between normal and narrow deviation (5 and 2.5kHz) for FM RX/TX | 
| **RF Gain**                   (051) | RF Receive Gain. This setting is also accessible via Encoder 2, RFG. | 
| **AGC Mode**                  (050) | Automatic Gain Control Mode setting. You may select preconfigured settings (SLOW,MED,FAST), define settings yourself (CUSTOM) or use MANUAL (no AGC, use RFG to control gain | 
| **Custom AGC (+=Slower)**     (052) | If AGC is set to CUSTOM, this controls the speed setting of AGC | 
| **RX Codec Gain**             (053) | Sets the Codec IQ signal gain. Higher values represent higher gain. If set to AUTO the mcHF controls the gain so that the dynamic range is used best. | 
| **RX/TX Freq Xlate**          (055) | Controls offset of the receiver IQ signal base frequency from the dial frequency. Use of +/-12Khz is recommended. Switching it to OFF is not recommended as it disables certain features. | 
| **Mic/Line Select**           (060) | Select used signal input for transmit (except in CW). Also changeable via long press on M3 | 
| **Mic Input Gain**            (061) | Microphone gain. Also changeable via Encoder 3 if Microphone is selected as Input | 
| **Line Input Gain**           (062) | LineIn gain. Also changeable via Encoder 3 if LineIn Left (L>L) or LineIn Right (L>R) is selected as Input | 
| **TX Audio Compress**         (065) | Control the TX audio compressor. Higher values == more compression. Set to CUSTOM to set user defined compression parameters. See below. Also changeable via Encoder 1 (CMP). | 
| **TX ALC Release Time**       (063) | If Audio Compressor Config is set to CUSTOM, sets the value of the Audio Compressor Release time. Otherwise shows predefined value of selected compression level. | 
| **TX ALC Input Gain**         (064) | If Audio Compressor Config is set to CUSTOM, sets the value of the ALC Input Gain. Otherwise shows predefined value of selected compression level. | 
| **RX NB Setting**             (054) | Set the Noise Blanker strength. Higher values mean more agressive blanking. Also changeable using Encoder 2 if Noise Blanker is active. | 
| **DSP NR Strength**           (010) | Set the Noise Reduction Strength. Higher values mean more agressive noise reduction but also higher CPU load. Use with extreme care. Also changeable using Encoder 2 if DSP is active. | 
| **TCXO Off/On/Stop**          (090) | The software TCXO can be turned ON (set frequency is adjusted so that generated frequency matches the wanted frequency); OFF (no correction or measurement done); or STOP (no correction but measurement). | 
| **TCXO Temp. (C/F)**          (091) | Show the measure TCXO temperature in Celsius or Fahrenheit. | 
| **Backup Config**             (197) | Backup your I2C Configuration to flash. If you don't have suitable I2C EEPROM installed this function is not available. | 
| **Restore Config**            (198) | Restore your I2C Configuration from flash. If you don't have suitable I2C EEPROM installed this function is not available. | 
| **Restart Codec**             (198) | Sometimes there is a problem with the I2S IQ signal stream from the Codec, resulting in mirrored signal reception. Restarting the CODEC Stream will cure that problem. Try more than once, if first call did not help. | 


## Configuration Menu (CON, `MENU_CONF`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **Save Out-Of-Band Freq.**    (232) | Select ON to save and restore frequencies which do not fit into the band during configuration saving (Power-Off or long press on Menu button) | 
| **TX on Out-Of-Band Freq.**   (207) | Permit low power transmission even if the frequency is out of the official ham bands. DO NOT USE WITH CONNECTED ANTENNA! Use a dummy load! | 
| **Transmit Disable**          (203) | Disable all transmissions unconditionally. In CW you will be able to hear a sidetone but not transmission is made. | 
| **Menu SW on TX disable**     (204) | Control if the screen automatically adapts Encoder value focus when switching between RX and TX. | 
| **TX Mute LineOut**           (205) | During transmission with frequency translation off line out will carry one of the two signal channels. Good for CW but not very useful otherwise. You may switch this signal off here. | 
| **TX Initial Muting Time**    (206) | When switching from RX to TX the audio and HF output will be muted for roughly VALUE ms. | 
| **Max Volume**                (210) | :soon:                                         | 
| **Max RX Gain (0=Max)**       (211) | :soon:                                         | 
| **Key Beep**                  (212) | If ON each keypress will generate a short beep | 
| **Beep Frequency**            (213) | Set beep frequency in Hz.                      | 
| **Beep Volume**               (214) | Set beep volume.                               | 
| **CAT Mode**                  (220) | Enabled the FT 817 emulation via USB. See Wiki for more information. | 
| **CAT Running In Sandbox**    (530) | If On, frequency Changes made via CAT will not automatically switch bands and affect the manually selected frequencies. | 
| **CAT-DIQ-FREQ-XLAT**         (400) | Select which frequency is reported via CAT Interface to the connected PC in Digital IQ Mode. If OFF, it reports the displayed frequency. If ON, it reports the center frequency, which is more useful with SDR programs. | 
| **XVTR Offs/Mult**            (280) | When connecting to a transverter, set this to 1 and set the XVERTER Offset to the LO Frequency of it. | 
| **XVTR Offset**               (281) | When transverter mode is enabled, this value is added to the mcHF frequency after being multiplied with the XVTR Offs/Mult. Use Step+ to set a good step width, much less turns with the dial knob if it is set to 1Mhz | 
| **Step Button Swap**          (201) | If ON, Step- behaves like Step+ and vice versa. | 
| **Band+/- Button Swap**       (202) | If ON, Band- behaves like Band+ and vice versa. | 
| **Reverse Touchscreen**       (122) | Some touchscreens have the touch coordiantes reversed. In this case, select ON | 
| **Voltmeter Cal.**            (208) | Adjusts the displayed value of the voltmeter.  | 
| **Freq. Calibrate**           (230) | Adjust the frequency correction of the local oscillator. Select 1Hz step size and measure TX frequency and adjust until both match. Or receive a know reference signal and zero-beat it and then adjust. More information in the Wiki. | 
| **Pwr. Det. Null**            (271) | :soon:                                         | 
| **FWD/REV ADC Swap.**         (276) | :soon:                                         | 
| **RX IQ Balance (80m)**       (240) | See Wiki Adjustments and Calibration.          | 
| **RX IQ Phase   (80m)**       (241) | See Wiki Adjustments and Calibration.          | 
| **RX IQ Balance (10m)**       (242) | See Wiki Adjustments and Calibration.          | 
| **RX IQ Phase   (10m)**       (243) | See Wiki Adjustments and Calibration.          | 
| **TX IQ Balance (80m)**       (250) | See Wiki Adjustments and Calibration.          | 
| **TX IQ Phase   (80m)**       (251) | See Wiki Adjustments and Calibration.          | 
| **TX IQ Balance (10m)**       (252) | See Wiki Adjustments and Calibration.          | 
| **TX IQ Phase   (10m)**       (253) | See Wiki Adjustments and Calibration.          | 
| **DSP NR BufLen**             (310) | :soon:                                         | 
| **DSP NR FFT NumTaps**        (311) | :soon:                                         | 
| **DSP NR Post-AGC**           (312) | :soon:                                         | 
| **DSP Notch ConvRate**        (313) | :soon:                                         | 
| **DSP Notch BufLen**          (314) | :soon:                                         | 
| **DSP Notch FFTNumTap**       (315) | :soon:                                         | 
| **NB AGC T/C (<=Slow)**       (320) | :soon:                                         | 
| **Reset Config EEPROM**       (341) | Clear the EEPROM so that at next start all stored configuration data is reset to the values stored in Flash (see Backup/Restore). | 


## Display Menu (DIS, `MENU_DISPLAY`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **LCD Auto Blank**            (090) | :soon:                                         | 
| **Step Size Marker**          (091) | If enabled, you'll see a line under the digit which is currently representing the selected step size | 
| **Filter BW Display**         (092) | :soon:                                         | 
| **Spectrum Type**             (109) | Select if you want a scope-like or a waterfall-like (actually a fountain) display | 
| **Spectrum Magnify**          (105) | Select level of magnification of spectrum. Also changeable via touch screen. | 
| **Spectrum Size**             (117) | Change height of spectrum display              | 
| **Spectrum Filter**           (101) | :soon:                                         | 
| **Spec FreqScale Colour**     (104) | :soon:                                         | 
| **Spec Line Colour**          (108) | :soon:                                         | 
| **Spectrum FFT Wind.**        (340) | Selects the use window algorithm for spectrum FFT | 
| **Scope Light**               ( 99) | The scope uses bars (NORMAL) or points (LIGHT) to represent data. LIGHT is a little less resource intensive. | 
| **Scope 1/Speed**             (100) | Lower Values: Higher refresh rate              | 
| **Scope AGC Adj.**            (106) | :soon:                                         | 
| **Scope Trace Colour**        (102) | :soon:                                         | 
| **Scope Grid Colour**         (103) | :soon:                                         | 
| **Scope Div.**                (107) | :soon:                                         | 
| **Scope NoSig Adj.**          (115) | :soon:                                         | 
| **Wfall 1/Speed**             (114) | Lower Values: Higher refresh rate.             | 
| **Wfall Colours**             (110) | Select colour scheme for waterfall display.    | 
| **Wfall Step Size**           (111) | How many lines are moved in a single screen update | 
| **Wfall Brightness**          (112) | :soon:                                         | 
| **Wfall Contrast**            (113) | :soon:                                         | 
| **Wfall NoSig Adj.**          (116) | :soon:                                         | 
| **Upper Meter Colour**        (122) | Set the colour of the scale of combined S/Power-Meter | 
| **Lower Meter Colour**        (123) | Set the colour of the scale of combined SWR/AUD/ALC-Meter | 
| **dBm display**               (120) | Enable an additional numeric display of the RX signal strength | 
| **S-Meter**                   (121) | Select the S-Meter measurement style. In old school mode, the RF Gain influence the S-Meter settings, higher RFG values increase S-Meter value. | 


## CW Mode Settings (CW , `MENU_CW`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **CW Keyer Mode**             (070) | Select how the mcHF interprets the connected keyer signals. Supported modes: Iambic A and B Keyer (IAM A/B), Straight Key (STR_K), and Ultimatic Keyer (ULTIM) | 
| **CW Keyer Speed**            (071) | Keyer Speed for the automatic keyer modes in WpM. Also changeable via Encoder 3 if in CW Mode. | 
| **CW Sidetone Gain**          (072) | Audio volume for the monitor sidetone in CW TX. Also changeable via Encoder 1 if in CW Mode. | 
| **CW Side/Offset Freq**       (073) | Sidetone Frequency (also Offset frequency, see CW Freq. Offset below) | 
| **CW Paddle Reverse**         (074) | Dit is Dah and Dah is Dit. Use if your keyer needs reverse meaning of the paddles. | 
| **CW TX->RX Delay**           (075) | How long to stay in CW mode after stop sending a signal. | 
| **CW Freq. Offset**           (076) | TX: display is TX frequency if received frequency was zero-beated. DISP: display is RX frequency if received signal is matched to sidetone. SHIFT: LO shifts, display is RX frequency if signal is matched to sidetone. | 
| **CW LSB/USB Select**         (031) | Set appropriate sideband mode for CW. If AUTO, sideband is chosen for bands by its frequency. Long press on Mode button to get the other sideband mode | 


## Filter Selection (FIL, `MENU_FILTER`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **SSB Filter 1**              (600) | :soon:                                         | 
| **SSB Filter 2**              (600) | :soon:                                         | 
| **SSB Filter 3**              (600) | :soon:                                         | 
| **SSB Filter 4**              (600) | :soon:                                         | 
| **CW Filter 1**               (600) | :soon:                                         | 
| **CW Filter 2**               (600) | :soon:                                         | 
| **CW Filter 3**               (600) | :soon:                                         | 
| **CW Filter 4**               (600) | :soon:                                         | 
| **AM Filter 1**               (600) | :soon:                                         | 
| **AM Filter 2**               (600) | :soon:                                         | 
| **AM Filter 3**               (600) | :soon:                                         | 
| **AM Filter 4**               (600) | :soon:                                         | 
| **SAM Filter 1**              (600) | :soon:                                         | 
| **SAM Filter 2**              (600) | :soon:                                         | 
| **SAM Filter 3**              (600) | :soon:                                         | 
| **SAM Filter 4**              (600) | :soon:                                         | 
| **AM  TX Audio Filter**       (330) | :soon:                                         | 
| **SSB TX Audio Filter2**      (332) | :soon:                                         | 


## PA Configuration (POW, `MENU_POW`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **Tune Power Level**          (P00) | Select the power level for TUNE operation. May be set to follow the selected power level or have a fixed power level. | 
| **Tune Tone (SSB)**           (P99) | Select if single tone or two tone is generated during TUNE operation. Not persistent. | 
| **CW PA Bias (If >0 )**       (260) | If set to a value above 0, this BIAS is used during CW transmission; otherwise normal BIAS is used during CW | 
| **Reduce Power on Low Bands** (P0A) | If set (recommended!)  frequencies below 8Mhz require higher power adjust values. This permits better control of generated power on these frequencies. | 
| **PA Bias**                   (261) | Defines the BIAS value of the PA. See Adjustment and Calibration for more information. | 
| **2200m 5W PWR Adjust**       (P01) | :soon:                                         | 
| **630m  5W PWR Adjust**       (P02) | :soon:                                         | 
| **160m  5W PWR Adjust**       (P03) | :soon:                                         | 
| **80m   5W PWR Adjust**       (P04) | :soon:                                         | 
| **60m   5W PWR Adjust**       (P05) | :soon:                                         | 
| **40m   5W PWR Adjust**       (P06) | :soon:                                         | 
| **30m   5W PWR Adjust**       (P07) | :soon:                                         | 
| **20m   5W PWR Adjust**       (P08) | :soon:                                         | 
| **17m   5W PWR Adjust**       (P09) | :soon:                                         | 
| **15m   5W PWR Adjust**       (P10) | :soon:                                         | 
| **12m   5W PWR Adjust**       (P11) | :soon:                                         | 
| **10m   5W PWR Adjust**       (P12) | :soon:                                         | 
| **6m    5W PWR Adjust**       (P13) | :soon:                                         | 
| **4m    5W PWR Adjust**       (P14) | :soon:                                         | 
| **2m    5W PWR Adjust**       (P15) | :soon:                                         | 
| **70cm  5W PWR Adjust**       (P16) | :soon:                                         | 
| **23cm  5W PWR Adjust**       (P17) | :soon:                                         | 
| **2200m Full PWR Adjust**     (O01) | :soon:                                         | 
| **630m  Full PWR Adjust**     (O02) | :soon:                                         | 
| **160m  Full PWR Adjust**     (O03) | :soon:                                         | 
| **80m   Full PWR Adjust**     (O04) | :soon:                                         | 
| **60m   Full PWR Adjust**     (O05) | :soon:                                         | 
| **40m   Full PWR Adjust**     (O06) | :soon:                                         | 
| **30m   Full PWR Adjust**     (O07) | :soon:                                         | 
| **20m   Full PWR Adjust**     (O08) | :soon:                                         | 
| **17m   Full PWR Adjust**     (O09) | :soon:                                         | 
| **15m   Full PWR Adjust**     (O10) | :soon:                                         | 
| **12m   Full PWR Adjust**     (O11) | :soon:                                         | 
| **10m   Full PWR Adjust**     (O12) | :soon:                                         | 
| **6m    Full PWR Adjust**     (O13) | :soon:                                         | 
| **4m    Full PWR Adjust**     (O14) | :soon:                                         | 
| **2m    Full PWR Adjust**     (O15) | :soon:                                         | 
| **70cm  Full PWR Adjust**     (O16) | :soon:                                         | 
| **23cm  Full PWR Adjust**     (O17) | :soon:                                         | 
| **2200m Coupling Adj.**       (C01) | :soon:                                         | 
| **630m Coupling Adj.**        (C02) | :soon:                                         | 
| **160m Coupling Adj.**        (C03) | :soon:                                         | 
| **80m  Coupling Adj.**        (C04) | :soon:                                         | 
| **40m  Coupling Adj.**        (C05) | :soon:                                         | 
| **20m  Coupling Adj.**        (C06) | :soon:                                         | 
| **15m  Coupling Adj.**        (C07) | :soon:                                         | 
| **6m   Coupling Adj.**        (C08) | :soon:                                         | 
| **2m   Coupling Adj.**        (C09) | :soon:                                         | 
| **70cm Coupling Adj.**        (C10) | :soon:                                         | 
| **23cm Coupling Adj.**        (C11) | :soon:                                         | 


## System Info (INF, `MENU_SYSINFO`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **Display**                   (I01) | :soon:                                         | 
| **Disp. Controller**          (I02) | :soon:                                         | 
| **SI570**                     (I02) | :soon:                                         | 
| **EEPROM**                    (I03) | :soon:                                         | 
| **Touchscreen**               (I04) | :soon:                                         | 
| **CPU**                       (I07) | :soon:                                         | 
| **Flash Size (kB)**           (I07) | :soon:                                         | 
| **RAM Size (kB)**             (I08) | :soon:                                         | 
| **Firmware**                  (I08) | :soon:                                         | 
| **Build**                     (I08) | :soon:                                         | 
| **Bootloader**                (I08) | :soon:                                         | 
| **RF Bands Mod**              (I05) | :soon:                                         | 
| **V/UHF Mod**                 (I06) | :soon:                                         | 


## Debug/Exper. Settings (INF, `MENU_DEBUG`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **TX Audio via USB**          (028) | :soon:                                         | 


[//]: # ( EOFILE                                                                       )

