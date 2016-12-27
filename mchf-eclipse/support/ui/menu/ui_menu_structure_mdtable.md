
[//]: # (                                                                              )
[//]: # ( WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !                         )
[//]: # (                                                                              )
[//]: # ( generated from  <${BUILD_ID}>  at  2016-12-27T18:50:03  by  ui_menu_structure_mdtable.py )
[//]: # (                                                                              )
[//]: # ( mcHF SDR TRX - Menu Structure Diagram as MarkDown-Table                      )
[//]: # (                                                                              )
[//]: # ( see <https://help.github.com/categories/writing-on-github/>                  )
[//]: # (                                                                              )
    

## Standard Menu (STD, `MENU_BASE`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **LSB/USB Auto Select**       (031) | If enabled, the appropriate SSB mode is chosen as default for bands by its frequency. | 
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
| **AGC Mode**                  (050) | Automatic Gain Control Mode setting. You may select preconfigured settings (SLOW,MED,FAST), define settings yourself (CUSTOM) or use MANUAL (no AGC\, use RFG to control gain | 
| **Custom AGC (+=Slower)**     (052) | If AGC is set to CUSTOM\, this controls the speed setting of AGC | 
| **RX Codec Gain**             (053) | Sets the Codec IQ signal gain. Higher values represent higher gain. If set to AUTO the mcHF controls the gain so that the dynamic range is used best. | 
| **RX/TX Freq Xlate**          (055) | Controls offset of the receiver IQ signal base frequency from the dial frequency. Use of +/-12Khz is recommended. Switching it to OFF is not recommended as it disables certain features. | 
| **Mic/Line Select**           (060) | Select used signal input for transmit (except in CW). Also changeable via long press on M3 | 
| **Mic Input Gain**            (061) | Microphone gain. Also changeable via Encoder 3 if Microphone is selected as Input | 
| **Line Input Gain**           (062) | LineIn gain. Also changeable via Encoder 3 if LineIn Left (L>L) or LineIn Right (L>R) is selected as Input | 
| **TX Audio Compress**         (065) | Control the TX audio compressor. Higher values == more compression. Set to CUSTOM to set user defined compression parameters. See below. Also changeable via Encoder 1 (CMP). | 
| **TX ALC Release Time**       (063) | If Audio Compressor Config is set to CUSTOM\, sets the value of the Audio Compressor Release time. Otherwise shows predefined value of selected compression level. | 
| **TX ALC Input Gain**         (064) | If Audio Compressor Config is set to CUSTOM\, sets the value of the ALC Input Gain. Otherwise shows predefined value of selected compression level. | 
| **RX NB Setting**             (054) | :soon:                                         | 
| **DSP NR Strength**           (010) | :soon:                                         | 
| **TCXO Off/On/Stop**          (090) | :soon:                                         | 
| **TCXO Temp. (C/F)**          (091) | :soon:                                         | 
| **Backup Config**             (197) | :soon:                                         | 
| **Restore Config**            (198) | :soon:                                         | 
| **Restart Codec**             (198) | :soon:                                         | 


## Configuration Menu (CON, `MENU_CONF`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **Save Out-Of-Band Freq.**    (232) | :soon:                                         | 
| **TX on Out-Of-Band Freq.**   (207) | :soon:                                         | 
| **Transmit Disable**          (203) | :soon:                                         | 
| **Menu SW on TX disable**     (204) | :soon:                                         | 
| **TX Mute LineOut**           (205) | :soon:                                         | 
| **TX Initial Muting Time**    (206) | :soon:                                         | 
| **Max Volume**                (210) | :soon:                                         | 
| **Max RX Gain (0=Max)**       (211) | :soon:                                         | 
| **Key Beep**                  (212) | :soon:                                         | 
| **Beep Frequency**            (213) | :soon:                                         | 
| **Beep Volume**               (214) | :soon:                                         | 
| **CAT Mode**                  (220) | :soon:                                         | 
| **CAT Running In Sandbox**    (530) | :soon:                                         | 
| **CAT-IQ-FREQ-XLAT**          (400) | :soon:                                         | 
| **XVTR Offs/Mult**            (280) | :soon:                                         | 
| **XVTR Offset**               (281) | :soon:                                         | 
| **Step Button Swap**          (201) | :soon:                                         | 
| **Band+/- Button Swap**       (202) | :soon:                                         | 
| **Reverse Touchscreen**       (122) | :soon:                                         | 
| **Voltmeter Cal.**            (208) | :soon:                                         | 
| **Freq. Calibrate**           (230) | :soon:                                         | 
| **Pwr. Det. Null**            (271) | :soon:                                         | 
| **FWD/REV ADC Swap.**         (276) | :soon:                                         | 
| **RX IQ Balance (80m)**       (240) | :soon:                                         | 
| **RX IQ Phase   (80m)**       (241) | :soon:                                         | 
| **RX IQ Balance (10m)**       (242) | :soon:                                         | 
| **RX IQ Phase   (10m)**       (243) | :soon:                                         | 
| **TX IQ Balance (80m)**       (250) | :soon:                                         | 
| **TX IQ Phase   (80m)**       (251) | :soon:                                         | 
| **TX IQ Balance (10m)**       (252) | :soon:                                         | 
| **TX IQ Phase   (10m)**       (253) | :soon:                                         | 
| **DSP NR BufLen**             (310) | :soon:                                         | 
| **DSP NR FFT NumTaps**        (311) | :soon:                                         | 
| **DSP NR Post-AGC**           (312) | :soon:                                         | 
| **DSP Notch ConvRate**        (313) | :soon:                                         | 
| **DSP Notch BufLen**          (314) | :soon:                                         | 
| **DSP Notch FFTNumTap**       (315) | :soon:                                         | 
| **NB AGC T/C (<=Slow)**       (320) | :soon:                                         | 
| **Reset Config EEPROM**       (341) | :soon:                                         | 


## Display Menu (DIS, `MENU_DISPLAY`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **LCD Auto Blank**            (090) | :soon:                                         | 
| **Step Size Marker**          (091) | :soon:                                         | 
| **Filter BW Display**         (092) | :soon:                                         | 
| **Spectrum Type**             (109) | :soon:                                         | 
| **Spectrum Magnify**          (105) | :soon:                                         | 
| **Spectrum Size**             (117) | :soon:                                         | 
| **Spectrum Filter**           (101) | :soon:                                         | 
| **Spec FreqScaleClr**         (104) | :soon:                                         | 
| **Spectrum LineClr**          (108) | :soon:                                         | 
| **Spectrum FFT**              (340) | :soon:                                         | 
| **Scope Light**               ( 99) | :soon:                                         | 
| **Scope 1/Speed**             (100) | :soon:                                         | 
| **Scope AGC Adj.**            (106) | :soon:                                         | 
| **Scope Trace Colour**        (102) | :soon:                                         | 
| **Scope Grid Colour**         (103) | :soon:                                         | 
| **Scope Div.**                (107) | :soon:                                         | 
| **Scope NoSig Adj.**          (115) | :soon:                                         | 
| **Wfall 1/Speed**             (114) | :soon:                                         | 
| **Wfall Colours**             (110) | :soon:                                         | 
| **Wfall Step Size**           (111) | :soon:                                         | 
| **Wfall Brightness**          (112) | :soon:                                         | 
| **Wfall Contrast**            (113) | :soon:                                         | 
| **Wfall NoSig Adj.**          (116) | :soon:                                         | 
| **S-Meter**                   (121) | :soon:                                         | 
| **Meter Colour Up**           (122) | :soon:                                         | 
| **Meter Colour Down**         (123) | :soon:                                         | 
| **dBm display**               (120) | Enable an additional numeric display of the RX signal strength | 
| **S-Meter**                   (121) | Select the S-Meter measurement style           | 


## CW Mode Settings (CW , `MENU_CW`)

| LABEL                         ( ID) | DESCRIPTION                                    | 
| ----------------------------------- | ---------------------------------------------- | 
| **CW Keyer Mode**             (070) | :soon:                                         | 
| **CW Keyer Speed**            (071) | :soon:                                         | 
| **CW Sidetone Gain**          (072) | :soon:                                         | 
| **CW Side/Off Freq**          (073) | :soon:                                         | 
| **CW Paddle Reverse**         (074) | :soon:                                         | 
| **CW TX->RX Delay**           (075) | :soon:                                         | 
| **CW Freq. Offset**           (076) | :soon:                                         | 


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
| **Tune Power Level**          (P00) | :soon:                                         | 
| **Tune Tone (SSB)**           (P99) | :soon:                                         | 
| **Reduce Power on Low Bands** (P0A) | :soon:                                         | 
| **CW PA Bias (If >0 )**       (260) | :soon:                                         | 
| **PA Bias**                   (261) | :soon:                                         | 
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

