You need to add 5V supply to connector via schottky-diode
and (poly)fuse 0.5A to make usb-sticks work. You can add
these components on ui-board. 5V pin of usb connector is
located nearest to the bottom side of the ui pcb. Solder
anode of diode to +5V and cathode via fuse to described
usb pin.

You need usb-stick with filesystem FAT/FAT32.

For firmware-upgrade you must plug the usb stick to mchf
otg (mikto-usb).

1) Only saving old firmware to stick:
Press BAND- button and hold it. Then press power button and
release both buttons after a second.
LCD backlight will start and green LED lights. When backlight
goes dark process is finished and you can find backup of old
firmware in root directroy of the stick mchfold.bin.

2) Saving old firmware and flashing new firmware
The binary of the new firmware you want to flash with the
name mchf.bin must be in root directory of the stick.
Press BAND- button and hold it. Then press power button and
release it after a second. Release BAND- button a second later.
LCD backlight will start and green LED lights. Then red LED
lights what indicates write process starts. When all is finished
backlight goes dark.

There are many codes implemented of blinking backlight in
combination with LEDs. The identifying is not ready worked out yet
so it is important if backlight flashes errors occured like
no usb-stick connected, usb-stick with wrong filesystem, no
space left on stick for backup the firmware, no file mchf.bin
found while trying to upgrade fw and so on.

2-be-continued

DF8OE, Andreas