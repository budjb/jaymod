#ifndef BGAME_JAYMOD_H
#define BGAME_JAYMOD_H

///////////////////////////////////////////////////////////////////////////////

// Various things both games should see

// Shotgun
#define M97_COUNT	11
#define	M97_SPREAD	700

// g_weapons defines
#define SBW_FOPS           0x0001 //    1
#define SBW_MEDIC          0x0002 //    2
#define SBW_ENGI           0x0004 //    4
#define SBW_FULLBAR        0x0008 //    8
#define SBW_HALFBAR        0x0010 //   16
#define	SBW_HELMET         0x0020 //   32
#define SBW_DROPBINOCS     0x0040 //   64
#define SBW_FAIRRIFLES     0x0080 //  128
#define	SBW_THKNIVES       0x0100 //  256
#define SBW_PSTHKNIVES     0x0200 //  512
#define SBW_M97            0x0400 // 1024
#define SBW_NOADRENALINE   0x0800 // 2048
#define SBW_MOLOTOV        0x1000 // 4096

// Class carryovers
#define SBS_COPS		1
#define SBS_ENGI		2
#define SBS_MEDI		4
#define SBS_FOPS		8

// Skill-5 benefit bit-flags.
#define SK5_BAT_SPRINT      0x0001 // faster stamina recharge

#define SK5_LWP_RECOIL      0x0001 // reduced recoil time

#define SK5_CVO_CHARGE      0x0001 // consume less charge
#define SK5_CVO_GRENADES    0x0002 // spawn/capacity 4 grenades
#define SK5_CVO_POISON      0x0004 // enable poison gas

#define SK5_ENG_CHARGE      0x0001 // consume less charge
#define SK5_ENG_GRENADES    0x0002 // spawn/capacity 10 grenades
#define SK5_ENG_MINE_SPOT   0x0004 // landmines take longer to spot
#define SK5_ENG_MINE_DEFUSE 0x0008 // landmines take longer to defuse
#define SK5_ENG_CONSTRUCT   0x0010 // build things faster
#define SK5_ENG_LM_BBETTY	0x0020 // enable bouncing-betty landmines
#define SK5_ENG_LM_PGAS     0x0040 // enable poison-gas landmines

#define SK5_FDO_CHARGE      0x0001 // consume less charge
#define SK5_FDO_GRENADES    0x0002 // spawn/capacity 2 grenades

#define SK5_MED_CHARGE      0x0001 // consume less charge
#define SK5_MED_GRENADES    0x0002 // spawn/capacity 4 grenades
#define SK5_MED_CARRY_CVO   0x0010 // carry-over health recharge to cvops
#define SK5_MED_CARRY_ENG   0x0020 // carry-over health recharge to eng
#define SK5_MED_CARRY_FDO   0x0040 // carry-over health recharge to fdops
#define SK5_MED_CARRY_SOL   0x0080 // carry-over health recharge to soldier

#define SK5_SOL_CHARGE      0x0001 // consume less charge 
#define SK5_SOL_GRENADES    0x0002 // spawn/capacity 8 grenades
#define SK5_SOL_POISON      0x0004 // enable poison gas

// Skill-5 global constants.
#define SK5G_SPRINT_FACTOR  2.00f // should be > level-2's 1.60f
#define SK5G_CHARGE_FACTOR  0.75f // 75% charge penalty of level-4
#define SK5G_MEDCARRY_TIMER 3500  // milliseconds to add 1 health
#define SK5G_RECOIL_FACTOR  0.85f // applied to existing recoil times (85%)

// bg_covertops
#define COPS_KEEPDISGUISE     1
#define COPS_MEDKIT           2
#define COPS_AMMOPACK         4
#define COPS_MINES            8
#define COPS_LIVEUNI          16
#define COPS_DRAWNAME         32

// g_misc defines
#define MISC_DOUBLEJUMP       0x0001 //  1
#define MISC_BINOCWAR         0x0002 //  2
#define MISC_ADMINSONLY       0x0004 //  4
#define MISC_PACKZ            0x0008 //  8
#define MISC_UNUSED           0x0010 // 16
#define MISC_BSREVIVE         0x0020 // 32
#define MISC_REALAIMSPREAD    0x0040 // 64

// userinfo JayFlags
//#define	JAYFLAGS_KILLSPREESOUNDS	1
//#define JAYFLAGS_PMSOUNDS				2
#define	JAYFLAGS_PMBLOCK				4

// Antilag/Prediction debug
#define DEBUGDELAG_ANTILAG              1
#define DEBUGDELAG_PREDICTION           2

// PERS_JAYFLAGS/effectXTime flags
#define JF_LOSTPANTS					1

// poison
#define POISON_ON						1 // Jaybird - not used really heh
#define POISON_NODISORIENT				2

// poison tracking
#define MAX_POISONEVENTS				10
#define POISONINTERVAL					1500
#define POISONDAMAGE					10
typedef struct poison_s {
	int poisoner;
	int fireTime;
} poison_t;

// Shotgun
// Animation time = 1000 / fps * numFrames
#define M97_RLT_RELOAD1			400		// Reload normal shell start
#define	M97_RLT_RELOAD2			900		// Reload normal shell loop
#define	M97_RLT_RELOAD2_QUICK	600		// Reload normal shell loop FAST
#define	M97_RLT_RELOAD3			550		// Reload normal shell end
#define	M97_RLT_ALTSWITCHFROM	2000	// Reload first shell and pump start
#define	M97_RLT_ALTSWITCHTO		300		// Reload first shell and pump to loop
#define	M97_RLT_DROP2			375		// Reload first shell and pump end

// Shotgun reload states
typedef enum {
	M97_READY,							// Not reloading
	M97_RELOADING_BEGIN,				// Reload normal shell start
	M97_RELOADING_BEGIN_PUMP,			// Reload first shell and pump start
	M97_RELOADING_AFTER_PUMP,			// Reload first shell and pump to loop
	M97_RELOADING_LOOP,					// Reload normal shell loop
} m97state_t;

typedef enum {
	HOLDABLE_HIT,   // hit counter: 16-bits @ 4-bits each { HEAD, HAND, TORSO, FOOT }
	HOLDABLE_HITF,  // hit counter (friendly): 16-bits @ 4-bits each { HEAD, HAND, TORSO, FOOT }
	HOLDABLE_M97,
} holdable_t;

// Jaybird
#define MAX_FIRETEAM_MEMBERS 9

#define MAX_CPU_BUFFER  32

// Functions
void BG_cpuUpdate      ( );
void PM_BeginM97Reload ( );
void PM_M97Reload      ( );
bool BG_IsPercent      ( const char* );

#if defined( CGAMEDLL ) || defined( GAMEDLL )
extern bool ammoTableNeedsUpdate;
void BG_updateAmmoTable();
#endif

// Killing Spree
typedef enum {
	KS_KILLINGSPREE,
	KS_RAMPAGE,
	KS_DOMINATING,
	KS_UNSTOPPABLE,
	KS_GODLIKE,
	KS_WICKEDSICK,
	KS_NUMLEVELS
} ks_t;

typedef enum {
	MK_DOUBLEKILL,
	MK_MULTIKILL,
	MK_MEGAKILL,
	MK_ULTRAKILL,
	MK_MONSTERKILL,
	MK_LUDICROUSKILL,
	MK_HOLYSHIT,
	MK_NUMLEVELS
} mk_t;

///////////////////////////////////////////////////////////////////////////////

typedef enum {
    BVF_NONE    = 0x00000000,
    BVF_ENABLED = 0x00000001,
} bulletVolumeFlags_t;

typedef enum {
    HVF_NONE    = 0x00000000,
    HVF_ENABLED = 0x00000001,
    HVF_HIT     = 0x00000002,
    HVF_DRAW    = 0x00000004,
} hitVolumeFlags_t;

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_JAYMOD_H
