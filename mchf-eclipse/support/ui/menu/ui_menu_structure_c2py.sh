#!/usr/bin/env bash

#
#  2016-12-21 HB9ocq - extract mcHF LCD-menu definition data from C-cource
#                      and transform to Python, showing on stdout
#
#  WARNING: THIS BASES HEAVILY UPON (UN-)DISCIPLINE IN C SYNTAX ! ! !
#  WARNING: ANY CHANGE OF DEFINITIONS AND DECLARATIONS IN $INPUT_C_SRC
#           POTENTIALLY BREAKS THIS TRANSFORMATION ! ! !
#
#  LAST WARNING:  YOU HAVE BEEN WARNED ! ! !
#

# the C-source to be read
INPUT_C_SRC=../../../drivers/ui/menu/ui_menu_structure.c

# Magic Formula
# -------------
#
#  1.   select lines of  $INPUT_C_SRC  starting with pattern  '<whitespaces>{<whithespaces>MENU_'
#
#    process each such that...
#
#  2.a  ...identifiers of  enum MENU_KIND  get renamed to MEK_...  (Menu Entry Kind)
#  2.b  ...C-macro UiMenuDesc(..) gets replaced
#  2.c  ...leading whitespaces gets homogenised to two python indents (' '*8) incl. '{'->'[',
#          trailing "garbage" gets yanked incl. '}'->']',
#  2.d  ...identifiers get quoted to become Python strings
#  2.e  ...(C) NULL and 0 gets (Python) None
#  2.f  ...force 1 space after comma (between entries)
#  2.g  ...yank plenks
#
#
#    the list comprehension + "dict(zip(..))" are basic Python building blocks
#    and need no further explanation HERE.
#
#
#    now we have a nice Python module, suitable for direct importing
#    and comfortable dict-wise data access  - all whishes granted!  :-)
#
#
#  QA-HINT: for syntax-checking the output, use  "./ui_menu_structure_c2py.sh | python - "
#

cat  <<  EOHEREFILE
#!/usr/bin/env python
#
# WARNING: generated data!  DO NOT EDIT MANUALLY ! ! !
#
# generated from  ${INPUT_C_SRC}  at  $(date  --utc  --iso=s)  by "$0"
#

MENU_DESCRIPTOR = [ dict(zip(["MENU_ID", "ME_KIND", "NR", "ID", "LABEL", "DESC"], e)) for e in [

$(  cat ${INPUT_C_SRC}  \
    | grep -P '^\s+{\s+MENU_' ${INPUT_C_SRC}  \
    | sed  -e 's!, MENU_\(ITEM\|GROUP\|INFO\|STOP\)!, MEK_\1!g'  \
           -e 's!UiMenuDesc(\("[^"]*"\))!\1!g'  \
           -e 's!^[ \x9]\+{!       \[!;s!},*.*! \],!'  \
           -e 's! \([A-Z][A-Z_0-9]\+\),! "\1",!g'  \
           -e 's! \(NULL\|0\) ! None !g'  \
           -e 's!",[ ]\+"!", "!g'  \
           -e 's! ,!,!g'  \
)

   ]
]

#EOFILE
EOHEREFILE
