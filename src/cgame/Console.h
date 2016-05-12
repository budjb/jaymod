#ifndef CGAME_CONSOLE_H
#define CGAME_CONSOLE_H

///////////////////////////////////////////////////////////////////////////////

class Console
{
private:
    deque<string> _lines;

public:
    Console();
    ~Console();

    void draw();
    void init();
    void print( const char* );

    Font font;
    Font fontShadowed;
};

///////////////////////////////////////////////////////////////////////////////

extern vmCvar_t cg_console;
extern vmCvar_t cg_consoleShadowed;
extern Console  console;

///////////////////////////////////////////////////////////////////////////////

#endif // CGAME_CONSOLE_H
