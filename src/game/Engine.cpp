#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

namespace {
    static char argbuf[MAX_STRING_CHARS];
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::argc()
{
    return ptr( G_ARGC );
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::argl( string& out )
{
    ostringstream oss;

    vector<string> argList;
    const size_t max = args( argList );
    for ( size_t i = 0; i < max; i++ ) {
        if (i)
            oss << ' ';
        oss << argList[i];
    }

    out = oss.str();
    return out.length();
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::args( vector<string>& out )
{
    const size_t count = ptr( G_ARGC );
    out.resize( count );
    for (size_t i = 0; i < count; i++) {
        ptr( G_ARGV, i, argbuf, sizeof(argbuf) );
        out[i] = argbuf;
    }

    return out.size();
}

///////////////////////////////////////////////////////////////////////////////

size_t
Engine::argv( size_t n, string& out )
{
    ptr( G_ARGV, n, argbuf, sizeof(argbuf) );
    out = argbuf;
    return out.length();
}
