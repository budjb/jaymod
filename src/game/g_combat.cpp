/*
 * name:		g_combat.c
 *
 * desc:		
 *
*/

#include <bgame/impl.h>
#include <omnibot/et/g_etbot_interface.h>

extern void BotRecordKill( int client, int enemy );
extern void BotRecordPain( int client, int enemy, int mod );
extern void BotRecordDeath( int client, int enemy );

extern vec3_t __muzzleTrace;

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, int score ) {
	if ( !ent || !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( cvars::gameState.ivalue != GS_PLAYING ) {
		return;
	}

	if( g_gametype.integer == GT_WOLF_LMS ) {
		return;
	}

	//ent->client->ps.persistant[PERS_SCORE] += score;
	ent->client->sess.game_points += score;

//	level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
============
AddKillScore

Adds score to both the client and his team, only used for playerkills, for lms
============
*/
void AddKillScore( gentity_t *ent, int score ) {
	if ( !ent || !ent->client ) {
		return;
	}
	// no scoring if not playing
	if ( cvars::gameState.ivalue != GS_PLAYING ) {
		return;
	}

	// someone already won
	if( level.lmsWinningTeam )
		return;

	if( g_gametype.integer == GT_WOLF_LMS ) {
		ent->client->ps.persistant[PERS_SCORE] += score;
		level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	}
	ent->client->sess.game_points += score;

	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	/*gitem_t		*item;
	int			weapon;
	gentity_t	*drop = 0;

	// drop the weapon if not a gauntlet or machinegun
	weapon = self->s.weapon;

	// make a special check to see if they are changing to a new
	// weapon that isn't the mg or gauntlet.  Without this, a client
	// can pick up a weapon, be killed, and not drop the weapon because
	// their weapon change hasn't completed yet and they are still holding the MG.

	// (SA) always drop what you were switching to
	if( self->client->ps.weaponstate == WEAPON_DROPPING ) {
		weapon = self->client->pers.cmd.weapon;
	}

	if( !( COM_BitCheck( self->client->ps.weapons, weapon ) ) ) {
		return;
	}

	if((self->client->ps.persistant[PERS_HWEAPON_USE])) {
		return;
	}

	// JPW NERVE don't drop these weapon types
	switch( weapon ) {
		case WP_NONE:
		case WP_KNIFE:
		case WP_DYNAMITE:
		case WP_ARTY:
		case WP_MEDIC_SYRINGE:
		case WP_SMOKETRAIL:
		case WP_MAPMORTAR:
		case VERYBIGEXPLOSION:
		case WP_MEDKIT:
		case WP_BINOCULARS:
		case WP_PLIERS:
		case WP_SMOKE_MARKER:
		case WP_TRIPMINE:
		case WP_SMOKE_BOMB:
		case WP_DUMMY_MG42:
		case WP_LOCKPICK:
		case WP_MEDIC_ADRENALINE:
			return;
		case WP_MORTAR_SET:
			weapon = WP_MORTAR;
			break;
		case WP_K43_SCOPE:
			weapon = WP_K43;
			break;
		case WP_GARAND_SCOPE:
			weapon = WP_GARAND;
			break;
		case WP_FG42SCOPE:
			weapon = WP_FG42;
			break;
		case WP_M7:
			weapon = WP_CARBINE;
			break;
		case WP_GPG40:
			weapon = WP_KAR98;
			break;
		case WP_MOBILE_MG42_SET:
			weapon = WP_MOBILE_MG42;
			break;
	}

	// find the item type for this weapon
	item = BG_FindItemForWeapon( weapon );
	// spawn the item
	
	drop = Drop_Item( self, item, 0, qfalse );
	drop->count = self->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];
	drop->item->quantity = self->client->ps.ammoclip[BG_FindClipForWeapon(weapon)];*/

	weapon_t primaryWeapon;
	gitem_t *item;
	gentity_t *ent;
	int i;

	if( cvars::gameState.ivalue == GS_INTERMISSION ) {
		return;
	}

	primaryWeapon = G_GetPrimaryWeaponForClient( self->client );

	if( primaryWeapon ) {
		// drop our primary weapon
		G_DropWeapon( self, primaryWeapon );
	}

	// Drop binoculars if you have 'em and is set to drop
	if (cvars::bg_weapons.ivalue & SBW_DROPBINOCS && self->client->ps.stats[STAT_KEYS] & ( 1 << INV_BINOCS )) {
		item = BG_FindItemForClassName("weapon_binoculars");
		ent = Drop_Item(self, item, crandom() * 360, qfalse);
		ent->think=MagicSink;
		ent->nextthink = level.time + 60000;
		ent->r.mins[2] -= 8;
	}
	
	// Jaybird - Drop ammo or health on death for respective class
	if (self->client->sess.playerType == PC_MEDIC) {
		for (i = 0; i < g_dropHealth.integer; i++) {
			item = BG_FindItemForClassName("item_health");
			ent = Drop_Item(self, item, crandom() * 360, qfalse);
			ent->think=MagicSink;
			ent->nextthink = level.time + 30000;
		}
	}

	if (self->client->sess.playerType == PC_FIELDOPS) {

		for (i = 0; i < g_dropAmmo.integer; i++) {
			item = BG_FindItemForClassName("weapon_magicammo");
			ent = Drop_Item(self, item, crandom() * 360, qfalse);
			ent->think=MagicSink;
			ent->nextthink = level.time + 30000;
			ent->count = 1;
			ent->s.density = 1;
		}
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;
	vec3_t		angles;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = int( self->s.angles[YAW] );
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = int( vectoyaw(dir) );

	angles[YAW] = vectoyaw ( dir );
	angles[PITCH] = 0; 
	angles[ROLL] = 0;
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) 
{
	gentity_t *other=&g_entities[killer];
	vec3_t dir;

	VectorClear( dir );
	if (other->inuse) {
		if (other->client) {
			VectorSubtract( self->r.currentOrigin, other->r.currentOrigin, dir );
			VectorNormalize( dir );
		} else if (!VectorCompare(other->s.pos.trDelta, vec3_origin)) {
			VectorNormalize2( other->s.pos.trDelta, dir );
		}
	}

	G_AddEvent( self, EV_GIB_PLAYER, DirToByte(dir) );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	if(self->health <= GIB_HEALTH) {
		GibEntity(self, 0);
	}
}


// these are just for logging, the client prints its own messages
char *modNames[] =
{
	"MOD_UNKNOWN",
	"MOD_MACHINEGUN",
	"MOD_BROWNING",
	"MOD_MG42",
	"MOD_GRENADE",
	"MOD_ROCKET",

	// (SA) modified wolf weap mods
	"MOD_KNIFE",
	"MOD_LUGER",
	"MOD_COLT",
	"MOD_MP40",
	"MOD_THOMPSON",
	"MOD_STEN",
	"MOD_GARAND",
	"MOD_SNOOPERSCOPE",
	"MOD_SILENCER",	//----(SA)	
	"MOD_FG42",
	"MOD_FG42SCOPE",
	"MOD_PANZERFAUST",
	"MOD_GRENADE_LAUNCHER",
	"MOD_FLAMETHROWER",
	"MOD_GRENADE_PINEAPPLE",
	"MOD_CROSS",
	// end

	"MOD_MAPMORTAR",
	"MOD_MAPMORTAR_SPLASH",

	"MOD_KICKED",
	"MOD_GRABBER",

	"MOD_DYNAMITE",
	"MOD_AIRSTRIKE", // JPW NERVE
	"MOD_SYRINGE",	// JPW NERVE
	"MOD_AMMO",	// JPW NERVE
	"MOD_ARTY",	// JPW NERVE

	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_EXPLOSIVE",

	"MOD_CARBINE",
	"MOD_KAR98",
	"MOD_GPG40",
	"MOD_M7",
	"MOD_LANDMINE",
	"MOD_SATCHEL",
	"MOD_TRIPMINE",
	"MOD_SMOKEBOMB",
	"MOD_MOBILE_MG42",
	"MOD_SILENCED_COLT",
	"MOD_GARAND_SCOPE",

	"MOD_CRUSH_CONSTRUCTION",
	"MOD_CRUSH_CONSTRUCTIONDEATH",
	"MOD_CRUSH_CONSTRUCTIONDEATH_NOATTACKER",

	"MOD_K43",
	"MOD_K43_SCOPE",

	"MOD_MORTAR",

	"MOD_AKIMBO_COLT",
	"MOD_AKIMBO_LUGER",
	"MOD_AKIMBO_SILENCEDCOLT",
	"MOD_AKIMBO_SILENCEDLUGER",

	"MOD_SMOKEGRENADE",

	// RF
	"MOD_SWAP_PLACES",

	// OSP -- keep these 2 entries last
	"MOD_SWITCHTEAM",
	"MOD_GOOMBA",
	"MOD_POISON_SYRINGE",
	"MOD_THROWING_KNIFE",
	"MOD_FEAR",
	"MOD_REFLECTED_FF",
	"MOD_M97",
	"MOD_POISON_GAS",
	"MOD_MOLOTOV",

    "MOD_SHOVE",
};

/*
==================
player_die
==================
*/
void BotRecordTeamDeath( int client );

void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	int			contents = 0, i, killer = ENTITYNUM_WORLD;
	const char*	killerName = "<world>";
	qboolean	nogib = qtrue;
	gitem_t		*item = NULL;
	gentity_t	*ent;
	qboolean	killedintank = qfalse;
	//float		timeLived;
	weapon_t	weap;

	// Jaybird - override/set weapon
    	if( meansOfDeath == MOD_FEAR ) {
		weap = BG_WeaponForMOD( self->client->lasthurt_mod );
    	} else if (meansOfDeath == MOD_FALLING && self->client->pmext.wasShoved) {
		weap = WP_NONE;
		attacker = &g_entities[self->client->pmext.shover];
		meansOfDeath = MOD_SHOVED;
    	} else { // default
		weap = BG_WeaponForMOD( meansOfDeath );
    	}

	if(attacker == self) {
		if(self->client) {
			self->client->pers.playerStats.suicides++;
			trap_PbStat ( self - g_entities , "suicide" , 
				va ( "%d %d %d" , self->client->sess.sessionTeam , self->client->sess.playerType , weap ) ) ;
		}
	} else if(OnSameTeam( self, attacker )) {
		G_LogTeamKill(	attacker,	weap );
	} else {
		G_LogDeath( self,		weap );
		G_LogKill(	attacker,	weap );

		if( cvars::gameState.ivalue == GS_PLAYING ) {
			if( attacker->client ) {
				attacker->client->combatState = (combatstate_t)( attacker->client->combatState | (1<<COMBATSTATE_KILLEDPLAYER) );
			}
		}
	}

	if( !OnSameTeam( self, attacker ) ) {
		self->isProp = qfalse;	// were we teamkilled or not?
	} else {
		self->isProp = qtrue;
	}	

	// if we got killed by a landmine, update our map
	if( self->client && meansOfDeath == MOD_LANDMINE ) {
		// if it's an enemy mine, update both teamlists
		/*int teamNum;
		mapEntityData_t	*mEnt;
		mapEntityData_Team_t *teamList;
	
		teamNum = inflictor->s.teamNum % 4;

		teamList = self->client->sess.sessionTeam == TEAM_AXIS ? &mapEntityData[0] : &mapEntityData[1];
		if((mEnt = G_FindMapEntityData(teamList, inflictor-g_entities)) != NULL) {
			G_FreeMapEntityData( teamList, mEnt );
		}

		if( teamNum != self->client->sess.sessionTeam ) {
			teamList = self->client->sess.sessionTeam == TEAM_AXIS ? &mapEntityData[1] : &mapEntityData[0];
			if((mEnt = G_FindMapEntityData(teamList, inflictor-g_entities)) != NULL) {
				G_FreeMapEntityData( teamList, mEnt );
			}
		}*/
		mapEntityData_t	*mEnt;

		if((mEnt = G_FindMapEntityData(&mapEntityData[0], inflictor-g_entities)) != NULL) {
			G_FreeMapEntityData( &mapEntityData[0], mEnt );
		}

		if((mEnt = G_FindMapEntityData(&mapEntityData[1], inflictor-g_entities)) != NULL) {
			G_FreeMapEntityData( &mapEntityData[1], mEnt );
		}
	}

	{
		mapEntityData_t	*mEnt;
		mapEntityData_Team_t *teamList = self->client->sess.sessionTeam == TEAM_AXIS ? &mapEntityData[1] : &mapEntityData[0];	// swapped, cause enemy team

		mEnt = G_FindMapEntityDataSingleClient( teamList, NULL, self->s.number, -1 );
		
		while( mEnt ) {
			if( mEnt->type == ME_PLAYER_DISGUISED ) {
				mapEntityData_t* mEntFree = mEnt;

				mEnt = G_FindMapEntityDataSingleClient( teamList, mEnt, self->s.number, -1 );

				G_FreeMapEntityData( teamList, mEntFree );
			} else {
				mEnt = G_FindMapEntityDataSingleClient( teamList, mEnt, self->s.number, -1 );
			}
		}
	}

	if( self->tankLink ) {
		G_LeaveTank( self, qfalse );

		killedintank = qtrue;
	}

	if( self->client->ps.pm_type == PM_DEAD || cvars::gameState.ivalue == GS_INTERMISSION ) {
		return;
	}

	// Player isn't playing dead now, he really is dead!
	self->client->ps.eFlags &= ~EF_PLAYDEAD;

	// Jaybird - clear poison effects
	self->client->ps.eFlags &= ~EF_POISON;
	G_ResetPoisonEvents(self);

	// OSP - death stats handled out-of-band of G_Damage for external calls
	if( meansOfDeath == MOD_FEAR ) {
		G_addStats( self, attacker, damage, self->client->lasthurt_mod );
		// tjw: this would normally be handled in G_Damage()
		G_AddKillSkillPoints( attacker, (meansOfDeath_t)self->client->lasthurt_mod, HR_HEAD, qfalse );
	}
	else {
		G_addStats(self, attacker, damage, meansOfDeath);
	}
	// OSP

	self->client->ps.pm_type = PM_DEAD;

	G_AddEvent( self, EV_STOPSTREAMINGSOUND, 0);

	if(attacker) {
		killer = attacker->s.number;
		killerName = (attacker->client) ? attacker->client->pers.netname : "<non-client>";
	}

	if(attacker == 0 || killer < 0 || killer >= MAX_CLIENTS) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if(cvars::gameState.ivalue == GS_PLAYING) {
		char *obit;

		if(meansOfDeath < 0 || meansOfDeath >= int( sizeof(modNames) / sizeof(modNames[0]) )) {
			obit = "<bad obituary>";
		} else {
			obit = modNames[meansOfDeath];
		}

		//////////////////////////////////////////////////////////////////////////
		// send the events

		Bot_Event_Death(self-g_entities, &g_entities[attacker-g_entities], obit);
		Bot_Event_KilledSomeone(attacker-g_entities, &g_entities[self-g_entities], obit);

		G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", killer, self->s.number, meansOfDeath, killerName, self->client->pers.netname, obit );
	}

	
	//////////////////////////////////////////////////////////////////////////

	// broadcast the death event to everyone
	ent = G_TempEntity( vec3_origin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	// Jaybird - They're dead.  Adjust their spree info.
	// G_endKillSpree(self, attacker, meansOfDeath);
	G_AddLoseSpree( self );

	self->enemy = attacker;
	// Jaybird - shrubbot shortcuts
	Q_strncpyz(self->client->pers.lastkilled, killerName, sizeof(self->client->pers.lastkilled));
    if (attacker && attacker->client) {
		Q_strncpyz(attacker->client->pers.lastkill, self->client->pers.netname, sizeof(attacker->client->pers.lastkill));
        self->client->lastkilledby_client = attacker->s.number;
    }

	//self->client->ps.persistant[PERS_KILLED]++;

	// CHRUKER: b010 - Make sure covert ops lose their disguises
	if ( self->client->ps.powerups[PW_OPS_DISGUISED] )
		self->client->ps.powerups[PW_OPS_DISGUISED] = 0;

	// JPW NERVE -- if player is holding ticking grenade, drop it
	if (self->client->ps.grenadeTimeLeft) {
		switch (self->s.weapon) {
			case WP_DYNAMITE:
			case WP_LANDMINE:
			case WP_LANDMINE_BBETTY:
			case WP_LANDMINE_PGAS:
			case WP_MOLOTOV:
			case WP_SATCHEL:
			case WP_TRIPMINE:
				break;

			default: {
				vec3_t launchvel, launchspot;
				gentity_t* m;

				launchvel[0] = crandom();
				launchvel[1] = crandom();
				launchvel[2] = random();
				VectorScale( launchvel, 160, launchvel );
				VectorCopy(self->r.currentOrigin, launchspot);
				launchspot[2] += 40;
		
				// Gordon: fixes premature grenade explosion, ta bani ;)
				m = fire_grenade(self, launchspot, launchvel, self->s.weapon);
				m->damage = 0;
				break;
			}
		}
	}

	if (attacker && attacker->client) {
		if ( attacker == self || OnSameTeam (self, attacker ) ) {

			// DHM - Nerve :: Complaint lodging
			// Jaybird - shrubbot immunity
			if( attacker != self && !level.warmupEndTime && cvars::gameState.ivalue == GS_PLAYING && !cmd::entityHasPermission( attacker, priv::base::voteImmunity )) {
				if( attacker->client->pers.localClient )
				{
					if(attacker->r.svFlags & SVF_BOT)
						trap_SendServerCommand( self-g_entities, "complaint -5" );
					else
						trap_SendServerCommand( self-g_entities, "complaint -4" );
				} else {
					if( meansOfDeath != MOD_CRUSH_CONSTRUCTION && meansOfDeath != MOD_CRUSH_CONSTRUCTIONDEATH && meansOfDeath != MOD_CRUSH_CONSTRUCTIONDEATH_NOATTACKER ) {
						if( g_complaintlimit.integer ) {

							if( !(meansOfDeath == MOD_LANDMINE && g_disableComplaints.integer & TKFL_MINES ) &&
								!((meansOfDeath == MOD_ARTY || meansOfDeath == MOD_AIRSTRIKE) && g_disableComplaints.integer & TKFL_AIRSTRIKE ) &&
								!(meansOfDeath == MOD_MORTAR && g_disableComplaints.integer & TKFL_MORTAR ) ) {
								trap_SendServerCommand( self-g_entities, va( "complaint %i", attacker->s.number ) );
								self->client->pers.complaintClient = attacker->s.clientNum;
								self->client->pers.complaintEndTime = level.time + 20500;
							}
						}
					}
				}
			}

			// high penalty to offset medic heal
/*			AddScore( attacker, WOLF_FRIENDLY_PENALTY ); */

			if( g_gametype.integer == GT_WOLF_LMS ) {
				AddKillScore( attacker, WOLF_FRIENDLY_PENALTY );
			}
		} else {

			//G_AddExperience( attacker, 1 );

			// JPW NERVE -- mostly added as conveneience so we can tweak from the #defines all in one place
			AddScore(attacker, WOLF_FRAG_BONUS);

			// Jaybird - Only run these if the player:
			// a) exists
			// b) isn't the one who died
			G_EndLoseSpree(attacker);
			G_AddKillSpree(attacker);
			G_FirstBlood(attacker);
			G_UpdateLastKill(attacker);
			G_AddMultiKill(attacker);

			if( g_gametype.integer == GT_WOLF_LMS ) {
				if( level.firstbloodTeam == -1 )
					level.firstbloodTeam = attacker->client->sess.sessionTeam;

				AddKillScore( attacker, WOLF_FRAG_BONUS );
			}

			attacker->client->lastKillTime = level.time;
		}
	} else {
		AddScore( self, -1 );

		if( g_gametype.integer == GT_WOLF_LMS )
			AddKillScore( self, -1 );
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// drop flag regardless
	if (self->client->ps.powerups[PW_REDFLAG]) {
		item = BG_FindItem("Red Flag");
		if (!item)
			item = BG_FindItem("Objective");

		self->client->ps.powerups[PW_REDFLAG] = 0;
	}
	if (self->client->ps.powerups[PW_BLUEFLAG]) {
		item = BG_FindItem("Blue Flag");
		if (!item)
			item = BG_FindItem("Objective");

		self->client->ps.powerups[PW_BLUEFLAG] = 0;
	}

	if (item) {
		vec3_t launchvel = { 0, 0, 0 };
		gentity_t *flag = LaunchItem(item, self->r.currentOrigin, launchvel, self->s.number);

		flag->s.modelindex2 = self->s.otherEntityNum2;// JPW NERVE FIXME set player->otherentitynum2 with old modelindex2 from flag and restore here
		flag->message = self->message;	// DHM - Nerve :: also restore item name

		Bot_Util_SendTrigger(flag, NULL, va("%s dropped.", flag->message), "dropped");

		// Clear out player's temp copies
		self->s.otherEntityNum2 = 0;
		self->message = NULL;
	}

	// send a fancy "MEDIC!" scream.  Sissies, ain' they?
	if (self->client != NULL) {
		if( self->health > GIB_HEALTH && meansOfDeath != MOD_SUICIDE && meansOfDeath != MOD_SWITCHTEAM ) {
			G_AddEvent( self, EV_MEDIC_CALL, 0 );

			// ATM: only register the goal if the target isn't in water.
			if(self->waterlevel <= 1)
			{
				Bot_AddFallenTeammateGoals(self, self->client->sess.sessionTeam);
			}			
		}
	}

	Cmd_Score_f( self );		// show scores

	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for(i=0; i<level.numConnectedClients; i++) {
		gclient_t *client = &level.clients[level.sortedClients[i]];

		if(client->pers.connected != CON_CONNECTED) continue;
		if(client->sess.sessionTeam != TEAM_SPECTATOR) continue;

		if(client->sess.spectatorClient == self->s.number) {
			Cmd_Score_f(g_entities + level.sortedClients[i]);
		}
	}

	self->takedamage = qtrue;	// can still be gibbed
	self->r.contents = CONTENTS_CORPSE;

	//self->s.angles[2] = 0;
	self->s.powerups = 0;
	self->s.loopSound = 0;
	
	self->client->limboDropWeapon = self->s.weapon; // store this so it can be dropped in limbo

	LookAtKiller( self, inflictor, attacker );
	self->client->ps.viewangles[0] = 0;
	self->client->ps.viewangles[2] = 0;
	//VectorCopy( self->s.angles, self->client->ps.viewangles );

//	trap_UnlinkEntity( self );
	// ydnar: so bodoes don't clip into world
	// Jaybird: ClientHiboxMaxZ will take care of effective hitbox
	self->r.maxs[2] = self->client->ps.crouchMaxZ;
	self->client->ps.maxs[2] = self->client->ps.crouchMaxZ;
	trap_LinkEntity( self );

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.timeCurrent + 800;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// never gib in a nodrop
	// FIXME: contents is always 0 here
	if ( self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) ) {
		GibEntity( self, killer );
		nogib = qfalse;
	}

	if(nogib){
		// normal death
		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH + 1;
		}

		// Arnout: re-enable this for flailing
/*		if( self->client->ps.groundEntityNum == ENTITYNUM_NONE ) {
			self->client->ps.pm_flags |= PMF_FLAILING;
			self->client->ps.pm_time = 750;
			BG_AnimScriptAnimation( &self->client->ps, ANIM_MT_FLAILING, qtrue );

			// Face explosion directory
			{
				vec3_t angles;

				vectoangles( self->client->ps.velocity, angles );
				self->client->ps.viewangles[YAW] = angles[YAW];
				SetClientViewAngle( self, self->client->ps.viewangles );
			}
		} else*/

			// DHM - Play death animation, and set pm_time to delay 'fallen' animation
			//if( G_IsSinglePlayerGame() && self->client->sess.sessionTeam == TEAM_ALLIES ) {
			//	// play "falldown" animation since allies bots won't ever die completely
			//	self->client->ps.pm_time = BG_AnimScriptEvent( &self->client->ps, self->client->pers.character->animModelInfo, ANIM_ET_FALLDOWN, qfalse, qtrue );
			//	G_StartPlayerAppropriateSound(self, "death");
			//} else {
				self->client->ps.pm_time = BG_AnimScriptEvent( &self->client->ps, self->client->pers.character->animModelInfo, ANIM_ET_DEATH, qfalse, qtrue );
				// death animation script already contains sound
			//}

			// record the death animation to be used later on by the corpse
			self->client->torsoDeathAnim = self->client->ps.torsoAnim;
			self->client->legsDeathAnim = self->client->ps.legsAnim;

			G_AddEvent( self, EV_DEATH1 + 1, killer );

		// the body can still be gibbed
		self->die = body_die;
	}

	if( meansOfDeath == MOD_MACHINEGUN ) {
		switch( self->client->sess.sessionTeam ) {
			case TEAM_AXIS:
				level.axisMG42Counter = level.time;
				break;
			case TEAM_ALLIES:
				level.alliesMG42Counter = level.time;
				break;
			default:
				break;
		}
	}

	G_FadeItems( self, MOD_SATCHEL );

	CalculateRanks();

	if( killedintank /*Gordon: automatically go to limbo from tank*/ ) {
		limbo( self, qfalse ); // but no corpse
	} else if (( meansOfDeath == MOD_SUICIDE || meansOfDeath == MOD_FEAR ) && cvars::gameState.ivalue == GS_PLAYING ) {
		limbo( self, qtrue );
	} else if( g_gametype.integer == GT_WOLF_LMS ) {
		if( !G_CountTeamMedics( self->client->sess.sessionTeam, qtrue ) ) {
			limbo( self, qtrue );
		}
	}
}

qboolean IsHeadShotWeapon (int mod) {
	// players are allowed headshots from these weapons
    switch (mod) {
        case MOD_LUGER:
        case MOD_COLT:
        case MOD_AKIMBO_COLT:
        case MOD_AKIMBO_LUGER:
        case MOD_AKIMBO_SILENCEDCOLT:
        case MOD_AKIMBO_SILENCEDLUGER:
        case MOD_MP40:
        case MOD_THOMPSON:
        case MOD_STEN:
        case MOD_KAR98:
        case MOD_K43:
        case MOD_K43_SCOPE:
        case MOD_CARBINE:
        case MOD_GARAND:
        case MOD_GARAND_SCOPE:
        case MOD_SILENCER:
        case MOD_SILENCED_COLT:
        case MOD_FG42:
        case MOD_FG42SCOPE:
		    return qtrue;
        default:
    	    return qfalse;
    }
}

/*
============
G_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS				damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR				armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK			do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION		kills godmode, armor, everything
	DAMAGE_JAY_NO_PROTECTION	unlike above, this really kills everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, const vec3_t in_dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			save;
	int			knockback;
	qboolean	wasAlive;
	hitRegion_t	hr = HR_NUM_HITREGIONS;
	int			limbo_health;

	limbo_health = FORCE_LIMBO_HEALTH;

	if (!targ->takedamage) {
		return;
	}

	// the intermission has already been qualified for, so don't
	// allow any extra scoring
	// CHRUKER: b024 - Don't do damage if at warmup and warmupdamage is set to 'None' and the target is a client.
	if ( level.intermissionQueued || (cvars::gameState.ivalue != GS_PLAYING && match_warmupDamage.integer == 0 && targ->client)) {
		return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// Arnout: invisible entities can't be damaged
	if( targ->entstate == STATE_INVISIBLE ||
		targ->entstate == STATE_UNDERCONSTRUCTION ) {
		return;
	}

	// xkan, 12/23/2002 - was the bot alive before applying any damage?
	wasAlive = (targ->health > 0) ? qtrue : qfalse;

	// Arnout: combatstate
	if( targ->client && attacker && attacker->client && attacker != targ ) {
		if( cvars::gameState.ivalue == GS_PLAYING ) {
			if( !OnSameTeam( attacker, targ ) ) {
				targ->client->combatState = (combatstate_t)( targ->client->combatState | (1<<COMBATSTATE_DAMAGERECEIVED) );
				attacker->client->combatState = (combatstate_t)( attacker->client->combatState | (1<<COMBATSTATE_DAMAGEDEALT) );
			}
		}
	}

    // JPW NERVE
    if ((targ->waterlevel >= 3) && (mod == MOD_FLAMETHROWER))
        return;
    // jpw

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER && !(targ->isProp) && !targ->scriptName) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			G_UseEntity( targ, inflictor, attacker );
		}
		return;
	}

	// TAT 11/22/2002
	//		In the old code, this check wasn't done for props, so I put that check back in to make props_statue properly work	
	// 4 means destructible
	if ( targ->s.eType == ET_MOVER && (targ->spawnflags & 4) && !targ->isProp ) 
	{
		if( !G_WeaponIsExplosive( (meansOfDeath_t)mod ) ) {
			return;
		}

		// check for team
		if( G_GetTeamFromEntity( inflictor ) == G_GetTeamFromEntity( targ ) ) {
			return;
		}
	} else if ( targ->s.eType == ET_EXPLOSIVE ) {
		if( targ->parent && G_GetWeaponClassForMOD( (meansOfDeath_t)mod ) == 2 ) {
			return;
		}

		if( G_GetTeamFromEntity( inflictor ) == G_GetTeamFromEntity( targ ) ) {
			return;
		}

		if( G_GetWeaponClassForMOD( (meansOfDeath_t)mod ) < targ->constructibleStats.weaponclass ) {
			return;
		}
	}
	else if ( targ->s.eType == ET_MISSILE && targ->methodOfDeath == MOD_LANDMINE ) {
		if( targ->s.modelindex2 ) {
			if( G_WeaponIsExplosive( (meansOfDeath_t)mod ) ) {
				mapEntityData_t	*mEnt;

				if((mEnt = G_FindMapEntityData(&mapEntityData[0], targ-g_entities)) != NULL) {
					G_FreeMapEntityData( &mapEntityData[0], mEnt );
				}

				if((mEnt = G_FindMapEntityData(&mapEntityData[1], targ-g_entities)) != NULL) {
					G_FreeMapEntityData( &mapEntityData[1], mEnt );
				}

				if( attacker && attacker->client ) {
					AddScore( attacker, 1 );
				}

				G_ExplodeMissile(targ);
			}
		}
		return;
	} else if ( targ->s.eType == ET_CONSTRUCTIBLE ) {

		if( G_GetTeamFromEntity( inflictor ) == G_GetTeamFromEntity( targ ) ) {
			return;
		}

		if( G_GetWeaponClassForMOD( (meansOfDeath_t)mod ) < targ->constructibleStats.weaponclass ) {
			return;
		}
		//bani - fix #238
		if ( mod == MOD_DYNAMITE ) {
			if( !( inflictor->etpro_misc_1 & 1 ) )
				return;
		}
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip || ( client->ps.powerups[PW_INVULNERABLE] && !( dflags & DAMAGE_JAY_NO_PROTECTION ))) {
			return;
		}
	}

	// check for godmode
	if ( targ->flags & FL_GODMODE ) {
		return;
	}

    // ugly-ass code but we do this to make in_dir read-only
    vec3_t dir;

	if ( !in_dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
        VectorCopy( in_dir, dir );
		VectorNormalize( dir );
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	} else if( dflags & DAMAGE_HALF_KNOCKBACK ) {
		knockback = int( knockback * 0.5f );
	}
	
	// ydnar: set weapons means less knockback
	if( client && (client->ps.weapon == WP_MORTAR_SET || client->ps.weapon == WP_MOBILE_MG42_SET) )
		knockback = int( knockback * 0.5f );

	if( targ->client && g_friendlyFire.integer && OnSameTeam(targ, attacker) ) {
		knockback = 0;
	}
	
	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

        // From NoQuarter, I'm not sure I need this
		if ( attacker && attacker->client && ( targ->client->ps.groundEntityNum != ENTITYNUM_NONE || G_WeaponIsExplosive((meansOfDeath_t)mod) )){
			targ->client->pmext.wasShoved = qtrue;
			targ->client->pmext.shover = attacker - g_entities;
		}

		if (targ == attacker && !(	mod != MOD_ROCKET &&
									mod != MOD_GRENADE &&
									mod != MOD_GRENADE_LAUNCHER &&
									mod != MOD_DYNAMITE
									&& mod != MOD_GPG40
									&& mod != MOD_M7
									&& mod != MOD_LANDMINE
									))
		{
			targ->client->ps.velocity[2] *= 0.25;
		}

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

    // skip damage if friendly fire is disabled
    if (!(dflags & DAMAGE_NO_PROTECTION)
        && targ != attacker
        && OnSameTeam( targ, attacker )
        && !g_friendlyFire.integer)
    {
        return;
    }

	if (damage < 1)
		damage = 1;

	take = damage;
	save = 0;

	if ( attacker->client && targ->client && targ != attacker && targ->health > 0 ) {
		// Jaybird - Hitsounds
		// vsay "hold your fire" on the first hit of a teammate
		// only applies if the player has been hurt before
		// and the match is not in warmup.
		if( OnSameTeam( targ, attacker )) {
			if(( !client->lasthurt_mod || client->lasthurt_client != attacker->s.number ) && cvars::gameState.ivalue == GS_PLAYING && ( targ->health - take ) > limbo_health ) {
				if( client->sess.sessionTeam == TEAM_AXIS )
					G_ClientSound( attacker, "sound/chat/axis/26a.wav" );
				else	
					G_ClientSound( attacker, "sound/chat/allies/26a.wav" );
			}

			if (mod != MOD_GOOMBA && mod != MOD_POISON_SYRINGE) {
				g_clientObjects[attacker->s.number].recordHit( AbstractHitVolume::ZONE_BODY, true );
			}
		} else {
			if (mod != MOD_GOOMBA && mod != MOD_POISON_SYRINGE) {
				g_clientObjects[attacker->s.number].recordHit( AbstractHitVolume::ZONE_BODY, false );
			}
		}
	}

	// adrenaline junkie!
	if( targ->client && targ->client->ps.powerups[PW_ADRENALINE] ) {
		take = int( take * 0.5f );
	}

	// save some from flak jacket
	// Jaybird - engineer class carryover
	if( targ->client && targ->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 4 && ( targ->client->sess.playerType == PC_ENGINEER || ( cvars::bg_skills.ivalue & SBS_ENGI ))) {
		if( mod == MOD_GRENADE ||
			mod == MOD_GRENADE_LAUNCHER ||
			mod == MOD_ROCKET ||
			mod == MOD_GRENADE_PINEAPPLE ||
			mod == MOD_MAPMORTAR ||
			mod == MOD_MAPMORTAR_SPLASH || 
			mod == MOD_EXPLOSIVE ||
			mod == MOD_LANDMINE ||
			mod == MOD_GPG40 ||
			mod == MOD_M7 ||
			mod == MOD_SATCHEL ||
			mod == MOD_ARTY ||
			mod == MOD_AIRSTRIKE ||
			mod == MOD_DYNAMITE ||
			mod == MOD_MORTAR ||
			mod == MOD_PANZERFAUST ||
			mod == MOD_MAPMORTAR ) {
			take -= int( take * 0.5f );
		}
	}

#ifndef DEBUG_STATS
	if ( g_debugDamage.integer )
#endif
	{
		G_Printf( "client:%i health:%i damage:%i mod:%s\n", targ->s.number, targ->health, take, modNames[mod] );
	}

	if( targ && targ->client && attacker && attacker->client && targ != attacker && targ->health > 0 && OnSameTeam( targ, attacker ) && g_friendlyFire.integer == 2 && IsReflectable( mod )) {
		int ffDamage;

		// Percentage based reflect
		ffDamage = int( take * g_reflectFriendlyFire.value / 100.f );
		if( ffDamage <= 0 ) {
			ffDamage = 0;
		}
		attacker->health -= ffDamage;

		// Give them pain!
		attacker->client->damage_blood += take;
		attacker->client->damage_knockback += knockback;

		// Set the lasthurt stuff so hitsounds do not replay
		targ->client->lasthurt_mod = mod;
		targ->client->lasthurt_client = attacker - g_entities;

		// Kill the player if necessary
		if( attacker->health <= 0 ) {
			attacker->deathType = MOD_REFLECTED_FF;
			attacker->enemy = attacker;
			if( attacker->die ) {
				attacker->die( attacker, attacker, attacker, ffDamage, MOD_REFLECTED_FF );
			}
		}
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) {
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_blood += take;
		client->damage_knockback += knockback;

		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
//	Team_CheckHurtCarrier(targ, attacker);

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
		targ->client->lasthurt_time = level.time;
	}

	// do the damage
	if( take ) {
		targ->health -= take;

		// Gordon: don't ever gib POWS
		if( ( targ->health <= 0 ) && ( targ->r.svFlags & SVF_POW ) ) {
			targ->health = -1;
		}

		// Ridah, can't gib with bullet weapons (except VENOM)
		// Arnout: attacker == inflictor can happen in other cases as well! (movers trying to gib things)
		//if ( attacker == inflictor && targ->health <= GIB_HEALTH) {
		if( targ->health <= GIB_HEALTH ) {
			if( !G_WeaponIsExplosive( (meansOfDeath_t)mod ) ) {
				targ->health = GIB_HEALTH + 1;
			}
		}

		if( g_damagexp.integer && client && G_GetTeamFromEntity( inflictor ) != G_GetTeamFromEntity( targ )) {
			// Jaybird - give them some per hit
			// They get 1 XP per 50 damage, so multiple .02 * take
			int skill = G_SkillForMOD( mod );
			if( skill >= 0 )
				G_AddSkillPoints( attacker, (skillType_t)skill, take * .02 );
		}

// JPW NERVE overcome previous chunk of code for making grenades work again
//		if ((take > 190)) // 190 is greater than 2x mauser headshot, so headshots don't gib
		// Arnout: only player entities! messes up ents like func_constructibles and func_explosives otherwise
		if( ( (targ->s.number < MAX_CLIENTS) && (take > 190) ) && !(targ->r.svFlags & SVF_POW) ) {
			targ->health = GIB_HEALTH - 1;
		}

		if( targ->s.eType == ET_MOVER && !Q_stricmp( targ->classname, "script_mover" ) ) {
			targ->s.dl_intensity = int( 255.f * (targ->health / (float)targ->count) );	// send it to the client
		}

		//G_Printf("health at: %d\n", targ->health);
		if( targ->health <= 0 ) {
			if( client && !wasAlive ) {
				targ->flags |= FL_NO_KNOCKBACK;
				// OSP - special hack to not count attempts for body gibbage
				if( targ->client->ps.pm_type == PM_DEAD ) {
					G_addStats(targ, attacker, take, mod);
				}

				if( (targ->health < FORCE_LIMBO_HEALTH) && (targ->health > GIB_HEALTH) ) {
					limbo(targ, qtrue);
				}
				// xkan, 1/13/2003 - record the time we died.
				if (!client->deathTime)
					client->deathTime = level.time;

				//bani - #389
				if( targ->health <= GIB_HEALTH ) {
					GibEntity( targ, 0 );
				}
			} else {
				targ->sound1to2 = hr;
				targ->sound2to1 = mod;
				targ->sound2to3 = (dflags & DAMAGE_RADIUS) ? 1 : 0;

				if( client ) {
					if( G_GetTeamFromEntity( inflictor ) != G_GetTeamFromEntity( targ ) ) {
						G_AddKillSkillPoints( attacker, (meansOfDeath_t)mod, hr, (qboolean)(dflags & DAMAGE_RADIUS)  );
					}
				}

				if( targ->health < -999 ) {
					targ->health = -999;
				}


				targ->enemy = attacker;
				targ->deathType = (meansOfDeath_t)mod;

				// Ridah, mg42 doesn't have die func (FIXME)
				if( targ->die ) {	
					// Kill the entity.  Note that this funtion can set ->die to another
					// function pointer, so that next time die is applied to the dead body.
					targ->die( targ, inflictor, attacker, take, mod );
					// OSP - kill stats in player_die function
				}

				if( targ->s.eType == ET_MOVER && !Q_stricmp( targ->classname, "script_mover" ) && (targ->spawnflags & 8) ) {
					return;	// reseructable script mover doesn't unlink itself but we don't want a second death script to be called
				}

				// if we freed ourselves in death function
				if (!targ->inuse)
					return;

				// RF, entity scripting
				if ( targ->health <= 0) {	// might have revived itself in death function
					if( targ->r.svFlags & SVF_BOT ) {
                        // Removed
					} else if(	( targ->s.eType != ET_CONSTRUCTIBLE && targ->s.eType != ET_EXPLOSIVE ) ||
								( targ->s.eType == ET_CONSTRUCTIBLE && !targ->desstages ) )	{ // call manually if using desstages
						G_Script_ScriptEvent( targ, "death", "" );
					}
				}
			}

		} else if ( targ->pain ) {
			if (dir) {	// Ridah, had to add this to fix NULL dir crash
				VectorCopy (dir, targ->rotate);
				VectorCopy (point, targ->pos3); // this will pass loc of hit
			} else {
				VectorClear( targ->rotate );
				VectorClear( targ->pos3 );
			}

			targ->pain (targ, attacker, take, point);
		} else {
			// OSP - update weapon/dmg stats
			G_addStats(targ, attacker, take, mod);
			// OSP
		}

		// RF, entity scripting
		G_Script_ScriptEvent( targ, "pain", va("%d %d", targ->health, targ->health+take) );

		// RF, record bot pain
		if (targ->s.number < level.maxclients)
		{
			// notify omni-bot framework
			Bot_Event_TakeDamage(targ-g_entities, attacker);
		}

		// Ridah, this needs to be done last, incase the health is altered in one of the event calls
		// Jaybird - playdead check
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}

        // Cheap way to ID inflictor entity as poison smoke.
		if (inflictor->poisonGasAlarm && mod == MOD_POISON_GAS && targ->health >= 0)
			G_AddEvent(targ, EV_COUGH, 0);
	}
}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/

void G_RailTrail( vec_t* start, vec_t* end ) {
	gentity_t* temp = G_TempEntity( start, EV_RAILTRAIL );
	VectorCopy( end, temp->s.origin2 );
	temp->s.dmgFlags = 0;
}

#define MASK_CAN_DAMAGE		(CONTENTS_SOLID | CONTENTS_BODY)

qboolean CanDamage (gentity_t *targ, vec3_t origin) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;
	vec3_t offsetmins = { -16.f, -16.f, -16.f };
	vec3_t offsetmaxs = { 16.f, 16.f, 16.f };

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	// Gordon: well, um, just check then...
	if(targ->r.currentOrigin[0] || targ->r.currentOrigin[1] || targ->r.currentOrigin[2]) {
		VectorCopy( targ->r.currentOrigin, midpoint );

		if( targ->s.eType == ET_MOVER ) {
			midpoint[2] += 32;
		}
	} else {
		VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
		VectorScale (midpoint, 0.5, midpoint);
	}

//	G_RailTrail( origin, dest );

	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, midpoint, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if (tr.fraction == 1.0)
		return qtrue;

	if(&g_entities[tr.entityNum] == targ)
		return qtrue;

	if( targ->client ) {
		VectorCopy( targ->client->ps.mins, offsetmins );
		VectorCopy( targ->client->ps.maxs, offsetmaxs );
	}

	// this should probably check in the plane of projection, 
	// rather than in world coordinate
	VectorCopy (midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmaxs[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	VectorCopy (midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmaxs[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	VectorCopy (midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmaxs[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	VectorCopy (midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmaxs[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	// =========================

	VectorCopy (midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmins[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	VectorCopy (midpoint, dest);
	dest[0] += offsetmaxs[0];
	dest[1] += offsetmins[1];
	dest[2] += offsetmins[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	VectorCopy (midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmaxs[1];
	dest[2] += offsetmins[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	VectorCopy (midpoint, dest);
	dest[0] += offsetmins[0];
	dest[1] += offsetmins[2];
	dest[2] += offsetmins[2];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_CAN_DAMAGE );
	if( tr.fraction == 1 || &g_entities[tr.entityNum] == targ ) {
		return qtrue;
	}

	return qfalse;
}

void G_AdjustedDamageVec( gentity_t *ent, vec3_t origin, vec3_t v )
{
	int i;

	if (!ent->r.bmodel)
		VectorSubtract(ent->r.currentOrigin,origin,v); // JPW NERVE simpler centroid check that doesn't have box alignment weirdness
	else {
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}
	}
}

/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage( vec3_t origin, gentity_t *inflictor, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod ) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
	float		boxradius;
	vec3_t		dest; 
	trace_t		tr;
	vec3_t		midpoint;
	int			flags = DAMAGE_RADIUS;

	if( mod == MOD_SATCHEL || mod == MOD_LANDMINE ) {
		flags |= DAMAGE_HALF_KNOCKBACK;
	}

	if( radius < 1 ) {
		radius = 1;
	}

	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement -- 
	// bounding box was checking against radius / sqrt(2) if collision is along box plane
	for( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - boxradius;
		maxs[i] = origin[i] + boxradius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if( ent == ignore ) {
			continue;
		}
		if( !ent->takedamage && ( !ent->dmgparent || !ent->dmgparent->takedamage )) {
			continue;
		}

		G_AdjustedDamageVec( ent, origin, v );

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage( ent, origin ) ) {
			if( ent->dmgparent ) {
				ent = ent->dmgparent;
			}

			if( AccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;


			G_Damage( ent, inflictor, attacker, dir, origin, (int)points, flags, mod );
		} else {
			VectorAdd( ent->r.absmin, ent->r.absmax, midpoint );
			VectorScale( midpoint, 0.5, midpoint );
			VectorCopy( midpoint, dest );
		
			trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
			if( tr.fraction < 1.0 ) {
				VectorSubtract( dest, origin, dest );
				dist = VectorLength( dest );
				if( dist < radius * 0.2f ) { // closer than 1/4 dist
					if( ent->dmgparent ) {
						ent = ent->dmgparent;
					}

					if( AccuracyHit( ent, attacker ) ) {
						hitClient = qtrue;
					}
					VectorSubtract (ent->r.currentOrigin, origin, dir);
					dir[2] += 24;
					G_Damage( ent, inflictor, attacker, dir, origin, (int)(points*0.1f), flags, mod );
				}
			}
		}
	}
	return hitClient;
}

/*
============
etpro_RadiusDamage
mutation of G_RadiusDamage which lets us selectively damage only clients or only non clients
============
*/
qboolean etpro_RadiusDamage( vec3_t origin, gentity_t *inflictor, gentity_t *attacker, float damage, float radius, gentity_t *ignore, int mod, RadiusScope scope ) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;
	float		boxradius;
	vec3_t		dest; 
	trace_t		tr;
	vec3_t		midpoint;
	int			flags = DAMAGE_RADIUS;

	if( mod == MOD_SATCHEL || mod == MOD_LANDMINE ) {
		flags |= DAMAGE_HALF_KNOCKBACK;
	}

	if( radius < 1 ) {
		radius = 1;
	}

	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement -- 
	// bounding box was checking against radius / sqrt(2) if collision is along box plane
	for( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - boxradius;
		maxs[i] = origin[i] + boxradius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if( ent == ignore ) {
			continue;
		}
		if( !ent->takedamage && ( !ent->dmgparent || !ent->dmgparent->takedamage )) {
			continue;
		}

        switch (scope) {
            default:
            case RADIUS_SCOPE_ANY:
                break;

            case RADIUS_SCOPE_CLIENTS:
		        if (!ent->client && ent->s.eType != ET_CORPSE )
                    continue;
                break;

            case RADIUS_SCOPE_NOCLIENTS:
		        if (ent->client)
                    continue;
                break;
		}

		if(	ent->waterlevel == 3 && mod == MOD_POISON_GAS) {
			continue;
		}

		G_AdjustedDamageVec( ent, origin, v );

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage( ent, origin ) ) {
			if( ent->dmgparent ) {
				ent = ent->dmgparent;
			}

			if( AccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;

			G_Damage( ent, inflictor, attacker, dir, origin, (int)points, flags, mod );
		} else {
			VectorAdd( ent->r.absmin, ent->r.absmax, midpoint );
			VectorScale( midpoint, 0.5, midpoint );
			VectorCopy( midpoint, dest );
		
			trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
			if( tr.fraction < 1.0 ) {
				VectorSubtract( dest, origin, dest );
				dist = VectorLength( dest );
				if( dist < radius * 0.2f ) { // closer than 1/4 dist
					if( ent->dmgparent ) {
						ent = ent->dmgparent;
					}

					if( AccuracyHit( ent, attacker ) ) {
						hitClient = qtrue;
					}
					VectorSubtract (ent->r.currentOrigin, origin, dir);
					dir[2] += 24;
					G_Damage( ent, inflictor, attacker, dir, origin, (int)(points*0.1f), flags, mod );
				}
			}
		}
	}

	return hitClient;
}
