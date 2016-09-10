import struct
import sys


class octaveRecord:
    def __init__(self):
        self.rname = ""
        self.rtype = ""
        self.rrows = 0
        self.rcols = 0
        self.record = []
    def dumpCell(self,cell,of):
        if (self.rtype == "complex"):
            of.write(" ({:f},{:f})".format(cell.real,cell.imag))
        elif (self.rtype == "float"):
            of.write(" {:f}".format(cell))
        elif (self.rtype == "int"):
            of.write(" {}".format(cell))
        elif (self.rtype == "matrix"):
            of.write(" {}".format(cell))    

    def dumpData(self,of):
        for row in self.record:
            for cell in row:
                self.dumpCell(cell,of)    
            of.write("\n")       

    def dump(self,of):
        of.write("# name: " + self.rname+ "\n")
        of.write("# type: " + self.rtype+ "\n")
        of.write("# rows: " + str(self.rrows) + "\n")
        of.write("# columns: " + str(self.rcols)+ "\n")
        self.dumpData(of)
        of.write("\n")
        of.flush()

    def get_dev(self,a,b):
        if (self.rtype == "complex"):
            iD = 10000.0
            rD = 10000.0
            if abs(a.real - b.real) > 0.000001:
                rD = abs(a.real / (a.real - b.real))
            if abs(a.imag - b.imag) > 0.0:
                iD = abs(a.imag / (a.imag - b.imag))
            return max(rD,iD)
        else:
            return abs(a / (a - b))

    def compare(self,rec2,of):
        of.write(self.rname + ": ")
        if ( self.rrows == rec2.rrows):
            if (self.rcols == rec2.rcols):
                #iterate over both arrays and compare the values
                for rowA,rowB in zip(self.record,rec2.record):
                    for cellA,cellB in zip(rowA,rowB):
                        res = ''
                        if (cellA == cellB):
                            res = '' # 'o'
                        elif self.get_dev(cellA,cellB) > 100.0:
                            res = '' # '#'
                        else:
                            of.write(str(self.get_dev(cellA,cellB)) +":")
                            self.dumpCell(cellA,of)
                            of.write(" /")
                            self.dumpCell(cellB,of)
                            of.write(" ")
                            
                        of.write(res)
                of.write("\n")
            else:
                of.write("col numbers differ\n")
        else:
            print of.write("row numbers differ")
        of.flush()
    

def intStr2float(hexValStr):
    return struct.unpack('f',struct.pack('L',int(hexValStr,16)))
    
def get_complex(word,FloatAsHexString):
    real = 0.0
    imag = 0.0
    if FloatAsHexString:
        real = intStr2float(word.split(",")[0].split("(")[1])[0]
        imag = intStr2float(word.split(",")[1].split(")")[0])[0]
    else:
        real = float(word.split(",")[0].split("(")[1])
        imag = float(word.split(",")[1].split(")")[0])

    return complex(real,imag)

def get_float(word,FloatAsHexString):
    if FloatAsHexString:
        return intStr2float(word)[0]
    else:
        return float(word)

def get_int(word):
    return int(word)

def readFromDump(fileName, FloatAsHexString = True):
    rname=""
    rhex=False
    rtype=""
    rvalues= {}
    rcols=0
    rrows=0
    rheaderWrite=True
    outOfSync = True

    f = open(fileName, 'r')
    recArray = []

    record = octaveRecord()

    records = []

    for line in f:
        if outOfSync == False:
            words = line.split()
            if words:
                if (words[0] == "#"):
                    if (words[1] == "name:"):
                        rname=words[2]
                    elif (words[1] == "type:"):
                        rtype=words[2]
                    elif (words[1] == "rows:"):
                        rrows=int(words[2])
                    elif (words[1] == "columns:"):
                        rcols=int(words[2])
                        rheaderWrite = True
                    elif (words[1] == "hex:"):
                        rhex = words[2] == "true"
        
                else:
                    if (rheaderWrite):
                        recArray = []
                        rheaderWrite = False
                        record.rname = rname
                        record.rtype = rtype
                        record.rrows = rrows
                        record.rcols = rcols
                    if (rtype == "complex"):
                        compArray = []
                        for word in words:
                            compArray.append(get_complex(word,FloatAsHexString))
                        recArray.append(compArray)
                    elif (rtype == "float"):
                        compArray = []
                        for word in words:
                            compArray.append(get_float(word,FloatAsHexString))
                        recArray.append(compArray)
                    elif (rtype == "int"):
                        compArray = []
                        for word in words:
                            compArray.append(get_int(word))
                        recArray.append(compArray)
                    elif (rtype == "matrix"):
                        compArray = []
                        for word in words:
                            compArray.append(get_float(word,rhex))
                        recArray.append(compArray)
                        
                    if (len(recArray) == rrows):
                        record.record = recArray
                        records.append(record)
                        record = octaveRecord()
                        rhex = False
        else:
            outOfSync = line.startswith("# Created by tfdmdv.c") == False
    f.close()
    return records              

records = readFromDump('tfdmdv_out_reference.txt')
records2 = readFromDump('tfdmdv_out.txt')

outputFile = open('log2.out', 'wt')
for rA,rB in zip(records,records2):
    rA.compare(rB,outputFile)

#for record in records2:
#    record.dump(outputFile)                               
outputFile.flush()
outputFile.close()        
