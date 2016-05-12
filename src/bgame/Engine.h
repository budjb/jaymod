#ifndef BGAME_ENGINE_H
#define BGAME_ENGINE_H

///////////////////////////////////////////////////////////////////////////////

/* Static class for global ET engine functions.
 *
 * This class should only contain functions common to both game/cgame.
 * This class is static and cgame/game must implement Engine.cpp.
 */
class Engine
{
public:
    typedef int (QDECL* Ptr)( int, ... );

    static Ptr ptr;

    static size_t argc ( );
    static size_t argl ( string& );
    static size_t args ( vector<string>& );
    static size_t args ( vector<string>&, const string& );
    static size_t argv ( size_t, string& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_ENGINE_H
