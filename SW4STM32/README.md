Introduction 

This directory contains SW4STM32 and CubeIDE compatible project that allows easy import to any
of those tools

Import into SW4STM32 IDE

- In the File menu, select 'Import...'
- Choose 'Existing Projects into Workspace' and click Next
- Select root directory: "root\SW4STM32\mchf-uhsdr", do not tick any of the options
- Click Finish

Import into STM32CubeIDE

- In the File menu, select 'Import...'
- Chose 'Import ac6 System Workbench for STM32 Project' and click Next
- Select root directory: "root\SW4STM32\mchf-uhsdr", 'Detect and configure project natures' selected
- Click Finish

Troubleshooting

- Fix for functions with wrong inline definition - use optimization (O1 at least), more info:
	https://stackoverflow.com/questions/41218006/gcc-fails-to-inline-functions-without-o2

For any problems compiling under windows please open an issue report on the project GitHub page.

Thank you!

Krassi Atanassov, M0NKA

