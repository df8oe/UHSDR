"""
This module contains experimental code for using the (extend) UHSDR API

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
import os

class CatCmdFt817:
    """
    List of used/supported FT817 CAT command codes
    This list includes the officially known FT817 codes (including the "undocumented" ones)
    """
    READ_EEPROM = 0xBB
    WRITE_EEPROM = 0xBC

class CatCmd(CatCmdFt817):
    """
    This list extends the FT817 by the special UHSDR codes implemented only in the UHSDR dialect
    of FT817 CAT (which must be different from the FT817 ones)
    """
    
    UHSDR_ID = 0x42
    """
    this will return the bytes ['U', 'H' , 'S', 'D', 'R' ] and is used to identify an UHSDR with high enough firmware level
    """
    
class UhsdrConfigIndex:
    """
    this is a completely incomplete list of config value indicies as used in the UHSDR firmware
    we list only the absolutely necessary ids here for the moment (those must never change in different firmware versions, would break this code)
    """
    VER_MAJOR = 176
    VER_MINOR = 310
    VER_BUILD = 171
    NUMBER_OF_ENTRIES = 407

def eprint(*args, **kwargs):
    """
    a small function to print to stderr, used for error and logging messages
    """
    print(*args, file=sys.stderr, **kwargs)

class catSerial:
    """
    Low Level FT817 CAT protocol handling on a serial communication
    """
    def __init__(self, comObj):
        self.comObj = comObj
    def sendCommand(self, command):
        bytesWritten = self.comObj.write(command)
        return bytesWritten == 5

    def readResponse(self,count):
        response = self.comObj.read(count)
        return (len(response) == count,response)
    

class catCommands:
    """
    CAT API: Here we have direct access to CAT API ations, each logical API function has its direct counterpart in this
    class
    We do not implement any extra control logic in here, just the call of the API function and returning the response
    This may be enriched to have all availabl CAT API functions, right now it is just what we need now
    """
    
    def __init__(self, catObj):
        self.catObj = catObj

    def execute(self, cmd, count):
        if self.catObj.sendCommand(cmd):
            ok,res = self.catObj.readResponse(count)
            return ok,bytearray(res)
        else:
            return (False,bytearray([]))

    def readEEPROM(self, addr):
        cmd = bytearray([ (addr & 0xff00)>>8,addr & 0xff, 0x00, 0x00, CatCmd.READ_EEPROM])
        ok,res = self.execute(cmd,2)
        if ok:
            return res[1] * 256 + res[0]
        else:
            return ok

    def readUHSDR(self):
        cmd = bytearray([ 0x00, 0x00 , 0x00, 0x00, CatCmd.UHSDR_ID])
        ok,res = self.execute(cmd,5)

        return res == bytearray("UHSDR")
   

    def writeEEPROM(self, addr, value16bit):
        cmd = bytearray([ (addr & 0xff00)>>8,addr & 0xff, (value16bit & 0xff) >> 0, (value16bit & 0xff00) >> 8, CatCmd.WRITE_EEPROM])
        ok,res = self.execute(cmd,1)
        return ok
    
    def readUHSDRConfig(self, index):
        return self.readEEPROM(index + 0x8000);

    def writeUHSDRConfig(self, index, value):
        return self.writeEEPROM(index + 0x8000, value);


class UhsdrConfig():
    """
    CONFIG MANAGEMENT: Handling of reading / writing TRX configurations, detection of TRX presence etc.
    This class represents high-level actions, should involve proper parameter checking etc.
    """

    def __init__(self, catObj):
        self.catObj = catObj

    def getVersion(self):
        """
        return firmware version as integer tuple (major,minor,build)
        or (False,False,False) if something goes wrong 
        """
        return (self.catObj.readUHSDRConfig(UhsdrConfigIndex.VER_MAJOR),
                    self.catObj.readUHSDRConfig(UhsdrConfigIndex.VER_MINOR),
                    self.catObj.readUHSDRConfig(UhsdrConfigIndex.VER_BUILD))

    def isUhsdrConnected(self):
        """
        we test if an UHSDR with extended API is connected by using an identification API call not present
        on a FT817 or older UHSDR / mcHF firmwares
        returns: True if a suitable TRX is connected, False otherwise
        """
        return self.catObj.readUHSDR()

    #
    def getConfigValueCount(self):
        return self.catObj.readUHSDRConfig(UhsdrConfigIndex.NUMBER_OF_ENTRIES)
        
    def getValue(self, index):
        # TODO: do some range checking here
        return self.catObj.readUHSDRConfig(index)

    def setValue(self, index, value):
        # TODO: do some range checking here
        retval = False
        if self.catObj.writeUHSDRConfig(index, value):
            retval = value == self.getValue(index)
        
        return retval


    def configToJson(self):
        """
        read the configuration from TRX into a data dictionary

        returns tuple with boolean success state and read data
        
        right now the returned JSON structure is quite simple
        we store the version number as list of 3 integers under the 'version' key
        we store the date and time of backup as UTC under the 'when' key
        we store each configuration value as addr/value pair under the 'eeprom' key
        """
        from datetime import datetime

        retval = False
        valList = []
        self.data = {}
        self.data['version'] = self.getVersion()
        self.data['when'] = str(datetime.utcnow())
        self.data['eeprom'] = []
        numberOfValues = self.getConfigValueCount()
        for index in range(numberOfValues):
            val = self.getValue(index)
            valList.append(val)
            self.data['eeprom'].append({ 'addr' : index , 'value' : val })
            
        retval = all(val is not False for val in valList) or len(valList) != 0
        return retval,self.data

    def jsonToConfig(self,data):
        """
        write the configuration in passed data dictionary to TRX

        returns tuple with boolean success state and human readable return msg 

        data dictionary must conform to format generated by configToJson()
        """
        
        retval = True
        retmsg = "OK"
        if data['when'] != None and len(data['version']) == 3 and len(data['eeprom']) == data['eeprom'][UhsdrConfigIndex.NUMBER_OF_ENTRIES]['value']:
            numberOfValues = data['eeprom'][UhsdrConfigIndex.NUMBER_OF_ENTRIES]['value']
            # we do not restore index 0 as it contains EEPROM type. This is never changed during configuration backup, too dangerous
            for index in range(numberOfValues):
                addr = data['eeprom'][index]['addr']
                value = data['eeprom'][index]['value']
                if addr != 0:
                    if self.setValue(addr,value) == False:
                        retmsg = "Restoring value {} at addr {} failed".format(value,addr)
                        retval = False
                        break
        else:
            retmsg = "Configuration data failed consistency check"
            retval = False
        return retval,retmsg

