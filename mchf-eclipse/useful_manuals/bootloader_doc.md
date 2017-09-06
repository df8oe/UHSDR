# DF8OE Bootloader Usage (Bootloader 3.3.0 and newer)


The DF8OE bootloader is responsible for starting the firmware of the TRX. It is also able to flash new firmware and bootloader.
The bootloader supports two different ways to install new software on the TRX, via USB drive or via DFU Upload. 
In most cases, the most convenient way is to use a USB drive to install new firmware with the USB drive connected to the (big) USB-A socket. 
To install a new bootloader (or new firmware) the alternate method is to run a DFU upload using a PC connected via USB cable to the small (Mini USB connector) and an appropriate DFU upload tool (e.g. Windows: DFUSe, Linux: dfu-util). 

The bootloader uses the two leds and the display to interact with the user. If the display is connected correctly, you will see instructions and messages on screen, otherwise you will have to watch the leds.

### Documentation for older bootloaders

The documentation is stored with the source code in the corresponding release archives ( in mchf-eclipse/useful\_docs/bootloader_doc.md) or in the github (even for not released bootloaders).  

### Installing The Bootloader

The procedure to install the DF8OE bootloader in its DFU form bl-<trx_id>.dfu (e.g. bl-mchf.dfu for the mcHF ) is the same as for the M0NKA bootloader (using DFUSe). See bootloader\_install.odt or bootloader\_install.pdf 
If you own a ST-Link, you can simply flash the bl-<trx_id>.bin/bl-<trx_id>.hex/bl-<trx_id>.elf using your favorite ST flash tool at address 0x08000000.


## Firmware Update Procedures Using a USB Drive

The USB drive approach can be used to upload new firmware without any need for special software. 

### Prerequisites 

1. You need the DF8OE bootloader to be installed (once) on your TRX
1. You need USB drive formatted with a single partition with the filesystem FAT or FAT32 (not exFAT, NTFS or anything else!). Normal USB pen drives formatted under Windows or Linux fulfill this criteria.


For the firmware-upgrade you must plug the USB-key into the TRX
big USB-A plug.

### Flashing New Firmware And Saving Old Firmware 

1. Place the binary file of the new firmware with the name "fw-<trx_id>.bin" (e.g. fw-mchf.bin for the mcHF) into the root directory of the USB drive.
1 Turn off the TRX and connect the USB drive.
1. Press BAND- button and hold it. 
1. Then press the power button and release it after a second once you see the LCD screen light up. 
1. Release BAND- button once you see the green LED light up. Now the dumping of the whole firmware flash into MCHFOLD.BIN starts. The resulting file will have a size of up to 1984kByte, depending on the actual flash size of the MCU (2M = 1984, 1M = 960, 512k = 448). 
1. Then the red LED lights up too, indicating the write process has started. 
1. When the update has finished, the backlight goes dark, both red and green LEDs remain steadily on. If not, and the red LED starts flashing, see the error codes section below.
1. Remove the USB drive and press BAND- to reboot into the new firmware, press Power to power off.

### Only Saving Old Firmware
1. Turn off the TRX and connect the USB drive.
1. Press BAND- button and hold it. 
1. Then press power button.
1. Release both buttons once you see the LCD screen light up. 
1. Release BAND- button once you see the green LED light up. Now the dumping of the firmware into fw-<trx_id>.old (e.g. fw-mchf.old for the mcHF) starts.
1. When finished, the backlight goes dark and the  green LED remains steadily on. If not, and the red LED starts flashing, see the error codes section below.
1. Remove the USB drive and press BAND- to reboot, press Power to power off


### Error Handling and Codes
During normal operation of the bootloader (by just pressing the power button), a flashing backlight indicates 
that the bootloader did not identify a valid firmware in flash. This can happen either because you 
never flashed one, or the flashed binary was not a valid firmware binary for your TRX, or you erased the flash memory (e.g. 
with an external debugger tool like the ST-Link), or in the worst case, you could have a defect in the processor 
flash memory.

If during firmware update mode the LCD backlight remains on with a slowly blinking green LED, your USB drive was not detected.
You can remove the USB drive and try to plug it in again, or you can try another key. Remember to follow the steps in "Flashing New Firmware and Saving Old Firmware" as shown above when plugging in the drive if you want to upgrade your firmware. Do not just press BAND- button as only the old firmware is written to your drive.

If in firmware update mode (start with Band- pressed) right after starting the TRX you see a black screen with one or more LEDs turned on or blinking, you don't have the DF8OE bootloader but most likely the mcHF M0NKA bootloader installed. See his pages for instructions how to use it or replace the bootloader with this one. The M0NKA bootloader needs a Windows software to flash the image and it uses the small Mini-USB port.

If everything else seems to be ok, your USB key is incompatible. Try another key. 
Keys manufactured by "SanDisk" are widely distributed, easy to purchase and seem 
to work well. 

For the firmware update mode there are a number of error conditions reported through a visual code.
If firmware reading/writing process ends in an error this is shown by turning off the LCD backlight then
turning the green LED off, blinking the red LED in bursts, then turning the LCD backlight on and off again (green LED remains off) and repeating the red LED flash bursts. This will stop if you press Power for a little while, which turns the TRX off.
 

The red LED is flashing in bursts of:

|Flashes|  Error                            	|
|-------|---------------------------------------|
|1		|USB problems							|
|2 		|fw-<trix_id>.bin not found on the USB drive		|
|3		|flash memory is too small for mchf.bin	|
|4		|problems writing fw-<trix_id>.old to the USB drive	|
|5		|problems reading fw-<trix_id>.bin from the USB drive	|
|6		|STM32F4 flash programming error		|
|7		|STM32F4 flash erase error				|
|8		|STM32F4 flash write protected			|


## Firmware and Bootload Update Procedures Using an USB Cable and DFU Upload

Both firmware and bootloader can be updated using the DFU Upload method. You will need a PC with proper STM DFU software installed. On Windows, the DfuSE tool from STM will do the job, on Linux install the dfu-util package.

### Prerequisites 

1. You need the DF8OE bootloader to be installed (once) on your TRX
1. Install the DFU software on your PC including the provided driver if necessary. 
1. Connect PC and TRX using the small USB connector with a Mini-USB cable.
1. Get the appropriate DFU file (__bl-<trx_id>.dfu__ or __fw-<trx_id>.dfu__)

### Starting The TRX in DFU mode

1. Turn the TRX off
1. Press and hold Band+
1. Press and hold Power
1. After two seconds you can release the Band+ button but keep the Power button pressed permanently. Now maybe you will see the LCD white backlight, which stays on - That is depending on the charge of LCD you have fitted and on the method you are driving it (SPI or parallel). It doesn't matter if it stays at text or goes white - both is ok. You will not see any LED flashing etc. All communication is only through the USB bus. 
1. Your PC should now recognize a new USB device "STM BOOTLOADER", manufacturer is "STMicroelectronics"
1. Keep the power button pressed until the very end of the instructions given below. 

#### Windows DfuSE Instructions

1. Start the DFU applicaton (if not already started) and upload as instructed. For the correct use of DfuSE see the bootloader_install.pdf. You don't have to do the first steps (since you started in the DFU using the bootloader) but then you need to follow the steps (A) to (D).
1. (A) Make sure you see "STM Device in DFU Mode"
1. (B) Select the file to upload using "Choose". 
1. (C) Select checkbox "Verify after Download"
1. (D) Press "Upgrade". __DO NOT USE__ "Upload". 
1. After successful flashing you can let go of the Power button, not earlier. If you interrupt power during the upload of a new firmware.bin, no problem. However, if you do this during the upload of a new bootloader, you may temporarily brick your TRX and you have to install the bootloader using the P6 jumper method described in the aformentioned bootloader_install.pdf.

 
#### Linux dfu-util

1. "dfu-util -D fw-mchf.dfu -a 0" or "dfu-util -D bl-mchf.dfu -a 0"
After successful flashing you can let go of the Power button, not earlier. If you interrupt power during the upload of a new firmware.bin, no problem. However, if you do this during the upload of a new bootloader, you may temporarily brick your TRX and you have to install the bootloader using the P6 jumper method described in the aformentioned bootloader_install.pdf.

  
    


### History

06/19/2017 [DB4PLE]
Changed to use the new file naming scheme 

03/14/2017 [DB4PLE]
Added DFU Update support via USB using Band+ button

03/12/2017 [DB4PLE]
Ported to HAL, now supports all tested USB pen drives (also previously not working ones!)
Some changes to the error led codes (simplified the coding to allow easier communication)

08/19/2016
activated big USB-A plug instead of mini plug (no modification needed
from now on)

10/02/2015
minor bugfixes.
added possibility to turn mchf off with pushing power button if it
ends in blinking backlight (error stage) or successfully finished
process.

09/29/2015
first release of bootloader which allows fw-upgrades via USB-key
connected to otg-plug (mini-USB). You need to add 5V supply to
connector via schottky-diode and (poly)fuse 0.5A to make
USB-sticks work.
I took STM AN3990 as base and adapted pin layout to mchf also
new blink codes due to mchf has less LEDS than DISCO-board where
AN3990 is written.

DF8OE, Andreas						10/19/2016
