#ifndef GAME_G_JAYMOD_H
#define GAME_G_JAYMOD_H

///////////////////////////////////////////////////////////////////////////////

#define ISONTEAM(x) (x->client->sess.sessionTeam == TEAM_AXIS || x->client->sess.sessionTeam == TEAM_ALLIES)

// Banner defines
#define BANNER_MAXLEN	        256
#define BANNER_MINTIME	        5

// g_headshot
#define HEADSHOT_ONLY		    1
#define HEADSHOT_INSTAGIB	    2

// g_slashKill
#define SLASHKILL_HALFCHARGE	1
#define SLASHKILL_ZEROCHARGE	2
#define SLASHKILL_SAMECHARGE	4
#define SLASHKILL_NOKILL		8

#define XPCAP_ALLOWGAIN		    0
#define	XPCAP_STOPGAIN		    1
#define	XPCAP_RESETXP		    2

#define XPSAVE_ENABLE           1
#define XPSAVE_RESETCAMPAIGN    2

// Jaymod Cvars
extern vmCvar_t sv_uptime;
extern vmCvar_t sv_uptimeStamp;
extern vmCvar_t	g_privateMessages;
extern vmCvar_t	g_playDead;
extern vmCvar_t	g_dragCorpse;
extern vmCvar_t	g_shove;
extern vmCvar_t	g_classChange;
extern vmCvar_t	team_maxArtillery;
extern vmCvar_t g_killingSpree;
extern vmCvar_t	g_xpSave;
extern vmCvar_t g_xpSaveTimeout;
extern vmCvar_t	g_xpCap;
extern vmCvar_t	g_xpMax;
extern vmCvar_t g_ammoRechargeTime;
extern vmCvar_t g_healthRechargeTime;
extern vmCvar_t g_packDistance;
extern vmCvar_t g_dropHealth;
extern vmCvar_t g_dropAmmo;
extern vmCvar_t	g_saveCampaignStats;
extern vmCvar_t g_intermissionTime;
extern vmCvar_t g_intermissionReadyPercent;
extern vmCvar_t	g_spectator;
extern vmCvar_t g_spawnInvul;
extern vmCvar_t	g_shoveNoZ;
extern vmCvar_t g_mapConfigs;
extern vmCvar_t	g_slashKill;

extern vmCvar_t g_censor;
extern vmCvar_t g_censorPenalty;

extern vmCvar_t g_moverScale;
extern vmCvar_t g_watermark;
extern vmCvar_t g_watermarkFadeAfter;
extern vmCvar_t g_watermarkFadeTime;
extern vmCvar_t g_goomba;
extern vmCvar_t	g_shortcuts;
extern vmCvar_t g_truePing;
extern vmCvar_t	sv_fps;
extern vmCvar_t	g_shoutcastpassword;
extern vmCvar_t	g_damagexp;
extern vmCvar_t g_mapScriptDirectory;
extern vmCvar_t	g_logOptions;
extern vmCvar_t g_vulnerableWeapons;
extern vmCvar_t	g_fear;
extern vmCvar_t	g_reflectFriendlyFire;
extern vmCvar_t	g_teamDamageRestriction;
extern vmCvar_t	g_teamDamageMinHits;
extern vmCvar_t	vote_voteBased;
extern vmCvar_t	vote_minPercent;
extern vmCvar_t g_muteTime;
extern vmCvar_t g_antiwarp;

// Class specific cvars
extern vmCvar_t	g_engineers;
extern vmCvar_t	g_soldiers;

extern vmCvar_t g_medicSelfHealDelay;
extern vmCvar_t	g_medics;

// Custom Levels
extern vmCvar_t g_levels_battlesense;
extern vmCvar_t g_levels_covertops;
extern vmCvar_t g_levels_engineer;
extern vmCvar_t g_levels_fieldops;
extern vmCvar_t g_levels_lightweapons;
extern vmCvar_t g_levels_medic;
extern vmCvar_t g_levels_soldier;
extern vmCvar_t g_defaultSkills;
extern vmCvar_t g_headshot;
extern vmCvar_t g_campaignFile;

// Killing Sprees
extern vmCvar_t g_killSpreeLevels;
extern vmCvar_t g_loseSpreeLevels;

// Banners
extern vmCvar_t g_bannerLocation;
extern vmCvar_t g_bannerTime;
extern vmCvar_t g_banners;

///////////////////////////////////////////////////////////////////////////////

#define G_SNAP_CLIENT_ENDFRAME     0x0001
#define G_SNAP_CLIENT_THINK        0x0002
#define G_SNAP_CLIENT_EXTRAPOLATE  0x0004
#define G_SNAP_ITEMS_POS           0x0008
#define G_SNAP_ITEMS_APOS          0x0010

#define G_TEST_SKIP_EINFO      0x0001  // TODO: nuke when done with scale testing
#define G_TEST_SKIP_TINFO      0x0002  // TODO: nuke when done with scale testing
#define G_TEST_SKIP_SC         0x0004  // TODO: nuke when done with scale testing
#define G_TEST_SKIP_HITSOUNDS  0x0008  // TODO: nuke when done with scale testing

#define G_TEST_SKIP_FIRE       0x0010  // TODO: nuke when done with scale testing
#define G_TEST_SKIP_LINK       0x0020  // TODO: nuke when done with scale testing
#define G_TEST_LOG_SETCS       0x0040  // TODO: nuke when done with scale testing
#define G_TEST_LOG_SETUI       0x0080  // TODO: nuke when done with scale testing

///////////////////////////////////////////////////////////////////////////////

namespace cache {
    extern string kickDuration;
    extern string kickMessage;
    extern int    kickTime;
} // namespace cache

///////////////////////////////////////////////////////////////////////////////

#define CH_KNIFE_DIST		64
#define CH_LADDER_DIST		100
#define CH_WATER_DIST		100
#define CH_BREAKABLE_DIST	64
#define CH_DOOR_DIST		96
#define CH_ACTIVATE_DIST	96
#define CH_EXIT_DIST		256
#define CH_FRIENDLY_DIST	1024
#define CH_REVIVE_DIST		64		// tjw: used to be 48 before smaller hitbox
#define CH_MAX_DIST			1024    // use the largest value from above
#define CH_MAX_DIST_ZOOM	8192    // max dist for zooming hints

// g_jaymod.c
#define CNSRPNLTY_KILL    1     // Gibs unless CNSRPNLTY_NOGIB
#define CNSRPNLTY_KICK    2     // Kick if the word is in their name
#define CNSRPNLTY_NOGIB   4     // Won't GIB, only kill
#define CNSRPNLTY_TMPMUTE 8     // Muted the next g_censorMuteTime.integer seconds

// Jaybird g_slashKill defines
// Shrub had these bitflagged, but that doesn't make sense.
// You can only use one of these at a time!
#define KILL_HALFBAR			1
#define KILL_NOBAR				2
#define KILL_SAMEBAR			3
#define KILL_DISABLE			4

// Jaybird - Class modification defines
// g_medics
#define MEDIC_LESSREGEN       4
#define MEDIC_NOREGEN         8
#define MEDIC_SHAREADRENALINE 16
#define MEDIC_DELAYREGEN      32

// g_engineers
#define ENGI_FRIENDLYMINES    1
#define ENGI_FRIENDLYDYNO     2
#define ENGI_SHAREXP          4

// g_soldiers
#define SDR_PANZERGRAV        1

#define VULN_PANZER   1
#define VULN_GRENADE  2
#define VULN_CANISTER 4
#define VULN_SATCHEL  8

// g_spectator
#define SPEC_CLICK     1
#define SPEC_CLICKMISS 2
#define SPEC_PERSIST   4
#define SPEC_FREE      8

#define LOGOPTS_CHAT           1
#define LOGOPTS_EXTENDED_WEAPS 2
#define LOGOPTS_BAN            8

/*
===================
g_jaymod.cpp
===================
*/
void     G_ApplyCustomRanks      ( gclient_t* );
void     G_AuthCheck             ( gentity_t*, char* );
void     G_Banners               ( );
void     G_BinocWar              ( qboolean );
void     G_ChatShortcuts         ( gentity_t* , string& );
void     G_FallDamage            ( gentity_t*, int );
void     G_InitCustomLevels      ( );
void     G_PlayDead              ( gentity_t* );
void     G_PlaySound_Cmd         ( );
void     G_PlaySoundEnv_Cmd      ( );
void     G_PrivateMessage        ( gentity_t* );
int      G_SkillForMOD           ( int );
void     G_TeamDamageRestriction ( gentity_t* );
int      G_TeamMaxArtillery      ( );
int      G_TeamMaxLandMines      ( );
void     ThrowingKnifeTouch      ( gentity_t*, gentity_t*, trace_t* );
void     G_Update_CS_Airstrikes  ( );
void     G_UpdateJaymodCS        ( );
void     G_UpdateUptime          ( );
qboolean IsReflectable           ( int );
bool     G_MutePlayer            ( gentity_t*, string, string = "" );
bool     G_UnmutePlayer          ( gentity_t* );
void     G_BanPlayer             ( gentity_t*, string, string, int );

// Poison
void     G_ResetPoisonEvent      ( gentity_t*, int );
void     G_ResetPoisonEvents     ( gentity_t* );
void     G_AddPoisonEvent        ( gentity_t*, gentity_t* );
void     G_RunPoisonEvents       ( gentity_t* );
void     G_ResetOnePoisonEvent   ( gentity_t* );
qboolean G_IsPoisoned            ( gentity_t* );

// Shoutcaster
void     G_MakeShoutcaster       ( );
void     G_RemoveShoutcaster     ( );
void     G_ShoutcasterLogin      ( gentity_t* );
void     G_ShoutcasterLogout     ( gentity_t* );

// ps.persistant flags
qboolean G_HasJayFlag            ( gentity_t*, int, int );
void     G_SetJayFlag            ( gentity_t*, int, int );
void     G_RemoveJayFlag         ( gentity_t*, int, int );

// +activate functions
void     G_AddClassSpecificTools ( gclient_t* );
void     G_ClassStealFixWeapons  ( gentity_t*, gentity_t* );
qboolean G_ClassSteal            ( gentity_t*, gentity_t* );
qboolean G_DragCorpse            ( gentity_t*, gentity_t* );
qboolean G_LiveUniformSteal      ( gentity_t*, gentity_t* );
qboolean G_PushPlayer            ( gentity_t*, gentity_t* );
qboolean G_UniformRestore        ( gentity_t*, gentity_t* );
qboolean G_UniformSteal          ( gentity_t*, gentity_t* );

// Throwing Knives

/*
===================
g_jaymod_utils.cpp
===================
*/
int      ClientNumbersFromString ( char*, int* );
void     G_ClientSound           ( gentity_t*, const char* );
void     G_ShakeClient           ( gentity_t*, int );
void     G_StripIPPort           ( string& );
char*    G_WeaponNumToStr        ( weapon_t );
void     G_DbLoad                ( );

// argc/argv manipulation
char*    Q_AddCR                 ( char* );
char*    Q_SayConcatArgs         ( int );

/*
===================
g_killingspree.cpp
===================
*/
#define KS_ENABLE 1	// Dummy
#define KS_RECORD 2

// Custom killingspree levels
extern	int killspreeLevels[6];
extern	int losespreeLevels[6];

// First/Last Kill
void G_FirstBlood                ( gentity_t* );
void G_UpdateLastKill            ( gentity_t* );
void G_DisplayLastKill           ( );

// Killing Sprees
void G_InitKillSpreeLevels       ( );
void G_AddKillSpree              ( gentity_t* );
void G_EndKillSpree              ( gentity_t*, gentity_t*, int );
void G_AddLoseSpree              ( gentity_t* );
void G_EndLoseSpree              ( gentity_t* );
void G_AddMultiKill              ( gentity_t* );

void G_LSFinalizeMap             ( );

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_G_JAYMOD_H
