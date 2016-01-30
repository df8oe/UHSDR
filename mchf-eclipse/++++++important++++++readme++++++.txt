If you get a "white screen" (or want to prevent this before update):

- resistors R30...R32 must be removed if you use LCD in parallel mode.


If you do an upgrade from firmware version < 219.26.15:

Please write down all settings in configuration menu **BEFORE** you
do the upgrade!!! Because of new bands are added configuration has
changed and is incompatible. After upgrade you have scrambled
configuration and before mcHF can be used you *MUST* proceed as following:

1) Press & hold Buttons F1-F3-F5 (MENU-SPLIT-TUNE)..
2) Switch on mcHF with power button while holding other buttons.
3) After a few seconds normal screen appears and again after 1...2 seconds
   a warning screen appears. *NOW* you can release the buttons.
4) When you power down mcHF using the power button default confugration
   is used at next startup. If you use serial EEPROM you *MUST* first
   select menu #197. This removes old (incompatible) configuration data
   which is backed up in virtual EEPROM. Now you can/must enter your
   configuration data you have written down.


For newly available hardware-modifications (serial EEPROM use, touchscreen
use etc.) please view history.txt

vy 73
Andreas, DF8OE