#! /usr/bin/env python
#

import re
import os
import os.path
import sys

###############################################################################

for arg in sys.argv[1:]:
    filename = os.path.basename(arg)
    (basename, ext) = os.path.splitext(filename)
    print '<!ENTITY %s SYSTEM "%s">' % (basename, filename)

print ''

for arg in sys.argv[1:]:
    filename = os.path.basename(arg)
    (basename, ext) = os.path.splitext(filename)
    entname = re.sub('\.{1}', ':', basename)
    content = re.sub('^[^\.]*\.?', '', basename)
    if (filename.startswith('cmd.')):
        content = '!' + content
    print '<!ENTITY %s "<link linkend=\'%s\'>%s</link>">' % (entname, basename, content)
