You need USB-key with filesystem FAT/FAT32.

For firmware-upgrade you must plug the USB-key to mchf
big USB plug.

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

If process ends in error (==backlight blinking) blink codes and state
of LEDs helps to identify the reason for malfunction. Please report
errors to GitHub issues describing how you get this error and the state
of backlight and LEDs.
You can turn off mchf by pressing power button or restart to transceiver
by pressing BANDM.

If backlight is flashing and everything seems to be ok, your USB key
is incompatible. This is very rare with older keys (manufactured
before 2012). Newer keys and keys with larger amount of memory
(> 8GB) do have more problems.
If you have problems try another key. Keys which are manufactured
by "SanDisk" are widely distributed and easy to purchase.

2-be-continued

DF8OE, Andreas						10/19/2016