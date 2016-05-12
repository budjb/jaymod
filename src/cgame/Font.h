#ifndef CGAME_FONT_H
#define CGAME_FONT_H

///////////////////////////////////////////////////////////////////////////////

class Font
{
private:
    Font();  // not permitted

    float _charTable[94][4];  // ASCII { 33..126 } st-coords

    const float _xstep;
    const float _ystep;
    const float _yadjstep;

protected:
    void drawChar( int, int, int );

public:
    Font( const string&, int, int, int, int, int = 0 );
    ~Font();

    int drawLine( int, int, const string&, vec4_t& = colorWhite );
    void registerShader();

    qhandle_t    shader;
    const string shaderPath;

    const int shaderWidth;
    const int shaderHeight;
    const int charWidth;
    const int charHeight;
};

///////////////////////////////////////////////////////////////////////////////

extern Font consoleFont;
extern Font consoleFontShadowed;

///////////////////////////////////////////////////////////////////////////////

#endif // CGAME_FONT_H
