<div align="center"><h1><b>The UHSDR Project</b></h1></div>

UHSDR stands for **U**niversal**H**am**S**oftware**D**efined**R**adio.
The project provides  the firmware to run standalone HAM transceivers.
It currently supports STM32F4 and STM32F7 MCU based QSD designs. The
current firmware provides receive and transmit for various analog modes
(SSB,AM, FM) including modes like Synchronous AM, plus digital modes
like FreeDV, RTTY  and of course CW (including Iambic/Ultimatic keyer).
An USB CAT and audio interface is part of the firmware as well. The
bootloader, which is another part of the project, permits to update the
firmware and bootloader using various means like USB disk or USB cable.

The base of the firmware was started by Chris, M0NKA, and Clint, KA7OEI
as part of the [mcHF](http://www.m0nka.co.uk/) project. In February 2017
both agreed to change firmware license to GPLv3. Because of this the mcHF
firmware has been extended in this project with new functionality
and also with support for use on different transceiver hardware. For
clarifying different license models on mcHF hardware and mcHF firmware the
name of the firmware changed to UHSDR.

The intent of this project is to give full support for mcHF [(and all
other known and listed hardware platforms)](https://github.com/df8oe/UHSDR/wiki/Supported-Hardware) as long as there are
contributors willing to support the given hardware.

So this is the best place to start with up-2-date developed firmware and
bootloader for mcHF.

Binaries of actual development are available as "bleeding edge builds".
Since 08-19-2017 all binaries which are populated are archived and can
accessed at GitHub startup page (see link a few lines down).

If you want to see the recent progress of the project, have a look [at
the commits here](https://github.com/df8oe/mchf-github/commits/active-devel).

You can find descriptions of menus and operating hints at [our WIKI](https://github.com/df8oe/UHSDR/wiki/)
All this is bundled in [startup page](https://df8oe.github.io/UHSDR/).

Have fun - Open-Source opens possibilities!

M0NKA, Chris<br/>
KA7OEI, Clint<br/>
DF8OE, Andreas<br/>
DB4PLE, Danilo<br/>
DD4WH, Frank<br/>
DL2FW, Michael<br/>
SP9BSL, Slawek<br/>
and the complete UHSDR community
