#!/usr/bin/python

"""

2016-12-30 HB9ocq - extract mcHF LCD-menu definition data from C-cource
                    and transform to Python, showing on stdout

WARNING: THIS BASES HEAVILY UPON (UN-)DISCIPLINE IN C SYNTAX ! ! !
WARNING: ANY CHANGE OF DEFINITIONS AND DECLARATIONS IN $INPUT_C_SRC
         POTENTIALLY BREAKS THIS TRANSFORMATION ! ! !

LAST WARNING:  YOU HAVE BEEN WARNED ! ! !


2017-01-01 HB9ocq - replacement for prototype shell script (ui_menu_structure_c2py.sh) 
                    for improved parsing accuracy (avoids issues with commas and 
                    all uppercase text in UiMenuDesc(...))
2017-01-06 HB9ocq - following change in definition of MenuDescriptor

AFTERLAST WARNING: BINDS EVEN TIGHTER TO WRITING DISCIPLINE IN C SYNTAX ! ! !

"""

# this points from HERE to the 'mchf-eclipse' directory of our project
MCHF_BASEDIR = r"../../../"

# the ONLY C-source we do read AND understand
INPUT_C_SRC = MCHF_BASEDIR + r"drivers/ui/menu/ui_menu_structure.c"


# to identify the build we rely on contents of this file
vers_data = None
with open(MCHF_BASEDIR + r"versions.txt", 'r') as f:
    vers_data = f.readlines()

## vers_data  contains just two lines:
## the first contains "the field headers",  identifiers are bracketed in '<>'
## e.g. """<bootloader testing> <firmware testing> <bootloader stable> <firmware stable>"""
## the second contains "the field values",   COMMA separated,  ATTENTION: also a leading COMMA is there, who knows what for...
## e.g. """,---,1.5.6,2.0.0,1.4.0"""
## and odd line endings (!)

#print("##DBG## vers_data[0] = {!r}".format(vers_data[0].strip()))
#print("##DBG## vers_data[1] = {!r}".format(vers_data[1].split(',')))

VERSIONSID = vers_data[1].split(',')[1:]  ## yank the first, empty element
BUILD_ID = VERSIONSID[1]  ## 1 = arbitrarily made selection for <firmware testing>

#print("##DBG## BUILD_ID = '{}'".format(BUILD_ID))


from datetime import datetime

TS_NOW = datetime.now().replace(microsecond=0).isoformat()


#-----------------------------------------------------

# read in the whole C source
menu_csrc = []
with open(INPUT_C_SRC, 'r') as f:
    menu_csrc = f.readlines()
    

MENU_DESCRIPTOR = []
"""
MENU_DESCRIPTOR is the datastructure we're gonna build

MENU_DESCRIPTOR is a list of dicts with entries "MENU_ID" "ME_KIND" "NR" "LABEL" "DESC"

e.g.  [ ... { 'MENU_ID': "TOP",
              'ME_KIND': "GROUP",
              'NR': "MENU_BASE",
              'LABEL': "Standard Menu",
              'DESC': ":soon:" },
        ...]
"""

    
import re

MENULN_REO = re.compile(  # pattern for lines..
      r'^\s+{\s*'                            # ..starting with whitespace + a left brace
    + r'(?P<MENU_ID>MENU_[\w]+)'             # ..an identifier 'MENU_xxx', we call it MENU_ID
    + r'[,\s]+'
    + r'(?P<ME_KIND>MENU_[\w]+)'             # ..a 2nd id 'MENU_xxx', we call it ME_KIND
    + r'[,\s]+'
    + r'(?P<NR>[\w]+)'                       # ..a 3rd rather long id 'yy...zz', we call it NR
    + r'[,\s]+'
    + r'NULL'                                # ..an initial value for a pointer: not relevant for handbook
    + r'[,\s]+'
    + r'"(?P<LABEL>[^"]*)"'                  # ..n chars in quotes, we call this LABEL
    + r'[,\s]+'
    + r'UiMenuDesc\("(?P<DESC>[^"]*)"\s*\)'  # ..a UiMenuDesc in quotes, we call it DESC
    + r'.*}.*$'                              # ..unintresting rest, incl. right brace 
)


# each line of the C source...
for ln in menu_csrc:
    # ...gets RE-searched for the interesting data,
    m = MENULN_REO.search(ln)
    if m:  # ...as things match: keep the ready-cooked dict
        MENU_DESCRIPTOR.append(m.groupdict())
        
if [] == MENU_DESCRIPTOR:
    print('ERROR: MENU_DESCRIPTOR is empty! (no lines defining UiMenu entries found in "{}".'.format(INPUT_C_SRC))
    exit(-1)

#-----------------------------------------------------

OUTPUT = """\
#!/usr/bin/env python
#
# WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !
#
# generated at  {TS_NOW}  by  "{__file__}"
#
# out of  {INPUT_C_SRC}  ({BUILD_ID})
#

BUILD_ID = "{BUILD_ID}"

MENU_DESCRIPTOR = {}

#EOFILE
"""


from pprint import pformat


# are we running as a program?
if '__main__' == __name__:
    print(OUTPUT.format(pformat(MENU_DESCRIPTOR), **globals()))
    exit(0)
    
#EOFILE
