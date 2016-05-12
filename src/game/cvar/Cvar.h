#ifndef GAME_CVAR_CVAR_H
#define GAME_CVAR_CVAR_H

///////////////////////////////////////////////////////////////////////////////

namespace objects {
    extern Cvar g_admin;
    extern Cvar g_adminLog;

    extern Cvar g_bulletmodeDebug;
    extern Cvar g_bulletmodeReference;
    extern Cvar g_bulletmodeTrail;

    extern Cvar g_hitmodeAntilag;
    extern Cvar g_hitmodeAntilagLerp;
    extern Cvar g_hitmodeDebug;
    extern Cvar g_hitmodeFat;
    extern Cvar g_hitmodeGhosting;
    extern Cvar g_hitmodeReference;
    extern Cvar g_hitmodeZone;

    extern Cvar g_kickMessage;
    extern Cvar g_kickTime;
    extern Cvar g_protestMessage;

    extern Cvar g_maxLandmines;
    extern Cvar g_snap;
    extern Cvar g_shutdownExit;
    extern Cvar g_warmup;

    extern Cvar sv_tempBanMessage;

    extern Cvar g_test; // TODO: nuke when done with scale testing
} // namespace objects

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CVAR_CVAR_H
