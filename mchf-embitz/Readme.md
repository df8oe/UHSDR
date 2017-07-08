# Building uhsdr firmware with EmBitz

This document describes how to build the uhsdr firmware using the EmBitz IDE on windows.

### What is EmBitz

[EmBitz](https://www.embitz.org) (formerly Em::Blocks) is a powerful C/C++ IDE for embedded software development on Windows. It is a free IDE built as a derivative of [Code::Block](http://www.codeblocks.org/) but specifically tailored for embedded software development. It comes with a bundled GNU ARM compiler 5.4 with different optimized libraries. See [features](https://www.embitz.org/feature-list/) at [Embitz.org](https://www.embitz.org).

It is a light and fast IDE compared to Eclipse but with a lot of handy and powerfull features for embedded development. See the discussion on inexpensive ARM development tools on [EEVBLOG electronics Community Forum](http://www.eevblog.com/forum/microcontrollers/inexpensive-arm-development-tools/).

### Prerequisites

  * EmBitz works on windows 7 and up.
  * Optional : Building the uhsdr firmware with the bundled GNU ARM toolchain 5.4 is fine, but you can install independent
[GNU ARM Embedded Toolchains](https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads) if you prefer another toolchain version.
  * Optional : EmBitz doesn't include any Git interface or plugin, an external Git tool is needed to publish your modifications on GitHub. [GitHub Desktop](https://desktop.github.com/) or [SmartGit](https://www.syntevo.com/smartgit/) are very easy to use. Read the explanations by DF8OE in the [CONTRIBUTING.md](https://github.com/df8oe/uhsdr/blob/active-devel/CONTRIBUTING.md) file.

### Building

  * Open the *uhsdr-embitz.eworkspace* file into EmBitz (File/Open/Workspace files menu)
  * Change the *ar* toolchain archiver to *arm-none-eabi-gcc-ar.exe* (Settings/Tools/Global Compiler Settings/Toolchain Executables), this is to allow link time optimization of libraries
  * Select the **Release** target of all projects and build them.
  * The ready to flash *fw-xxx.bin* file is in mchf-embitz/bin/Release directory.

### Debugging

EmBitz uses own tailor made GDB debuggers, allowing some features such as live variables and "hot" connect to running targets without stopping. The integrated ARM GDB supports the inexpensive ST-LINK V2 clones from Ebay.

  * Select the **Debug** target of all projects and build them.
  * Press and hold the **POWER** button of the radio and start the debug session with the F8 key. The GDB is started and automatically flashes the firmware onto the CPU. Don't release the **POWER** button until the main firmware is run to the point where the GPIO pin controlling the power (POWERDOWN signal) is set.
  ** Once powered on, the firmware could be reseted or halted and breakpoints could be set without loosing the power.

Have fun - Open-Source opens possibilities!

F4FHH, Nicolas
