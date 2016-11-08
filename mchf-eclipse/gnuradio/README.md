# mcHF Gnuradio Simulation
This is a simplified simulation of the mcHF signal paths.

Currently only the RX Path is implemented, and even this is not complete.

## Contributing

Feel free to add more elements to the mchf Simulation. Please try to be as close as possible 
to the real implementation, since the purpose is to be able to simulate the mcHF not to build a 
new SDR in gnuradio. Of course experiments and propose changes can be demonstrated using the simulation.

## Running it
You will need a recent gnuradio-companion installed (gnuradio 3.7)

You will need to install the hierarchical blocks in the subblocks folder.
This can be done by opening them in the gnuradio-companion and pressing 
the "Generate" button (the one left to the "Run" button in the toolbar)
Since the blocks have dependencies, you may have to generated other blocks 
restart gnuradio-companion before they will generate without error.

You may also use the commandline grcc utility, which you just need to run with the filename as parameter.
Same rule applies, generation will fail if the dependencies have not been generated before.

Once that is done, you can open mchf_main.grc
You will have to adjust the file paths in 


The simulation expects 48Khz IQ sounddata as input. This can be produced by the mcHF itself 
using the DIQ mode and a simple audio recorder. Of course any other IQ Recording of 16bit IQ data 
such as one made with HDSDR will work too. The output is saved in a wav file too. The default file paths are

IQ Input: /tmp/mchfiq.wav
Audio Output: /tmp/mchfaudio.wav 

The input file name is locate in mchf_rf_frontend.grc. Remember to regenerate after changing the filename.
The output file name is in the mchf_main.grc file. 

