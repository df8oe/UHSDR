# DF8OE Bootloader Usage


The DF8OE bootloader is responsible for starting the firmware of the mcHF. it is also able to flash new firmware and bootloader.
The bootloader supports two ways to install new software on the mcHF, via USB drive or via DFU Upload. 
In most cases, the most convenient way is to use a USB drive to install new firmware using the a USB drive connected to the (big) USB-A socket. 
To install a new bootloader (or new firmware) the alternate method is to run a DFU upload using a PC connected via USB cable to the small (Mini USB connector) and an appropriate DFU upload tool (e.g. Windows: DFUSe, Linux: dfu-util). 


### Installing The Bootloader

The procedure to install the DF8OE in its DFU form (bootloader.dfu) is the same as for the M0NKA bootloader (using DFUSe). See bootloader\_install.odt or bootloader\_install.pdf 
If you own a ST-Link, you can simply flash the bootloader.bin/bootloader.hex/bootloader.elf using your favorite ST flash tool at adress 0x08000000.

## Firmware Update Procedures Using a USB Drive

The USB drive approach can be used to upload new firmware with any need for special software. 

### Prerequisites 

1. You need the DF8OE bootloader to be installed (once) on your mcHF
1. You need USB drive formatted with a single partition with the filesystem FAT or FAT32 (not exFAT, NTFS or anything else!). Normal USB pen drives formatted under Windows or Linux fulfill this criteria.


For firmware-upgrade you must plug the USB-key to mchf
big USB-A plug.

### Flashing New Firmware And Saving Old Firmware 

1. Place binary of the new firmware with the name "mchf.bin" on in the root directory of the drive.
1 turn off mcHF and connect drive.
1. Press BAND- button and hold it. 
1. Then press power button and release it after a second once you see the LCD screen light up. 
1. Release BAND- button once you see the green LED light up. Now the dumping of the whole firmware flash into MCHFOLD.BIN starts. Resulting file will have a size of 960kByte, independent of the actually flash size. 
1. Then the red LED lights up too, indicating the write process start. 
1. When finished, backlight goes dark, both red and green led remain steadily on. If not, and red led starts flashing, see error codes section below.
1. Remove USB drive or press BAND- to reboot into new firmware, press Power to power off

### Only Saving Old Firmware
1. Turn off mcHF and connect drive.
1. Press BAND- button and hold it. 
1. Then press power button.
1. Release both buttons once you see the LCD screen light up. 
1. Release BAND- button once you see the green LED light up. Now the dumping of the firmware into MCHFOLD.BIN starts.
1. When finished, backlight goes dark, green led remains steadily on. If not, and red led starts flashing, see error codes section below.
1. Remove USB drive or press BAND- to reboot, press Power to power off


### Error Handling and Codes
During normal operation of the bootloader (just pressing power), a flashing backlight indicates 
that the bootloader did not identify a valid firmware in flash. This can happen either because you 
never flashed one, the flashed binary was not a valid mcHF binary, you erase the flash memory (e.g. 
with an external debugger tool like the ST-Link) or in worst case, you have a defect in the processor 
flash memory.

If in firmware update mode backlight remains on with a slowly blining green led in firmware update mode, your USB drive was not detected.
You can remove the device and try to plug it in again, or you can try another key. Remember to press BAND- before pluging in the drive if you want to upgrade your firmware. Otherwise only the old firmware is written to your drive.

If in firmware update mode (start with Band- pressed) right after starting the mcHF you see a black screen with one or more leds turned on or blinking, you don't have the DF8OE bootloader but most likely the mcHF M0NKA bootloader. See his pages for instructions how to use it or replace the bootloader with this one. The M0NKA bootloader needs a Windows software to flash the image and is usin the small Mini-USB port.

If everything else seems to be ok, your USB key is incompatible. Try another key. 
Keys manufacturedcby "SanDisk" are widely distributed, easy to purchase and seem 
to work well. 

For the firmware update mode there are a number of error conditions reported through a visual code.
If firmware reading/writing process ends in an error this is shown by turning the backlight off, 
turning the green led off and blinking the red led in burst, turning the backlight on and off (green remains off) and repeating the red led flash burst. This stops only if you press Power for a little while, which turns the mcHF off.
 

The red led is flashing in bursts of:

|Flashes|  Error                            	|
|-------|---------------------------------------|
|1		|USB problems							|
|2 		|mchf.bin not found on USB drive		|
|3		|flash memory too small for mchf.bin	|
|4		|problems writing MCHFOLD.BIN to drive	|
|5		|problems reading mchf.bin from disk	|
|6		|STM32F4 flash programming error		|
|7		|STM32F4 flash erase error				|
|8		|STM32F4 flash write protected			|


## Firmware and Bootload Update Procedures Using an USB Cable and DFU Upload

Both firmware and bootloader can be updated using the DFU Upload method. You will need a PC with proper STM DFU software installed. On Windows, the DfuSE tool from STM will do the job, on Linux install the dfu-util package.

### Prerequisites 

1. You need the DF8OE bootloader to be installed (once) on your mcHF
1. Install the DFU software on your PC including the provided driver if necessary. 
1. Connect PC and mcHF (small USB connector) using a Mini-USB cable.
1. Get the appropriate DFU file (__bootloader.dfu__ or __firmware.dfu__)

### Starting The mcHF in DFU mode

1. Turn mcHF off
1. Press and hold Band+
1. Press and hold Power
1. After two second you can release the Band+ button but keep the Power button pressed permanently. You will see the white backlight, which stays on. You will not see any led flashing etc. All communication is done using the USB bus only. 
1. Your PC should now recognize a new USB device "STM BOOTLOADER", manufacturer is "STMicroelectronics"
1. Keep the power button pressed until the very end of the instructions given below. 

#### Windows DfuSE Instructions

1. Start the DFU applicaton (if not already started) and upload as instructed. For the correct use of DfuSE see the bootloader_install.pdf. You don't have to do the first steps (since you started in the DFU using the bootloader) but then you need to follow the steps (A) to (D).
1. (A) Make sure you see "STM Device in DFU Mode"
1. (B) Select the file to upload using "Choose". 
1. (C) Select checkbox "Verify after Download"
1. (D) Press "Upgrade". __DO NOT USE__ "Upload". 
1. After successful flashing you can let go of the Power button, not earlier. If you interrupt power during the upload of a new firmware.bin, no problem. However, if you do this during the upload of a new bootloader, you may temporarily brick your mcHF and you have to install the bootloader using the P6 jumper method described in the aformentioned bootloader_install.pdf.

 
#### Linux dfu-util

1. "dfu-util -D mchf.dfu -a 0" or "dfu-util -D bootloader.dfu -a 0"
After successful flashing you can let go of the Power button, not earlier. If you interrupt power during the upload of a new firmware.bin, no problem. However, if you do this during the upload of a new bootloader, you may temporarily brick your mcHF and you have to install the bootloader using the P6 jumper method described in the aformentioned bootloader_install.pdf.

  
    


### History

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
AN3990 is written for.

DF8OE, Andreas						10/19/2016