/*
 * We must guarantee the order of initialization for certain static objects.
 * In order to do that, all objects with any such requirements are placed in
 * this central file.
 */

#include <bgame/impl.h>
#include <base/static.cpp.inc>

///////////////////////////////////////////////////////////////////////////////
//                       
//   _____   ____ _ _ __ 
//  / __\ \ / / _` | '__|
// | (__ \ V / (_| | |   
//  \___| \_/ \__,_|_|   
//                       
///////////////////////////////////////////////////////////////////////////////

namespace cvar {
    list<Cvar*> Cvar::REGISTRY;
    list<Cvar*> Cvar::UPDATE;

///////////////////////////////////////////////////////////////////////////////
//                                     _     _           _       
//   _____   ____ _ _ __   _ _    ___ | |__ (_) ___  ___| |_ ___ 
//  / __\ \ / / _` | '__| (_|_)  / _ \| '_ \| |/ _ \/ __| __/ __|
// | (__ \ V / (_| | |     _ _  | (_) | |_) | |  __/ (__| |_\__ |
//  \___| \_/ \__,_|_|    (_|_)  \___/|_.__// |\___|\___|\__|___/
//                                        |__/                   
///////////////////////////////////////////////////////////////////////////////

namespace objects {
    Cvar bg_cpu ( "cl_cpu", "", CVAR_ROM | CVAR_USERINFO );

    Cvar bg_dynamiteTime   ( "cg_dynamiteTime",   "0", CVAR_ROM );
    Cvar bg_glow           ( "cg_glow",           "0", CVAR_ROM );
    Cvar bg_misc           ( "cg_misc",           "0", CVAR_ROM );
    Cvar bg_panzerWar      ( "cg_panzerWar",      "0", CVAR_ROM );
    Cvar bg_poisonSyringes ( "cg_poisonSyringes", "0", CVAR_ROM );
    Cvar bg_skills         ( "cg_skills",         "0", CVAR_ROM );
    Cvar bg_sniperWar      ( "cg_sniperWar",      "0", CVAR_ROM );
    Cvar bg_weapons        ( "cg_weapons",        "0", CVAR_ROM );
    Cvar bg_wolfrof        ( "cg_wolfrof",        "0", CVAR_ROM );

    Cvar bg_maxEngineers   ( "cg_maxEngineers", "-1", CVAR_ROM );
    Cvar bg_maxMedics      ( "cg_maxMedics",    "-1", CVAR_ROM );
    Cvar bg_maxFieldOps    ( "cg_maxFieldOps",  "-1", CVAR_ROM );
    Cvar bg_maxCovertOps   ( "cg_maxCovertOps", "-1", CVAR_ROM );

    Cvar bg_maxFlamers       ( "cg_maxFlamers",       "-1", CVAR_ROM );
    Cvar bg_maxPanzers       ( "cg_maxPanzers",       "-1", CVAR_ROM );
    Cvar bg_maxMG42s         ( "cg_maxMG42s",         "-1", CVAR_ROM );
    Cvar bg_maxMortars       ( "cg_maxMortars",       "-1", CVAR_ROM );
    Cvar bg_maxGrenLaunchers ( "cg_maxGrenLaunchers", "-1", CVAR_ROM );
    Cvar bg_maxM97s          ( "cg_maxM97s",          "-1", CVAR_ROM );

    Cvar bg_sk5_battle    ( "cg_sk5_battle",    "0", CVAR_ROM );
    Cvar bg_sk5_lightweap ( "cg_sk5_lightweap", "0", CVAR_ROM );
    Cvar bg_sk5_cvops     ( "cg_sk5_cvops",     "0", CVAR_ROM );
    Cvar bg_sk5_eng       ( "cg_sk5_eng",       "0", CVAR_ROM );
    Cvar bg_sk5_fdops     ( "cg_sk5_fdops",     "0", CVAR_ROM );
    Cvar bg_sk5_medic     ( "cg_sk5_medic",     "0", CVAR_ROM );
    Cvar bg_sk5_soldier   ( "cg_sk5_soldier",   "0", CVAR_ROM );

    Cvar bg_bulletmode ( "cg_bulletmode", "0", CVAR_ROM );
    Cvar bg_hitmode    ( "cg_hitmode",    "0", CVAR_ROM );

    Cvar bg_ammoUnlimited      ( "cg_ammoUnlimited",      "0", CVAR_ROM );
    Cvar bg_ammoFireDelayNudge ( "cg_ammoFireDelayNudge", "0", CVAR_ROM );
    Cvar bg_ammoNextDelayNudge ( "cg_ammoNextDelayNudge", "0", CVAR_ROM );

    Cvar bg_covertops( "cg_covertops", "0", CVAR_ROM );

    Cvar bg_fixedphysics    ( "cg_fixedphysics",       "1",   CVAR_ROM );
    Cvar bg_fixedphysicsfps ( "cg_fixedphysicsfps",    "125", CVAR_ROM );

    Cvar bg_proneDelay      ( "cg_proneDelay",         "0",   CVAR_ROM );

    Cvar gameState( "gameState", "-1", CVAR_ROM );

///////////////////////////////////////////////////////////////////////////////

    Cvar cl_mac        ( "cl_mac",        "", CVAR_ROM | CVAR_USERINFO );
    Cvar cl_maxpackets ( "cl_maxpackets", "30", 0, cb_cl_maxpackets );
    Cvar cl_rate       ( "rate",          "" );
    Cvar cl_snaps      ( "snaps",         "",   0, cb_cl_snaps );
    Cvar cl_timeNudge  ( "cl_timeNudge",  "0",  0, cb_cl_timeNudge );

    Cvar sv_fps     ( "sv_fps",     "20", 0, cb_sv_fps );
    Cvar sv_maxRate ( "sv_maxRate", "25000" );
} // namespace objects

///////////////////////////////////////////////////////////////////////////////

} // namespace cvar
