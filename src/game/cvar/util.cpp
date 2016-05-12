#include <bgame/impl.h>

namespace cvar {

///////////////////////////////////////////////////////////////////////////////

void
cb_g_kickTime( Cvar& cvar )
{
    static const string varName = "$TIME";

    string s = objects::g_kickMessage.svalue;
    const string::size_type fpos = s.find( varName );
    if (fpos == string::npos)
        return;

    cache::kickTime = str::toSeconds( cvar.svalue );
    cache::kickDuration = str::toStringSecondsRemaining( cache::kickTime, true );

    s.replace( fpos, varName.length(), cache::kickDuration );
    cache::kickMessage = s;

    // also set this cvar for ET engine generated messages when reconnecting after kick
    objects::sv_tempBanMessage.set( cache::kickMessage );
}

///////////////////////////////////////////////////////////////////////////////

void
cb_g_warmup( Cvar& cvar )
{
    if (objects::g_warmup.ivalue > 0 && objects::gameState.ivalue != GS_PLAYING) {
        level.warmupEndTime = level.time + (objects::g_warmup.ivalue * 1000);
        trap_SetConfigstring( CS_WARMUP, va( "%i", level.warmupEndTime ));
    }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace str
