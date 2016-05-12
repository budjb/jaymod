#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

Font::Font(
    const string& shaderPath_,
    int    shaderWidth_,
    int    shaderHeight_,
    int    charWidth_,
    int    charHeight_,
    int    yadj_ )

    : _xstep       ( float(charWidth_) / float(shaderWidth_) )
    , _ystep       ( float(charHeight_) / float(shaderHeight_) )
    , _yadjstep    ( float(yadj_) / float(shaderHeight_) )
    , shaderPath   ( shaderPath_ )
    , shaderWidth  ( shaderWidth_ )
    , shaderHeight ( shaderHeight_ )
    , charWidth    ( charWidth_ )
    , charHeight   ( charHeight_ )
{
    // Populate charTable.

    const int maxcol = int(shaderWidth / charWidth);

    int col = 0;
    int row = 0;

    for (int i = 0; i < 94; i++) {
        _charTable[i][0] = col * _xstep;
        _charTable[i][1] = row * _ystep + _yadjstep;
        _charTable[i][2] = col * _xstep + _xstep;
        _charTable[i][3] = row * _ystep + _yadjstep + _ystep;

        col++;
        if (col >= maxcol) {
            col = 0;
            row++;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

Font::~Font()
{
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Draw char at native screen resolution.
 */ 
void
Font::drawChar( int x, int y, int c )
{
    trap_R_DrawStretchPic(
        x,
        y,
        charWidth,
        charHeight,
        _charTable[c][0],
        _charTable[c][1],
        _charTable[c][2],
        _charTable[c][3],
        shader );
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Draw text at native screen resolution, and no linefeed or line wrap.
 */
int
Font::drawLine( int x, int y, const string& text, vec4_t& color )
{
    int nchars = 0;
    trap_R_SetColor( color );

    const string::size_type max = text.length();
    for (string::size_type i = 0; i < max; i++) {
        nchars++;
        const char c = text[i];
        if (c == Q_COLOR_ESCAPE && (i+1 < max) && text[i+1] != Q_COLOR_ESCAPE) {
            trap_R_SetColor( g_color_table[ (text[i+1] - '0') & 0x1f ] );
            i++;
            continue;
        }

        if (c > 32 && c < 127)
            drawChar( x, y, c - 33 );

        x += charWidth;
        if ((x + charWidth) > cgs.glconfig.vidWidth)
            break;
    }

    return nchars;
}

///////////////////////////////////////////////////////////////////////////////

void
Font::registerShader()
{
    shader = trap_R_RegisterShaderNoMip( shaderPath.c_str() );
}

///////////////////////////////////////////////////////////////////////////////

Font consoleFont         ( "fonts/6x13",          128, 64, 6, 13, -1 );
Font consoleFontShadowed ( "fonts/6x13_shadowed", 128, 64, 6, 13, -1 );
