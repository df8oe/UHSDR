# DF8OE Bootloader Usage


The DF8OE bootloader is responsible for starting the firmware of the mcHF and is also able to flash new firmware from a USB drive into the mcHF and/or to dump the current firmware on a USB drive connected to the (big) USB-A socket. 

### Installing The Bootloader

The procedure to install the DF8OE in its DFU form (bootloader.dfu) is the same as for the M0NKA bootloader (using DFUSe). See bootloader\_install.odt or bootloader\_install.pdf 
If you own a ST-Link, you can simply flash the bootloader.bin/bootloader.hex/bootloader.elf using your favorite ST flash tool at adress 0x08000000.

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
|5		|STM32F4 flash programming error		|
|6		|STM32F4 flash erase error				|
|7		|STM32F4 flash write protected			|


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