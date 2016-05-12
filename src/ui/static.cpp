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
    Cvar bg_sniperWar ( "ui_sniperWar", "0", CVAR_ROM );
    Cvar bg_weapons   ( "ui_weapons",   "0", CVAR_ROM );

    Cvar bg_sk5_cvops   ( "ui_sk5_cvops",   "0", CVAR_ROM );
    Cvar bg_sk5_eng     ( "ui_sk5_eng",     "0", CVAR_ROM );
    Cvar bg_sk5_fdops   ( "ui_sk5_fdops",   "0", CVAR_ROM );
    Cvar bg_sk5_medic   ( "ui_sk5_medic",   "0", CVAR_ROM );
    Cvar bg_sk5_soldier ( "ui_sk5_soldier", "0", CVAR_ROM );
} // namespace objects

///////////////////////////////////////////////////////////////////////////////

} // namespace cvar
