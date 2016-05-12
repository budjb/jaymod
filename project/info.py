#! /usr/bin/env python
#

import os
import platform
import re
import subprocess
import sys
import time

###############################################################################

def toBoolean(value):
    if (not value):
        return 0
    if (value.lower() == 'true'):
        return 1
    if (value.lower() == 'enable'):
        return 1
    if (value.lower() == 'enabled'):
        return 1
    if (value.lower() == 'yes'):
        return 1
    return 0

###############################################################################

def fetchRepoData():
    p = subprocess.Popen('svn info', shell=True, stdout=subprocess.PIPE)
    p.wait()
    if (p.returncode != 0):
        print 'ERROR: command failure: svn info'
        sys.exit(1)

    data = {
        'URL'    : '',
        'UUID'   : '',
        'Rev'    : '',
        'LCRev'  : '',
        'LCDate' : '', }

    headerRX = re.compile('^\\s*([^:]*)\\s*:\\s*(.*)')
    for line in p.stdout:
        mo = headerRX.search(line.strip())
        if (not mo):
            continue

        if (mo.group(1) == 'URL'):
            data['URL'] = mo.group(2)
        elif (mo.group(1) == 'Repository UUID'):
            data['UUID'] = mo.group(2)
        elif (mo.group(1) == 'Revision'):
            data['Rev'] = mo.group(2)
        elif (mo.group(1) == 'Last Changed Rev'):
            data['LCRev'] = mo.group(2)
        elif (mo.group(1) == 'Last Changed Date'):
            data['LCDate'] = mo.group(2)

    return data

###############################################################################

class InfoException(Exception):
    pass
  
###############################################################################

class Project:
    def __init__(this, dbFileName):
        initDB = {
            'name'         : 'Unknown',
            'namef'        : 'unknown',
            'namex'        : 'Unknown',
            'website'      : 'http://www.somewhere.net',
            'irc'          : 'irc.somewhere.net #unknown',
            'author'       : 'Unknown Author',
            'copyright'    : '',
            'versionMajor' : '0',
            'versionMinor' : '0',
            'versionPoint' : '0',
            'buildDate'    : 'disable',
            'buildRelease' : 'false',
            'repoURL'      : 'disable',
            'repoUUID'     : 'disable',
            'repoRev'      : 'disable',
            'repoLCRev'    : 'disable',
            'repoLCDate'   : 'disable',
            'platformName' : 'UNKNOWN', }

        for k in initDB.keys():
            this.__dict__[k] = initDB[k]

        this.platformNamef = platform.system().lower()
        if (re.compile('^cygwin.*').match(this.platformNamef)):
            this.platformNamef = 'windows'
        elif (re.compile('^darwin*').match(this.platformNamef)):
            this.platformNamef = 'osx'
        elif ('PLATFORM' in os.environ):
            this.platformNamef = os.environ['PLATFORM']

        this.variant = this.platformNamef
        if ('VARIANT' in os.environ and len(os.environ['VARIANT']) > 0):
                this.variant += '-' + os.environ['VARIANT']

        this.nightly = ""
        if ('NIGHTLY' in os.environ):
            this.nightly = time.strftime('%Y%m%d-');

        skipRX = re.compile('^\\s*#|^\\s*$')
        vmatchRX = re.compile('^\\s*::(.*)')
        namevalRX = re.compile('^\\s*([A-Za-z][A-Za-z0-9_]*)\\s*=\\s*(.*)')

        variantMatch = 1
        lineno = 0

        f = open(dbFileName, 'r')
        try:
            for line in f:
                line = line.strip()
                lineno += 1

                if (skipRX.search(line)):
                    continue

                mo = vmatchRX.search(line)
                if (mo):
                    if (re.compile(mo.group(1)).search(this.variant)):
                        variantMatch = 1
                    else:
                        variantMatch = 0
                    continue

                if (not variantMatch):
                    continue

                mo = namevalRX.search(line)
                if (not mo):
                    raise InfoException( 'Invalid database line [' + dbFileName
                        + ':' + str(lineno) + ']: ' + line )

                if (not mo.group(1) in initDB):
                    raise InfoException( 'Invalid database key [' + dbFileName
                        + ':' + str(lineno) + ']: ' + mo.group(1) )

                this.__dict__[mo.group(1)] = mo.group(2)

        finally:
            f.close()

        this.versionMajor = int(this.versionMajor)
        this.versionMinor = int(this.versionMinor)
        this.versionPoint = int(this.versionPoint)

        this.versionHex = "0x%08x" % (
            (this.versionMajor & 0xff) << 24 |
            (this.versionMinor & 0xff) << 16 |
            (this.versionPoint & 0xffff) )

        if ( toBoolean(this.buildDate) ):
            this.buildDate = time.strftime('%a %b %d %H:%M:%S %Z %Y')
        else:
            this.buildDate = 'UNKNOWN'

        if (len(this.nightly) > 0):
            this.buildStability = 'nightly'
        elif ( toBoolean(this.buildRelease) ):
            if ( this.versionMinor % 2 ):
                this.buildStability = 'experimental'
            else:
                this.buildStability = 'stable'
        else:
            this.buildStability = 'development'

        this.buildTarget = this.variant

        data = fetchRepoData()
        if ( toBoolean(this.repoURL) ):
            this.repoURL = data['URL']
        else:
            this.repoURL = ''

        if ( toBoolean(this.repoUUID) ):
            this.repoUUID = data['UUID']
        else:
            this.repoUUID = ''

        if ( toBoolean(this.repoRev) ):
            this.repoRev = data['Rev']
        else:
            this.repoRev = '0'

        if ( toBoolean(this.repoLCRev) ):
            this.repoLCRev = data['LCRev']
        else:
            this.repoLCRev = '0'

        if ( toBoolean(this.repoLCDate) ):
            this.repoLCDate = data['LCDate']
        else:
            this.repoLCDate = ''

        this.title = "%s %s%d.%d.%d" % (
            this.name,
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint )

        this.titlex = "%s ^f%s%d.%d.%d" % (
            this.namex,
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint )

        this.version = "%s%d.%d.%d" % (
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint )

        this.versionx = "^f%s%d.%d.%d" % (
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint )

        this.packageBase = "%s-%s%d.%d.%d" % (
            this.namef,
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint )

        this.packageBasev = "%s-%s%d.%d.%d-%s" % (
            this.namef,
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint,
            this.variant )

        this.pk3 = "%s-%s%d.%d.%d.pk3" % (
            this.namef,
            this.nightly,
            this.versionMajor,
            this.versionMinor,
            this.versionPoint )

    def dump(this, mode):
        if (mode == 1):
            print 'PROJECT.name           = ' + this.name
            print 'PROJECT.namef          = ' + this.namef
            print 'PROJECT.namex          = ' + this.namex
            print 'PROJECT.website        = ' + this.website
            print 'PROJECT.irc            = ' + this.irc.replace('#', '\\#')
            print 'PROJECT.author         = ' + this.author
            print 'PROJECT.copyright      = ' + this.copyright
            print 'PROJECT.versionMajor   = ' + str( this.versionMajor )
            print 'PROJECT.versionMinor   = ' + str( this.versionMinor )
            print 'PROJECT.versionPoint   = ' + str( this.versionPoint )
            print 'PROJECT.buildDate      = ' + this.buildDate
            print 'PROJECT.buildStability = ' + this.buildStability
            print 'PROJECT.buildTarget    = ' + this.buildTarget
            print 'PROJECT.repoURL        = ' + this.repoURL
            print 'PROJECT.repoUUID       = ' + this.repoUUID
            print 'PROJECT.repoRev        = ' + this.repoRev
            print 'PROJECT.repoLCRev      = ' + this.repoLCRev
            print 'PROJECT.repoLCDate     = ' + this.repoLCDate
            print 'PROJECT.platformName   = ' + this.platformName
            print 'PROJECT.platformNamef  = ' + this.platformNamef
            print 'PROJECT.title          = ' + this.title
            print 'PROJECT.titlex         = ' + this.titlex
            print 'PROJECT.version        = ' + this.version
            print 'PROJECT.versionx       = ' + this.versionx
            print 'PROJECT.packageBase    = ' + this.packageBase
            print 'PROJECT.packageBasev   = ' + this.packageBasev
            print 'PROJECT.pk3            = ' + this.pk3

        if (mode == 2):
            print '#define JAYMOD_name           "' + this.name + '"'
            print '#define JAYMOD_namef          "' + this.namef + '"'
            print '#define JAYMOD_namex          "' + this.namex + '"'
            print '#define JAYMOD_website        "' + this.website + '"'
            print '#define JAYMOD_irc            "' + this.irc + '"'
            print '#define JAYMOD_author         "' + this.author  + '"'
            print '#define JAYMOD_copyright      "' + this.copyright + '"'
            print '#define JAYMOD_versionMajor   "' + str( this.versionMajor ) + '"'
            print '#define JAYMOD_versionMinor   "' + str( this.versionMinor ) + '"'
            print '#define JAYMOD_versionPoint   "' + str( this.versionPoint ) + '"'
            print '#define JAYMOD_versionHex     ' + this.versionHex
            print '#define JAYMOD_buildDate      "' + this.buildDate + '"'
            print '#define JAYMOD_buildStability "' + this.buildStability + '"'
            print '#define JAYMOD_buildTarget    "' + this.buildTarget + '"'
            print '#define JAYMOD_repoURL        "' + this.repoURL + '"'
            print '#define JAYMOD_repoUUID       "' + this.repoUUID + '"'
            print '#define JAYMOD_repoRev        "' + this.repoRev + '"'
            print '#define JAYMOD_repoLCRev      "' + this.repoLCRev + '"'
            print '#define JAYMOD_repoLCDate     "' + this.repoLCDate + '"'
            print '#define JAYMOD_platformName   "' + this.platformName + '"'
            print '#define JAYMOD_platformNamef  "' + this.platformNamef + '"'
            print '#define JAYMOD_title          "' + this.title + '"'
            print '#define JAYMOD_titlex         "' + this.titlex + '"'
            print '#define JAYMOD_version        "' + this.version + '"'
            print '#define JAYMOD_versionx       "' + this.versionx + '"'
            print '#define JAYMOD_packageBase    "' + this.packageBase + '"'
            print '#define JAYMOD_packageBasev   "' + this.packageBasev + '"'
            print '#define JAYMOD_pk3            "' + this.pk3 + '"'
            print
            print '#define JAYMOD_' + this.platformNamef.upper()
            print '#define JAYMOD_' + this.buildStability.upper()

        if (mode == 3):
            print 'define(<<__name>>, <<'           + this.name                + '>>)dnl'
            print 'define(<<__namef>>, <<'          + this.namef               + '>>)dnl'
            print 'define(<<__website>>, <<'        + this.website             + '>>)dnl'
            print 'define(<<__irc>>, <<'            + this.irc                 + '>>)dnl'
            print 'define(<<__author>>, <<'         + this.author              + '>>)dnl'
            print 'define(<<__copyright>>, <<'      + this.copyright           + '>>)dnl'
            print 'define(<<__versionMajor>>, <<'   + str( this.versionMajor ) + '>>)dnl'
            print 'define(<<__versionMinor>>, <<'   + str( this.versionMinor ) + '>>)dnl'
            print 'define(<<__versionPoint>>, <<'   + str( this.versionPoint ) + '>>)dnl'
            print 'define(<<__buildDate>>, <<'      + this.buildDate           + '>>)dnl'
            print 'define(<<__buildStability>>, <<' + this.buildStability      + '>>)dnl'
            print 'define(<<__buildTarget>>, <<'    + this.buildTarget         + '>>)dnl'
            print 'define(<<__repoURL>>, <<'        + this.repoURL             + '>>)dnl'
            print 'define(<<__repoUUID>>, <<'       + this.repoUUID            + '>>)dnl'
            print 'define(<<__repoRev>>, <<'        + this.repoRev             + '>>)dnl'
            print 'define(<<__repoLCRev>>, <<'      + this.repoLCRev           + '>>)dnl'
            print 'define(<<__repoLCDate>>, <<'     + this.repoLCDate          + '>>)dnl'
            print 'define(<<__platformName>>, <<'   + this.platformName        + '>>)dnl'
            print 'define(<<__platformNamef>>, <<'  + this.platformNamef       + '>>)dnl'
            print 'define(<<__title>>, <<'          + this.title               + '>>)dnl'
            print 'define(<<__titlex>>, <<'         + this.titlex              + '>>)dnl'
            print 'define(<<__version>>, <<'        + this.version             + '>>)dnl'
            print 'define(<<__versionx>>, <<'       + this.versionx            + '>>)dnl'
            print 'define(<<__packageBase>>, <<'    + this.packageBase         + '>>)dnl'
            print 'define(<<__packageBasev>>, <<'   + this.packageBasev        + '>>)dnl'
            print 'define(<<__pk3>>, <<'            + this.pk3                 + '>>)dnl'

        if (mode == 4):
            print '<!ENTITY project:name           "' + this.name                + '">'
            print '<!ENTITY project:namef          "' + this.namef               + '">'
            print '<!ENTITY project:namex          "' + this.namex               + '">'
            print '<!ENTITY project:website        "' + this.website             + '">'
            print '<!ENTITY project:irc            "' + this.irc                 + '">'
            print '<!ENTITY project:author         "' + this.author              + '">'
            print '<!ENTITY project:copyright      "' + this.copyright           + '">'
            print '<!ENTITY project:versionMajor   "' + str( this.versionMajor ) + '">'
            print '<!ENTITY project:versionMinor   "' + str( this.versionMinor ) + '">'
            print '<!ENTITY project:versionPoint   "' + str( this.versionPoint ) + '">'
            print '<!ENTITY project:buildDate      "' + this.buildDate           + '">'
            print '<!ENTITY project:buildStability "' + this.buildStability      + '">'
            print '<!ENTITY project:buildTarget    "' + this.buildTarget         + '">'
            print '<!ENTITY project:repoURL        "' + this.repoURL             + '">'
            print '<!ENTITY project:repoUUID       "' + this.repoUUID            + '">'
            print '<!ENTITY project:repoRev        "' + this.repoRev             + '">'
            print '<!ENTITY project:repoLCRev      "' + this.repoLCRev           + '">'
            print '<!ENTITY project:repoLCDate     "' + this.repoLCDate          + '">'
            print '<!ENTITY project:platformName   "' + this.platformName        + '">'
            print '<!ENTITY project:platformNamef  "' + this.platformNamef       + '">'
            print '<!ENTITY project:title          "' + this.title               + '">'
            print '<!ENTITY project:titlex         "' + this.titlex              + '">'
            print '<!ENTITY project:version        "' + this.version             + '">'
            print '<!ENTITY project:versionx       "' + this.versionx            + '">'
            print '<!ENTITY project:packageBase    "' + this.packageBase         + '">'
            print '<!ENTITY project:packageBasev   "' + this.packageBasev        + '">'
            print '<!ENTITY project:pk3            "' + this.pk3                 + '">'

###############################################################################

mode = 0
dbFileName = 'info.db'

for arg in sys.argv[1:]:
    if (arg == '-mk'):
        mode = 1
        continue

    if (arg == '-h'):
        mode = 2
        continue

    if (arg == '-m4'):
        mode = 3
        continue

    if (arg == '-xml'):
        mode = 4
        continue

    dbFileName = arg

if (mode == 0):
    print 'ERROR: mode must be specified. Expecting one of { -mk, -h, -m4, -tex }'
    sys.exit(1)

###############################################################################

try:
    p = Project(dbFileName)
    p.dump(mode)
except (InfoException, IOError), x:
    print str(x)
    sys.exit(1)
