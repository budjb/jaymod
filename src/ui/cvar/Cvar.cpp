#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

void
Cvar::update()
{
    const list<Cvar*>::iterator end = Cvar::UPDATE.end();
    for ( list<Cvar*>::iterator it = Cvar::UPDATE.begin(); it != end; it++ )
        (*it)->trapUpdate();
}
