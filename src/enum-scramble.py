#! /usr/bin/env python
#

import re
import random
import sys

###############################################################################

mode = 0
filename = ''
for arg in sys.argv[1:]:
    if (arg == '-h'):
        mode = 1
        continue

    if (arg == '-cpp'):
        mode = 2
        continue

    filename = arg

if (mode == 0):
    print 'ERROR: mode must be specified. Expecting one of { -h, -cpp }'
    sys.exit(1)

if (filename == ''):
    print 'ERROR: input file must be specified.'
    sys.exit(1)

###############################################################################

fh = open( filename, 'r' )
items = []

if (mode == 1):
    for line in fh:
        line = line.strip()
        value = re.sub( '([A-Za-z0-9_]*).*', '\\1', line )
        items.append( "    %s," % ( value ))
    random.shuffle( items )

if (mode == 2):
    for line in fh:
        line = line.strip()
        value = re.sub( '([A-Za-z0-9_]*).*', '\\1', line )
        items.append( "    \"%s\"," % ( value ))

fh.close()

for line in items:
    print line
