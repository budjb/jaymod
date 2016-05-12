changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
dnl
dnl
//////////////////////////////////////////////////////////////////////
//
// __title (r<<>>__repoLCRev)
//
//
// SAMPLE CONFIGURATION FILE.
//
// If you have any questions regarding a specific cvar, please
// consult the bundled documentation.
//
// __copyright
// __website
// __irc
//
//
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//
// SECURITY
//
//////////////////////////////////////////////////////////////////////

set g_password           ""
set g_shoutcastpassword  ""
set rconpassword         ""
set refereePassword      ""
set sv_privatePassword   ""

//////////////////////////////////////////////////////////////////////
//
// LOGGING
//
//////////////////////////////////////////////////////////////////////

set g_log         ""
set g_logOptions  "0"
set g_logSync     "0"
set g_adminLog    ""

//////////////////////////////////////////////////////////////////////
//
// BRANDING
//
//////////////////////////////////////////////////////////////////////

//sets .NAME  "MyClan"
//sets .URL   "http://www.myserver.net/"

set sv_hostname  "ETHost"
set g_watermark  ""

set g_protestMessage  "Visit www.myserver.com to file a protest."
set g_kickMessage     "You have been kicked for $TIME."
set g_kickTime        "2m"

//////////////////////////////////////////////////////////////////////
//
// MOTD
//
//////////////////////////////////////////////////////////////////////

set server_motd0  ""
set server_motd1  ""
set server_motd2  ""
set server_motd3  ""
set server_motd4  ""
set server_motd5  ""

//////////////////////////////////////////////////////////////////////
//
// REGISTRATION
//
//////////////////////////////////////////////////////////////////////

set sv_master1  "etmaster.idsoftware.com"
set sv_master2  ""
set sv_master3  ""
set sv_master4  ""
set sv_master5  ""

//////////////////////////////////////////////////////////////////////
//
// NETWORKING
//
//////////////////////////////////////////////////////////////////////

set sv_allowDownload      "1"
set sv_dl_maxRate         "42000"
set sv_floodProtect       "1"
set sv_fps                "20"
set sv_fullmsg            "Server is full."
set sv_lanForceRate       "1"
set sv_maxPing            "0"
set sv_maxRate            "13000"
set sv_maxclients         "20"
set sv_minPing            "0"
set sv_packetdelay        "0"
set sv_packetloss         "0"
set sv_padPackets         "0"
set sv_privateClients     "4"
set sv_pure               "1"
set sv_reconnectlimit     "3"
set sv_showAverageBPS     "0"
set sv_showloss           "0"
set sv_timeout            "240"
set sv_wwwBaseURL         ""
set sv_wwwDlDisconnected  "0"
set sv_wwwDownload        "0"
set sv_wwwFallbackURL     ""
set sv_zombietime         "2"

//////////////////////////////////////////////////////////////////////
//
// VOTING
//
//////////////////////////////////////////////////////////////////////

set vote_allow_balancedteams   "1"
set vote_allow_comp            "1"
set vote_allow_friendlyfire    "1"
set vote_allow_gametype        "1"
set vote_allow_generic         "1"
set vote_allow_kick            "1"
set vote_allow_map             "1"
set vote_allow_matchreset      "1"
set vote_allow_matchrestart    "1"
set vote_allow_mutespecs       "1"
set vote_allow_muting          "1"
set vote_allow_nextmap         "1"
set vote_allow_pub             "1"
set vote_allow_referee         "0"
set vote_allow_shuffleteamsxp  "1"
set vote_allow_swapteams       "1"
set vote_allow_timelimit       "0"
set vote_allow_warmupdamage    "1"
set vote_limit                 "5"
set vote_percent               "50"

//////////////////////////////////////////////////////////////////////
//
// BANNERS
//
//////////////////////////////////////////////////////////////////////

set g_bannerLocation  "4"
set g_bannerTime      "60"

set g_banners  "2"
set g_banner1  "^3THIS SERVER IS RUNNING __title"
set g_banner2  "^3Check forums at __website"

//////////////////////////////////////////////////////////////////////
//
// Matchplay
//
//////////////////////////////////////////////////////////////////////

set g_gametype      "2"
set g_campaignFile  ""

set g_headshot   "0"
set g_knifeonly  "0"
set g_panzerWar  "0"
set g_sniperWar  "0"

set match_latejoin       "1"
set match_minplayers     "0"
set match_mutespecs      "0"
set match_readypercent   "100"
set match_timeoutcount   "3"
set match_timeoutlength  "180"
set match_warmupDamage   "1"

//////////////////////////////////////////////////////////////////////
//
// TEAMS
//
//////////////////////////////////////////////////////////////////////

set g_userAlliedRespawnTime  "0"
set g_userAxisRespawnTime    "0"
set g_teamForceBalance       "1"

set g_ammoRechargeTime    "60000"
set g_healthRechargeTime  "10000"

set team_maxArtillery  "6"
set team_maxLandMines  "20"

set team_maxFlamers        "-1"
set team_maxGrenLaunchers  "-1"
set team_maxM97s           "-1"
set team_maxMG42s          "-1"
set team_maxMortars        "-1"
set team_maxPanzers        "-1"

set team_maxplayers  "0"

set team_maxMedics     "-1"
set team_maxEngineers  "-1"
set team_maxFieldOps   "-1"
set team_maxCovertOps  "-1"

set team_nocontrols  "1"

//////////////////////////////////////////////////////////////////////
//
// PLAYERS
//
//////////////////////////////////////////////////////////////////////

set g_defaultSkills "0 0 0 0 0 0 0"

set g_levels_battlesense   "20 50 90 140 200"
set g_levels_covertops     "20 50 90 140 200"
set g_levels_engineer      "20 50 90 140 200"
set g_levels_fieldops      "20 50 90 140 200"
set g_levels_lightweapons  "20 50 90 140 200"
set g_levels_medic         "20 50 90 140 200"
set g_levels_soldier       "20 50 90 140 200"

set g_covertops  "0"
set g_engineers  "0"
set g_medics     "0"
set g_soldiers   "0"

set g_sk5_battle     "1"
set g_sk5_cvops      "7"
set g_sk5_eng        "127"
set g_sk5_fdops      "3"
set g_sk5_lightweap  "1"
set g_sk5_medic      "243"
set g_sk5_soldier    "7"

set g_covertopsChargeTime  "30000"
set g_engineerChargeTime   "30000"
set g_LTChargeTime         "40000"
set g_soldierChargeTime    "20000"

set g_medicChargeTime     "45000"
set g_medicSelfHealDelay  "0"

//////////////////////////////////////////////////////////////////////
//
// BULLETMODE
//
//////////////////////////////////////////////////////////////////////

set g_bulletmode           "0"
set g_bulletmodeDebug      "0"
set g_bulletmodeReference  "1"
set g_bulletmodeTrail      "0"

//////////////////////////////////////////////////////////////////////
//
// HITMODE
//
//////////////////////////////////////////////////////////////////////

set g_hitmode             "0"
set g_hitmodeAntilag      "800"
set g_hitmodeAntilagLerp  "1"
set g_hitmodeDebug        "0"
set g_hitmodeFat          "0"
set g_hitmodeGhosting     "0"
set g_hitmodeReference    "1"
set g_hitmodeZone         "0"

//////////////////////////////////////////////////////////////////////
//
// MISCELLANEOUS
//
//////////////////////////////////////////////////////////////////////

set g_admin                     "1"
set g_alliedmaxlives            "0"
set g_altStopwatchMode          "0"
set g_antiwarp                  "1"
set g_autoFireteams             "0"
set g_axismaxlives              "0"
set g_censor                    "0"
set g_censorPenalty             "0"
set g_classChange               "0"
set g_complaintlimit            "6"
set g_damagexp                  "0"
set g_debugAlloc                "0"
set g_debugConstruct            "0"
set g_debugDamage               "0"
set g_debugMove                 "0"
set g_debugSkills               "0"
set g_disableComplaints         "0"
set g_dragCorpse                "1"
set g_dropAmmo                  "2"
set g_dropHealth                "2"
set g_dynamiteTime              "30"
set g_enforcemaxlives           "1"
set g_fastres                   "0"
set g_fear                      "0"
set g_filterBan                 "1"
set g_filtercams                "0"
set g_fixedPhysics              "1"
set g_fixedPhysicsFPS           "125"
set g_forcerespawn              "0"
set g_friendlyFire              "1"
set g_glow                      "0"
set g_goomba                    "4"
set g_gravity                   "800"
set g_heavyWeaponRestriction    "100"
set g_inactivity                "0"
set g_intermissionReadyPercent  "75"
set g_intermissionTime          "30"
set g_ipcomplaintlimit          "3"
set g_killSpreeLevels           "5 10 15 20 25 30"
set g_killingSpree              "1"
set g_knockback                 "1000"
set g_landminetimeout           "1"
set g_lms_followTeamOnly        "1"
set g_lms_lockTeams             "0"
set g_lms_matchlimit            "2"
set g_lms_roundlimit            "3"
set g_lms_teamForceBalance      "1"
set g_loseSpreeLevels           "10 20 30"
set g_mapConfigs                "mapconfigs"
set g_mapScriptDirectory        "mapscripts"
set g_maxGameClients            "0"
set g_maxlives                  "0"
set g_maxlivesRespawnPenalty    "0"
set g_misc                      "66"
set g_moverScale                "1.0"
set g_movespeed                 "76"
set g_muteTime                  "0"
set g_noTeamSwitching           "0"
set g_packDistance              "4"
set g_playDead                  "1"
set g_poisonSyringes            "1"
set g_proneDelay                "0"
set g_privateMessages           "1"
set g_reflectFriendlyFire       "100"
set g_saveCampaignStats         "1"
set g_scriptDebug               "0"
set g_scriptDebugLevel          "0"
set g_scriptName                ""
set g_shortcuts                 "0"
set g_shove                     "100"
set g_shoveNoZ                  "1"
set g_skills                    "0"
set g_slashKill                 "0"
set g_smoothClients             "1"
set g_snap                      "7"
set g_spawnInvul                "3"
set g_spectator                 "0"
set g_spectatorInactivity       "0"
set g_speed                     "320"
set g_teamDamageMinHits         "6"
set g_teamDamageRestriction     "0"
set g_truePing                  "1"
set g_voiceChatsAllowed         "4"
set g_vulnerableWeapons         "0"
set g_warmup                    "30"
set g_watermarkFadeAfter        "60"
set g_watermarkFadeTime         "60"
set g_weapons                   "5606"
set g_wolfrof                   "0"
set g_xpCap                     "0"
set g_xpMax                     "0"
set g_xpSave                    "1"
set g_xpSaveTimeout             "1h"
