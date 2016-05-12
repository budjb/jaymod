/*
 * NOTE: no OSX support at this time 
 */

#include <game/g_local.h>
#include <omnibot/common/BotExports.h>

///////////////////////////////////////////////////////////////////////////////

eomnibot_error
Omnibot_LoadLibrary( const int version, const char* const libbase, const char* const customdir )
{
    return BOT_ERROR_CANTLOADDLL;
}

///////////////////////////////////////////////////////////////////////////////

void
Omnibot_FreeLibrary()
{
}
