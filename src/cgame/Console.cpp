#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

Console::Console()
    : font         ( "fonts/6x13",          128, 64, 6, 13, -1 )
    , fontShadowed ( "fonts/6x13_shadowed", 128, 64, 6, 13, -1 )
{
}

///////////////////////////////////////////////////////////////////////////////

Console::~Console()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Console::draw()
{
    deque<string>::size_type max = cg_console.integer;

    // Enforce valid range.
    if (max < 0) {
        max = 0;
        trap_Cvar_Set( "cg_console", "0" );
        trap_Cvar_Update( &cg_console );
    }
    else if (cg_console.integer > 60) {
        max = 60;
        trap_Cvar_Set( "cg_console", "250" );
        trap_Cvar_Update( &cg_console );
    }

    if (max == 0) {
        _lines.clear();
        return;
    }

    while (max < _lines.size())
        _lines.pop_front();

    Font& f = cg_consoleShadowed.integer ? fontShadowed : font;

    const int nlast = _lines.size() < max ? _lines.size() : max;
    int y = nlast * f.charHeight;

    deque<string>::reverse_iterator it = _lines.rbegin();
    for (int i = 0; i < nlast; i++, it++) {
        y -= f.charHeight;
        f.drawLine( 2, y, *it );
    }
}

///////////////////////////////////////////////////////////////////////////////

void
Console::init()
{
    font.registerShader();
    fontShadowed.registerShader();
}

///////////////////////////////////////////////////////////////////////////////

void
Console::print( const char* text )
{
    if (!cg_console.integer)
        return;

    string line;
    for (const char* p = text; *p; p++) {
        const char c = *p;
        switch (c) {
            case '\n':
                _lines.push_back( line );
                line.resize( 0 );
                break;

            default:
                if (c < 33 || c > 126)
                    line += ' ';
                else
                    line += c;
                break;
        }
    }

    if (line.length())
        _lines.push_back( line );
}

///////////////////////////////////////////////////////////////////////////////

vmCvar_t cg_console;
vmCvar_t cg_consoleShadowed;
Console  console;
