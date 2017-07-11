<div align="center"><h1><b>The UHSDR Project</b></h1></div>

UHSDR stands for **U**niversal**H**am**S**oftware **D**efined **R**adio.
The project provides  the firmware to run standalone HAM transceivers.
It currently supports STM32F4 and STM32F7 MCU based QSD designs. The
current firmware provides receive and transmit for various analog modes
(SSB,AM, FM) including modes like Synchronous AM, plus digital modes
like FreeDV and of course CW (including Iambic/Ultimatic keyer). An USB
CAT and audio interface is part of the firmware as well. The bootloader,
which is another part of the project, permits to update the firmware and
bootloader using various means like USB disk or USB cable.

The base of the firmware was started by Chris, M0NKA, and Clint, KA7OEI
as part of the [mcHF](http://www.m0nka.co.uk/) project. Thanks to the
Open Source Approach (now with the GPLv3 license) the mcHF firmware has
been extended in this project with new functionality and also with
support for use on different transceiver hardware.

The intent of this project is to give full support for mcHF [(and all
other known and listed hardware platforms)](https://github.com/df8oe/UHSDR/wiki/Supported-Hardware) as long as there are
contributors willing to support the given hardware.

So this is the best place to start with up-2-date developed firmware and
bootloader for mcHF.

If you only want binaries, you can find them for stable releases and
pre-releases in "github releases".

Up-to-date binaries of actual development are available as "daily
snapshots". Explicit Versioning is only done in RELEASES. For
identifying daily snapshots you
must use build time stamp which is shown in splash screen and the System
Info menu.

If you want to see the recent progress of the project, have a look [at
the commits here](https://github.com/df8oe/mchf-github/commits/active-devel).


All this is bundled in [startup page](http://df8oe.github.io/UHSDR/).

Have fun - Open-Source opens possibilities!

DF8OE, Andreas<br/>
DB4PLE, Danilo<br/>
DD4WH, Frank<br/>
and the complete UHSDR community
