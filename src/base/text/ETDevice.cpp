#include <base/public.h>
#include <base/text/impl.h>

namespace text {

///////////////////////////////////////////////////////////////////////////////

ETDevice::ETDevice( uint32 cols_, uint32 rows_, uint32 indent_ )
    : AbstractDevice ( cols_, rows_, indent_ )
    , cnormal        ( '9' )
    , cbold          ( 'O' )
    , cdim           ( 'H' )
    , cpass          ( '2' )
    , cfail          ( '1' )
    , cwarning       ( '6' )
    , cdebug         ( 'B' )
    , cdebugBOLD     ( '2' )
    , cdebugDIM      ( '9' )
    , cheader        ( '8' )
    , cheaderBOLD    ( '3' )
    , cheaderDIM     ( '@' )
    , cvalue         ( 'G' )
    , cvalueBOLD     ( '3' )
    , cvalueDIM      ( 'M' )
{
}
 
///////////////////////////////////////////////////////////////////////////////

ETDevice::~ETDevice()
{
}

///////////////////////////////////////////////////////////////////////////////

void
ETDevice::doFormat( const Buffer& buffer, list<string>& lines )
{
    string pending;
    uint32 position = 0;
    bool   wrapping = false;
    uint32 width    = cols;

    uint32 lmargin = 0;
    stack<uint32> lmarginStack;

    uint32 rmargin = 0;
    stack<uint32> rmarginStack;

    char color = '7';
    stack<char> colorStack;

    for ( uint32 i = 0; i < buffer.length; i++ ) {
        const uint8 code = buffer.data[i];
        switch (code) {
            case MANIP_COLOR_BLACK:
            case MANIP_COLOR_RED:
            case MANIP_COLOR_GREEN:
            case MANIP_COLOR_YELLOW:
            case MANIP_COLOR_BLUE:
            case MANIP_COLOR_CYAN:
            case MANIP_COLOR_MAGENTA:
            case MANIP_COLOR_WHITE:
            case MANIP_COLOR_ORANGE:
            case MANIP_COLOR_MDGREY:
            case MANIP_COLOR_LTGREY:
            case MANIP_COLOR_LTGREY2:
            case MANIP_COLOR_MDGREEN:
            case MANIP_COLOR_MDYELLOW:
            case MANIP_COLOR_MDBLUE:
            case MANIP_COLOR_MDRED:
            case MANIP_COLOR_MDBROWN:
            case MANIP_COLOR_LTORANGE:
            case MANIP_COLOR_DKCYAN:
            case MANIP_COLOR_MDPURPLE:
            case MANIP_COLOR_MDCYAN:
            case MANIP_COLOR_PURPLE:
            case MANIP_COLOR_BLUEGREY:
            case MANIP_COLOR_LTOLIVE:
            case MANIP_COLOR_DKGREEN:
            case MANIP_COLOR_LTRED:
            case MANIP_COLOR_ROSE:
            case MANIP_COLOR_DKORANGE:
            case MANIP_COLOR_LTBROWN:
            case MANIP_COLOR_MDGOLD:
            case MANIP_COLOR_LTGOLD:
            case MANIP_COLOR_GOLD: {
                    char tmp = '0' + ((code - 0x80) & 31);
                    if (color != tmp) {
                        color = tmp;
                        pending += '^';
                        pending += color;
                    }
                }
                break;

            case MANIP_COLOR_NONE:
                break;

#define CASE_PSEUDO_COLOR(manip,literal) \
            case manip: \
                if (color != literal) { \
                    color = literal; \
                    pending += '^'; \
                    pending += color; \
                } \
                break;

CASE_PSEUDO_COLOR( MANIP_COLOR_NORMAL,     cnormal )
CASE_PSEUDO_COLOR( MANIP_COLOR_BOLD,       cbold )
CASE_PSEUDO_COLOR( MANIP_COLOR_DIM,        cdim )
CASE_PSEUDO_COLOR( MANIP_COLOR_PASS,       cpass )
CASE_PSEUDO_COLOR( MANIP_COLOR_FAIL,       cfail )
CASE_PSEUDO_COLOR( MANIP_COLOR_WARNING,    cwarning )
CASE_PSEUDO_COLOR( MANIP_COLOR_DEBUG,      cdebug )
CASE_PSEUDO_COLOR( MANIP_COLOR_DEBUGBOLD,  cdebugBOLD )
CASE_PSEUDO_COLOR( MANIP_COLOR_DEBUGDIM,   cdebugDIM )
CASE_PSEUDO_COLOR( MANIP_COLOR_HEADER,     cheader )
CASE_PSEUDO_COLOR( MANIP_COLOR_HEADERBOLD, cheaderBOLD )
CASE_PSEUDO_COLOR( MANIP_COLOR_HEADERDIM,  cheaderDIM )
CASE_PSEUDO_COLOR( MANIP_COLOR_VALUE,      cvalue )
CASE_PSEUDO_COLOR( MANIP_COLOR_VALUEBOLD,  cvalueBOLD )
CASE_PSEUDO_COLOR( MANIP_COLOR_VALUEDIM,   cvalueDIM )

#undef CASE_PSEUDO_COLOR

            case MANIP_CPOP:
                if (!colorStack.empty()) {
                    color = colorStack.top();
                    colorStack.pop();
                    pending += '^';
                    pending += color;
                }
                break;

            case MANIP_CPUSH:
                colorStack.push( color );
                break;

            case MANIP_LINDENT:
                lmarginStack.push( lmargin );
                lmargin += indent;
                break;

            case MANIP_LUNINDENT:
                if (!lmarginStack.empty()) {
                    lmargin = lmarginStack.top();
                    lmarginStack.pop();
                }
                break;

            case MANIP_NEWLINE:
                pending += '\n';
                lines.push_back( pending );
                pending.clear();
                position = 0;
                pending += '^';
                pending += color;
                break;

            case MANIP_RINDENT:
                rmarginStack.push( rmargin );
                rmargin += indent;
                width = cols - rmargin;
                break;

            case MANIP_RUNINDENT:
                if (!rmarginStack.empty()) {
                    rmargin = rmarginStack.top();
                    rmarginStack.pop();
                    width = cols - rmargin;
                }
                break;

            case ' ':
                if (!wrapping) {
                    if (position < lmargin) {
                        const uint32 delta = lmargin - position;
                        pending.append( delta, ' ' );
                        position += delta;
                    }

                    pending += ' ';
                    position++;
                }
                break;

            default:
                if (wrapping)
                    wrapping = false;

                if (position < lmargin) {
                    const uint32 delta = lmargin - position;
                    pending.append( delta, ' ' );
                    position += delta;
                }

                if (isprint( code )) {
                    pending += char(code);
                }
                else {
                    pending += '.';
                }

                position++;
                break;
        }

        // A little safety; width values >= 20 required before we wrap.
        if (width >= 20 && position >= width) {
            pending += '\n';
            lines.push_back( pending );
            pending.clear();
            position = 0;
            wrapping = true;
            pending += '^';
            pending += color;
        }
    }

    if (position) {
        pending += '\n';
        lines.push_back( pending );
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace text
