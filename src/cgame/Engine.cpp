#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace {
    static char argbuf[MAX_STRING_CHARS];
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::argc()
{
    return ptr( CG_ARGC );
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::argl( string& out )
{
    ptr( CG_ARGS, argbuf, sizeof(argbuf) );
    out = argbuf;
    return out.length();
}
///////////////////////////////////////////////////////////////////////////////

size_t
Engine::args( vector<string>& out )
{
    const size_t count = ptr( CG_ARGC );
    out.resize( count );
    for (size_t i = 0; i < count; i++) {
        ptr( CG_ARGV, i, argbuf, sizeof(argbuf) );
        out[i] = argbuf;
    }

    return out.size();
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::argv( size_t n, string& out )
{
    ptr( CG_ARGV, n, argbuf, sizeof(argbuf) );
    out = argbuf;
    return out.length();
}   
