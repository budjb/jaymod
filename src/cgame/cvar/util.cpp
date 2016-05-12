#include <bgame/impl.h>

namespace cvar {

///////////////////////////////////////////////////////////////////////////////

void cb_cl_maxpackets( Cvar& var )
{
    needClientFlagsUpdated = true;
}

///////////////////////////////////////////////////////////////////////////////

void cb_cl_snaps( Cvar& var )
{
    if (var.ivalue != cvars::sv_fps.ivalue)
        var.set( cvars::sv_fps.ivalue );
}

///////////////////////////////////////////////////////////////////////////////

void cb_cl_timeNudge( Cvar& var )
{
    needClientFlagsUpdated = true;

    /* cl_timenudge less than -50 or greater than 50 doesn't actually
     * do anything more than -50 or 50 (actually the numbers are probably
     * closer to -30 and 30, but 50 is nice and round-ish)
     * might as well not feed the myth, eh?
     */
    if (var.ivalue < -50)
        var.set( -50 );
    else if (var.ivalue > 50)
        var.set( 50 );
}

///////////////////////////////////////////////////////////////////////////////

void cb_sv_fps( Cvar& var )
{
    if (var.ivalue != cvars::cl_snaps.ivalue)
        cvars::cl_snaps.set( var.ivalue );
}

///////////////////////////////////////////////////////////////////////////////

} // namespace str
