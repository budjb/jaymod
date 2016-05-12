#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

void
SEngine::dropClient( int slot, const text::Buffer& msg, const string& action, int duration )
{
    string text;
    str::toDropMessage( text, false, msg, action );
    Engine::ptr( G_DROP_CLIENT, slot, text.c_str(), duration );
}
