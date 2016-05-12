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
    Cvar bg_cpu ( "sv_cpu", "", CVAR_ROM | CVAR_SERVERINFO_NOUPDATE );

    Cvar bg_dynamiteTime    ( "g_dynamiteTime",   "30", CVAR_JAYMODINFO );
    Cvar bg_glow            ( "g_glow",           "0",  CVAR_JAYMODINFO );
    Cvar bg_misc            ( "g_misc",           "0",  CVAR_JAYMODINFO );
    Cvar bg_panzerWar       ( "g_panzerWar",      "0",  CVAR_JAYMODINFO );
    Cvar bg_poisonSyringes  ( "g_poisonSyringes", "0",  CVAR_JAYMODINFO );
    Cvar bg_skills          ( "g_skills",         "0",  CVAR_JAYMODINFO );
    Cvar bg_sniperWar       ( "g_sniperWar",      "0",  CVAR_JAYMODINFO );
    Cvar bg_weapons         ( "g_weapons",        "0",  CVAR_JAYMODINFO );
    Cvar bg_wolfrof         ( "g_wolfrof",        "0",  CVAR_JAYMODINFO );

    Cvar bg_maxEngineers    ( "team_maxEngineers","-1", CVAR_JAYMODINFO );
    Cvar bg_maxMedics       ( "team_maxMedics",   "-1", CVAR_JAYMODINFO );
    Cvar bg_maxFieldOps     ( "team_maxFieldOps", "-1", CVAR_JAYMODINFO );
    Cvar bg_maxCovertOps    ( "team_maxCovertOps","-1", CVAR_JAYMODINFO );

    Cvar bg_maxFlamers      ( "team_maxFlamers",  "-1", CVAR_JAYMODINFO );
    Cvar bg_maxPanzers      ( "team_maxPanzers",  "-1", CVAR_JAYMODINFO );
    Cvar bg_maxMG42s        ( "team_maxMG42s",    "-1", CVAR_JAYMODINFO );
    Cvar bg_maxMortars      ( "team_maxMortars",  "-1", CVAR_JAYMODINFO );
    Cvar bg_maxGrenLaunchers( "team_maxGrenLaunchers", "-1", CVAR_JAYMODINFO );
    Cvar bg_maxM97s         ( "team_maxM97s",     "-1", CVAR_JAYMODINFO );

    Cvar bg_sk5_battle    ( "g_sk5_battle",    "1",    CVAR_JAYMODINFO );
    Cvar bg_sk5_lightweap ( "g_sk5_lightweap", "1",    CVAR_JAYMODINFO );
    Cvar bg_sk5_cvops     ( "g_sk5_cvops",     "7",    CVAR_JAYMODINFO );
    Cvar bg_sk5_eng       ( "g_sk5_eng",       "127",  CVAR_JAYMODINFO );
    Cvar bg_sk5_fdops     ( "g_sk5_fdops",     "3",    CVAR_JAYMODINFO );
    Cvar bg_sk5_medic     ( "g_sk5_medic",     "243",  CVAR_JAYMODINFO );
    Cvar bg_sk5_soldier   ( "g_sk5_soldier",   "7",    CVAR_JAYMODINFO );

    Cvar bg_bulletmode    ( "g_bulletmode",    "0",    CVAR_JAYMODINFO );
    Cvar bg_hitmode       ( "g_hitmode",       "0",    CVAR_JAYMODINFO );

    Cvar bg_ammoUnlimited      ( "g_ammoUnlimited",      "0", CVAR_JAYMODINFO );
    Cvar bg_ammoFireDelayNudge ( "g_ammoFireDelayNudge", "0", CVAR_JAYMODINFO );
    Cvar bg_ammoNextDelayNudge ( "g_ammoNextDelayNudge", "0", CVAR_JAYMODINFO );

    Cvar bg_covertops          ( "g_covertops",          "0", CVAR_ARCHIVE | CVAR_JAYMODINFO );

    Cvar bg_fixedphysics       ( "g_fixedphysics",        "1",   CVAR_JAYMODINFO );
    Cvar bg_fixedphysicsfps    ( "g_fixedphysicsfps",     "125", CVAR_JAYMODINFO );

    Cvar bg_proneDelay         ( "g_proneDelay",          "0",   CVAR_JAYMODINFO );

    Cvar gameState( "gameState", "-1", CVAR_WOLFINFO | CVAR_ROM );

///////////////////////////////////////////////////////////////////////////////

    Cvar g_admin    ( "g_admin",    "", CVAR_ARCHIVE | CVAR_LATCH );
    Cvar g_adminLog ( "g_adminLog", "", CVAR_ARCHIVE, AdminLog::cvarCallback );

    Cvar g_bulletmodeDebug     ( "g_bulletmodeDebug",     "0", 0, NULL );
    Cvar g_bulletmodeReference ( "g_bulletmodeReference", "1", 0, NULL );
    Cvar g_bulletmodeTrail     ( "g_bulletmodeTrail",     "0", 0, AbstractBulletModel::cvarTrail );

    Cvar g_hitmodeAntilag      ( "g_hitmodeAntilag",     "800", 0, AbstractHitModel::cvarAntilag );
    Cvar g_hitmodeAntilagLerp  ( "g_hitmodeAntilagLerp",   "1", 0, AbstractHitModel::cvarAntilagLerp );
    Cvar g_hitmodeDebug        ( "g_hitmodeDebug",         "0", 0, NULL );
    Cvar g_hitmodeFat          ( "g_hitmodeFat",           "0", 0, AbstractHitModel::cvarFat );
    Cvar g_hitmodeGhosting     ( "g_hitmodeGhosting",      "0", 0, AbstractHitModel::cvarGhosting );
    Cvar g_hitmodeReference    ( "g_hitmodeReference",     "1", 0, NULL );
    Cvar g_hitmodeZone         ( "g_hitmodeZone",          "1", 0, AbstractHitModel::cvarZone );

    Cvar g_maxLandmines ( "team_maxLandmines", "10" );
    Cvar g_shutdownExit ( "g_shutdownExit",    "0" );
    Cvar g_snap         ( "g_snap",            "7" );
    Cvar g_warmup       ( "g_warmup",          "60", 0, cb_g_warmup );

    Cvar g_kickMessage    ( "g_kickMessage",    "You have been kicked for ^G$TIME^*." );
    Cvar g_kickTime       ( "g_kickTime",       "2m", CVAR_JAYMODCB_INIT, cb_g_kickTime );
    Cvar g_protestMessage ( "g_protestMessage", "Visit ^/www.myserver.com^* to file a protest" );

    Cvar sv_tempBanMessage( "sv_tempBanMessage", "", CVAR_ROM );

    Cvar g_test( "g_test", "0", 0, NULL );  // TODO: nuke when done with scale testing
} // namespace objects

///////////////////////////////////////////////////////////////////////////////

} // namespace cvar

///////////////////////////////////////////////////////////////////////////////
//  ____       _       _ _                 
// |  _ \ _ __(_)_   _(_) | ___  __ _  ___ 
// | |_) | '__| \ \ / / | |/ _ \/ _` |/ _ |
// |  __/| |  | |\ V /| | |  __/ (_| |  __/
// |_|   |_|  |_| \_/ |_|_|\___|\__, |\___|
//                              |___/      
///////////////////////////////////////////////////////////////////////////////

PrivilegeSet PrivilegeSet::REGISTRY;

namespace priv {
    namespace pseudo {
        const Privilege
            all       ( Privilege::TYPE_PSEUDO, "all" ),
            behaviors ( Privilege::TYPE_PSEUDO, "behaviors" ),
            commands  ( Privilege::TYPE_PSEUDO, "commands" );
    } // namespace pseudo

    namespace base {
        const Privilege 
            balanceImmunity ( Privilege::TYPE_BEHAVIORAL, "balanceImmunity" ),
            banPermanent    ( Privilege::TYPE_BEHAVIORAL, "banPermanent" ), 
            censorImmunity  ( Privilege::TYPE_BEHAVIORAL, "censorImmunity" ),
            commandChat     ( Privilege::TYPE_BEHAVIORAL, "commandChat" ),
            commandSilent   ( Privilege::TYPE_BEHAVIORAL, "commandSilent" ),
            reasonNone      ( Privilege::TYPE_BEHAVIORAL, "reasonNone" ),
            specChat        ( Privilege::TYPE_BEHAVIORAL, "specChat" ),
            voteAny         ( Privilege::TYPE_BEHAVIORAL, "voteAny" ),
            voteImmunity    ( Privilege::TYPE_BEHAVIORAL, "voteImmunity" );
    } // namespace base
} // namespace priv

///////////////////////////////////////////////////////////////////////////////
//                     _ 
//   ___ _ __ ___   __| |
//  / __| '_ ` _ \ / _` |
// | (__| | | | | | (_| |
//  \___|_| |_| |_|\__,_|
//                      
///////////////////////////////////////////////////////////////////////////////

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

InlineText AbstractCommand::_ovalue;

///////////////////////////////////////////////////////////////////////////////

namespace {
    // cross-platform compatible shared-library static init function
    class Init0 {
    public:
        Init0() {
            AbstractCommand::_ovalue = xcvalue;
            AbstractCommand::_ovalue.prefixOutside = '[';
            AbstractCommand::_ovalue.suffixOutside = ']';
        }
    } init0;
} // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

Registry registry;

///////////////////////////////////////////////////////////////////////////////

namespace builtins {
    About        about;
    AdminTest    adminTest;
    Ban          ban;
    BanInfo      banInfo;
    BanList      banList;
    BanUser      banUser;
    CancelVote   cancelVote;
    Chicken      chicken;
    CrazyGravity crazygravity;
    CryBaby      crybaby;
    DbLoad       dbLoad;
    DbSave       dbSave;
    Disorient    disorient;
    FTime        ftime;
    Finger       finger;
    Fling        fling;
    Flinga       flinga;
    Glow         glow;
    Help         help;
    Kick         kick;
    Launch       launch;
    Launcha      launcha;
    LevAdd       levAdd;
    LevDelete    levDelete;
    LevEdit      levEdit;
    LevInfo      levInfo;
    LevList      levList;
    ListPlayers  listPlayers;
    Lock         lock;
    Lol          lol;
    LsPlayers    lsPlayers;
    Mute         mute;
    News         news;
    NextMap      nextMap;
    Orient       orient;
    Page         page;
    Pants        pants;
    PanzerWar    panzerwar;
    Pause        pause;
    Pip          pip;
    Pop          pop;
    PutTeam      putTeam;
    Rename       rename;
    Reset        reset;
    ResetXp      resetXp;
    ResetmyXp    resetmyXp;
    Restart      restart;
    Revive       revive;
    Seen         seen;
    SetLevel     setLevel;
    Shake        shake;
    Shuffle      shuffle;
    Slap         slap;
    Smite        smite;
    SniperWar    sniperwar;
    Spec         spec;
    Spec999      spec999;
    Splat        splat;
    Splata       splata;
    Status       status;
    Swap         swap;
    Throw        throw_;
    Throwa       throwa;
    Time         time;
    Unban        unban;
    Unlock       unlock;
    Unmute       unmute;
    Unpause      unpause;
    Uptime       uptime;
    UserDelete   userDelete;
    UserEdit     userEdit;
    UserInfo     userInfo;
    UserList     userList;
} // namespace builtin

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
