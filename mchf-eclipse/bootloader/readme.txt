You need to add 5V supply to connector via schottky-diode
and (poly)fuse 0.5A to make usb-key work. You can add
these components on ui-board. 5V pin of usb connector is
located nearest to the bottom side of the ui pcb. Solder
anode of diode to +5V and cathode via fuse to described
usb pin. It is a good idea to add tantalum 100uF/6V on
5V-Pin of USB for cleaning power supply of stick.

You need usb-key with filesystem FAT/FAT32.

For firmware-upgrade you must plug the usb-key to mchf
otg (mikto-usb).

1) Only saving old firmware to key:
Press BAND- button and hold it. Then press power button and
release both buttons after a second.
LCD backlight will start and green LED lights. When backlight
goes dark process is finished and you can find backup of old
firmware in root directroy of the key mchfold.bin. Momentary
file is as big as flash memory of STM32F4 because of no test
how big firmware really is is done.

2) Saving old firmware and flashing new firmware
The binary of the new firmware you want to flash must have
name mchf.bin and must be in root directory of the key.
Press BAND- button and hold it. Then press power button and
release it after a second. Release BAND- button a second later.
LCD backlight will start and green LED lights. Then red LED
lights what indicates write process starts. When all is finished
backlight goes dark.

There are many codes implemented of blinking backlight in
combination with LEDs. The identifying is not ready worked out yet
so it is important if backlight flashes errors occured like
no usb-key connected, usb-key with wrong filesystem, no
space left on key for backup the firmware, no file mchf.bin
found while trying to upgrade fw and so on. Many codes are already
described in led-blink-codes.txt but not all of them.

If process ends without error (== black backlight) you can directly
boot either by pulling USB-key or pushing BAND- again.

If process ends in error (==backlight blinking) you can turn off
mchf by pressing power button.

2-be-continued

DF8OE, Andreas						10/02/2015