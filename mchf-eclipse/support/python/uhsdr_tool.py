"""
This module contains experimental code for using the (extend) UHSDR API
and contains a small commandline client for backup and restore of
the UHSDR configuration data from/to a TRX

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
this program. If not, see <http://www.gnu.org/licenses/>.
"""

from __future__ import print_function

__author__ = "DB4PLE"
__copyright__ = "Copyright 2018, UHSDR project"
__credits__ = ["DB4PLE"]
__license__ = "GPLv3"
__status__ = "Prototype"

import sys
import json
import os
import uhsdr

def listUHSDRSerial():
    """
    list serial ports. For new pySerial version we return only the real UHSDR device, otherwise just a list of serial port objects
    
    returns tupel (Boolean,List) True if UHSDR filter was active, False otherwise, second element is actual list of info for com ports. 
    """
    import serial
    import serial.tools.list_ports;
    from distutils.version import StrictVersion    
    #  UHSDR uses USB VID:PID=0483:5732
    retval = False,[]
    
    if StrictVersion(serial.VERSION) >= StrictVersion("3.0"):
        retval = True,[ comport.device  for comport in serial.tools.list_ports.comports() if comport.vid == 0x0483 and comport.pid == 0x5732 ]
    else:
        uhsdr.eprint("pySerial version ",serial.VERSION," is too old for detecting UHSDR device, using full serial port list")
        retval = False,[ vars(comport)  for comport in serial.tools.list_ports.comports()]
    return retval

def backupRestoreApp():
    import serial
    """
    Simple command line tool to backup/restore the configuration from/to TRX
    """
    
    if False and "idlelib" in sys.modules:
        uhsdr.eprint("Running in IDLE GUI, please set parameters directly in script, search for #change ")
        #change parameters here
        sys.argv = [sys.argv[0],'--port','15','-b']
        #no more changes below
        
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-b","--backup", help="backup the UHSDR TRX configuration to file", action="store_true")
    parser.add_argument("-r","--restore", help="restore the UHSDR TRX configuration from file", action="store_true")
    parser.add_argument("-p","--port", help="UHSDR serial port either by number (COM<num> in Windows, Linux /dev/ttyACM<num>) or full device name ( e.g. '/dev/cu.modemABCD', all operating systems )", type=str, default="undefined serial port name")
    parser.add_argument("-f","--file", help="filename to backup to/restore from, if not defined 'uhsdr_config.json' is used", type=str, default="uhsdr_config.json")

    args = parser.parse_args()

    print("This program backups and restores the configuration of UHSDR TRX via USB")

    if args.port == -1:
        print("Use -h to get more information")
        print("")
        isFiltered, uhsdrDeviceList = listUHSDRSerial()
        if uhsdrDeviceList == []:
            uhsdr.eprint("No suitable serial port detected, make sure to connect UHSDR TRX")
        else:
            if isFiltered:
                uhsdr.eprint("List of detected UHSDR TRX")
            else:
                uhsdr.eprint("List of detected serial ports:")
            uhsdr.eprint(uhsdrDeviceList)
            uhsdr.eprint("Please specify valid UHSDR TRX serial port number(!) with -p / --port")
    else:

        try:
            int(args.port);
            comPort= ("COM" if os.name == "nt" else "/dev/ttyACM") + str(args.port) 
        except:
            comPort = args.port

        uhsdr.eprint("Opening serial port ",comPort,":")
        try:
            mySer = serial.Serial(comPort, 38400, timeout=0.500, parity=serial.PARITY_NONE)
        except:
            uhsdr.eprint("... failed")
            raise
        uhsdr.eprint("... okay")    
        myCom = uhsdr.catSerial(mySer)
        myCAT = uhsdr.catCommands(myCom)
        myUHSDR = uhsdr.UhsdrConfig(myCAT)

        uhsdr.eprint("Detecting if UHSDR is connected: ")
        if myUHSDR.isUhsdrConnected():
            uhsdr.eprint("... yes")
            version = myUHSDR.getVersion()
            print("Detected UHSDR Firmware Version", version)
            if args.backup:
                uhsdr.eprint("Reading configuration from UHSDR TRX:")
                ok,data = myUHSDR.configToJson()
                if ok:
                    uhsdr.eprint("... success. Now saving in file:")
                    with open(args.file, 'w') as outfile:
                        try:
                            json.dump(data, outfile, indent=4)
                            uhsdr.eprint("... saved to " + args.file + " file")
                        except:
                            uhsdr.eprint("... failed to save to " + args.file + " file")
                            raise
                        outfile.close()
                else:
                    uhsdr.eprint("... could not read data sucessfully")
            elif args.restore:
                with open(args.file, 'r') as infile:
                    data = json.load(infile)
                    uhsdr.eprint("Sending configuration to UHSDR TRX:")
                    ok,msg = myUHSDR.jsonToConfig(data)
                    if ok:
                        uhsdr.eprint("... restored all data from " + args.file + " file")
                        print("ACTIVATING RESTORED CONFIGURATION: Switch off  TRX WITHOUT saving configuration -> Press Band+ and Power buttons")
                        print("")
                        print("ATTENTION: In case of an accidential restore:")
                        print("Press Power button to save running configuration, overwriting the restored values in configuration memory.")
                    else:   
                        uhsdr.eprint("... problem occured: ",msg)
                        
                    infile.close()
        else:   
            uhsdr.eprint("... could not find a connected UHSDR with extended CAT commands (required)")
            
        mySer.close()
    


if __name__ == "__main__":
    backupRestoreApp()
