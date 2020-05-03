Introduction 

This directory contains SW4STM32 compatible project that allows easy import and building the source
on Windows systems

Import into SW4STM32 IDE

- In the File menu, select 'Import...'
- Choose 'Existing Projects into Workspace' and click Next
- Select root directory: "root\SW4STM32\mchf-uhsdr", do not tick any of the options
- Click Finish

Import into STM32CubeIDE

- not possible ATM

Troubleshooting

- Fix for functions with wrong inline definition - use optimization (O1 at least), more info:
	https://stackoverflow.com/questions/41218006/gcc-fails-to-inline-functions-without-o2



