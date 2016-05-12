#! /usr/bin/env python
#

import os
import os.path
import sys

###############################################################################

for arg in sys.argv[1:]:
    filename = os.path.basename(arg)
    (basename, ext) = os.path.splitext(filename)
    print "&%s;" % (basename)
