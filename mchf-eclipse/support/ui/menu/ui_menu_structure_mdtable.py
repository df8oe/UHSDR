#!/usr/bin/python
"""
2016-12-24 HB9ocq, DB4ple - support program to document the menu structure of uhsdr amateur radio firmware
2017-01-01 HB9ocq - refinement
2017-01-06 HB9ocq - following change in definition of MenuDescriptor

relies upon module  ui_menu_structure_c2py.py  in the same directory

"""


from ui_menu_structure_c2py import MENU_DESCRIPTOR, BUILD_ID
# MENU_DESCRIPTOR is a list of dicts with entries "MENU_ID" "ME_KIND" "NR" "LABEL" "DESC"
# e.g.  [ ... { 'MENU_ID': "TOP",
#               'ME_KIND': "GROUP",
#               'NR': "MENU_BASE",
#               'LABEL': "Standard Menu",
#               'DESC': ":soon:"},
#         ...]
        


from datetime import datetime

TS_NOW = datetime.now().replace(microsecond=0).isoformat()


# all unique MENU_IDs
am = set([md['MENU_ID'] for md in MENU_DESCRIPTOR])  ## random order

# forced ORDER  ## 2017-01-01 HB9ocq - arbitrary selection, feel free to rearrange 
am = ['MENU_TOP',
      'MENU_BASE',
      'MENU_CONF',
      'MENU_FILTER',
      'MENU_CW',
      'MENU_POW',
      'MENU_DISPLAY',
      'MENU_MEN2TOUCH', 
      'MENU_SYSINFO',
      'MENU_DEBUG',
      ]


#-----------------------------------------------------

GEN_SENTENCE = r'generated at  {TS_NOW}  by "{__file__}"'.format(**globals())

OUTPUT = ""

# preface/header
OUTPUT += r"""
[//]: # (                                                                              )
[//]: # ( WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !                         )
[//]: # (                                                                              )
[//]: # ( {GEN_SENTENCE} )
[//]: # (                                                                              )
[//]: # ( mcHF SDR TRX v{BUILD_ID} - Menu Structure Diagram as MarkDown-Table )
[//]: # (                                                                              )
[//]: # ( see <https://help.github.com/categories/writing-on-github/>                  )
[//]: # (                                                                              )

# uhsdr firmware v{BUILD_ID} - UI Menu Overview

{GEN_SENTENCE}


""".format(**globals())

# a paragraph for every group
for gm in [md for md in MENU_DESCRIPTOR if (('MENU_GROUP' == md['ME_KIND']) and (md['NR'] in am))]:
    # header  H2
    OUTPUT += r"""
## {LABEL} (`{NR}`)
    """.format(**gm)

    # table header
    OUTPUT += """
| {:<25}     ({:>43}) | {:<46} | """.format("LABEL", "NR", "DESCRIPTION")
    OUTPUT += """
| {0:-<25}------{0:->43}- | {0:-<46} | """.format("-")
    # table rows
    for md in MENU_DESCRIPTOR:
        if((0 != md['NR']) and ('MENU_STOP' != md['ME_KIND']) and (gm['NR'] ==  md['MENU_ID'])):
            # for sensible entries only
            md['LABEL'] = "**{LABEL}**".format(**md)
            OUTPUT += """
| {LABEL:<29} ({NR:>43}) | {DESC:<46} | """.format(**md)

    OUTPUT += "\n\n"
    
# footer
OUTPUT += r"""
[//]: # ( EOFILE                                                                       )
"""


# are we running as a program?
if '__main__' == __name__:
    print(OUTPUT)
    exit(0)

#EOFILE
