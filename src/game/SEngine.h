#ifndef GAME_SENGINE_H
#define GAME_SENGINE_H

///////////////////////////////////////////////////////////////////////////////

/* Static class for global ET engine functions.
 *
 * This class should only contain functions specific to game module.
 * This class is static game must implement Engine.cpp.
 */
class SEngine : public Engine
{
public:
    static void dropClient( int, const text::Buffer&, const string& = "", int = 0 );
};

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_SENGINE_H
