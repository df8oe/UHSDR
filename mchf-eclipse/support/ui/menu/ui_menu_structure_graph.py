#!/usr/bin/python
"""
2017-01-01 HB9ocq - support program to document the menu structure of mcHF amateur radio SDR TRX
2017-01-06 HB9ocq - following change in definition of MenuDescriptor

relies upon module  ui_menu_structure_c2py.py  in the same directory

"""


from ui_menu_structure_c2py import MENU_DESCRIPTOR, BUILD_ID
# MENU_DESCRIPTOR is a list of dicts with entries "MENU_ID" "ME_KIND" "NR" "LABEL"
# e.g.  [ ... { 'MENU_ID': "TOP",
#               'ME_KIND': "GROUP",
#               'NR': "MENU_BASE",
#               'LABEL': "Standard Menu",
#               'DESC': ":soon:" },
#         ...]
        


from datetime import datetime

TS_NOW = datetime.now().replace(microsecond=0).isoformat()


#-----------------------------------------------------

GEN_SENTENCE = r'generated at  {TS_NOW}  by "{__file__}"'.format(**globals())

OUTPUT = ""

# preface/header
OUTPUT += r"""
#  
#  WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !
#  
#  {GEN_SENTENCE}
#  
#  mcHF SDR TRX v{BUILD_ID} - Menu Structure Diagram in DOT-language
#  
#  (see <http://www.graphviz.org/content/dot-language> )
#  

    digraph mcHF_menus {{
       
        graph [ fontsize = 14,
                label = "\nmcHF v{BUILD_ID} - Menus Overview\n{GS2}",
              ];
       
        rankdir=LR
        nodesep=.05
       
        #  -  -  -  -
""".format(GS2=GEN_SENTENCE.replace('"', '\\"'), **globals())

# artificial node for MENU_TOP
OUTPUT += """
        "MENU_TOP" [
            shape = none
            image = "mcHF-logo.png"
            label = ""
        ];

"""

# body - nodes

for md in MENU_DESCRIPTOR:
    if((0 != md['NR']) and ('MENU_STOP' != md['ME_KIND'])):
        # declare a node for id
        OUTPUT += """
        "{NR}" [
             label = "{NR} | {LABEL}"
             shape = record
             ];
        """.format(**md).replace('<', '\<').replace('>', '\>')


# body - edges

for mId in set([md['MENU_ID'] for md in MENU_DESCRIPTOR]):
    # start subgraph for every "parent"
    OUTPUT += """
        #  -  -  -  -

        subgraph "{0}" {{
            label = "{0} beef.0f.dead.e5e1"

    """.format(mId)

    for md in MENU_DESCRIPTOR:
        if((0 != md['NR']) and ('MENU_STOP' != md['ME_KIND']) and (mId == md['MENU_ID'])):
            # ...add an edge
            OUTPUT += """
            {MENU_ID} -> {NR}
            """.format(**md)

    OUTPUT += """
        }}  ## END subgraph {MENU_ID}
    """.format(**md)

    
# footer
OUTPUT += """
        #  -  -  -  -
    }

#EOFILE
"""


# are we running as a program?
if '__main__' == __name__:
    print(OUTPUT)
    exit(0)

#EOFILE
