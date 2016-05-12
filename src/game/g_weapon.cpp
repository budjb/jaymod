/*
 * name:		g_weapon.c
 *
 * desc:		perform the server side effects of a weapon firing
 *
*/

#include <bgame/impl.h>
#include <omnibot/et/g_etbot_interface.h>

namespace {

///////////////////////////////////////////////////////////////////////////////

vec3_t	__forward;
vec3_t	__right;
vec3_t	__up;
vec3_t	__muzzleEffect;

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

vec3_t __muzzleTrace;

// forward dec
void Bullet_Fire (gentity_t *ent, float spread, int damage, qboolean distance_falloff);

int G_GetWeaponDamage( int weapon ); // JPW

qboolean G_WeaponIsExplosive( meansOfDeath_t mod )
{
	switch( mod ) {
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
		case MOD_PANZERFAUST:
		case MOD_LANDMINE:
		case MOD_GPG40:
		case MOD_M7:
		case MOD_ARTY:
		case MOD_AIRSTRIKE:
		case MOD_MORTAR:
		case MOD_SATCHEL:
		case MOD_DYNAMITE:
		// map entity based explosions
		case MOD_GRENADE:
		case MOD_MAPMORTAR:
		case MOD_MAPMORTAR_SPLASH:
		case MOD_EXPLOSIVE:
		case MOD_TELEFRAG: // Gordon: yes this _SHOULD_ be here, kthxbye
		case MOD_CRUSH:
			return qtrue;
		default:
			return qfalse;
	}
}

int G_GetWeaponClassForMOD( meansOfDeath_t mod )
{
	switch( mod ) {
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
		case MOD_PANZERFAUST:
		case MOD_LANDMINE:
		case MOD_GPG40:
		case MOD_M7:
		case MOD_ARTY:
		case MOD_AIRSTRIKE:
		case MOD_MORTAR:
		// map entity based explosions
		case MOD_GRENADE:
		case MOD_MAPMORTAR:
		case MOD_MAPMORTAR_SPLASH:
		case MOD_EXPLOSIVE:
		case MOD_POISON_GAS:
			return 0;
		case MOD_SATCHEL:
			return 1;
		case MOD_DYNAMITE:
			return 2;
		default:
			return -1;
	}
}

#define NUM_NAILSHOTS 10

/*
======================================================================

KNIFE/GAUNTLET (NOTE: gauntlet is now the Zombie melee)

======================================================================
*/

// Let's use the same angle between function we've used before
extern float sAngleBetweenVectors(vec3_t a, vec3_t b);

/*
==============
Weapon_Knife
==============
*/
void Weapon_Knife( gentity_t *ent ) {
	gentity_t	*traceEnt, *tent;
	int			damage, mod;
	vec3_t		pforward, eforward;

	vec3_t		end;

	mod = MOD_KNIFE;

	// Jaybird - because the knife now has ammo, add it back.
	ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_KNIFE)] += 1;

	AngleVectors (ent->client->ps.viewangles, __forward, __right, __up);
	CalcMuzzlePoint ( ent, ent->s.weapon, __forward, __right, __up, __muzzleTrace );
	VectorMA (__muzzleTrace, CH_KNIFE_DIST, __forward, end);

    TraceContext trx(*ent, *ent, MASK_SHOT, reinterpret_cast<vec_t(&)[3]>(*__muzzleTrace), reinterpret_cast<vec_t(&)[3]>(*end) );
    if (!AbstractBulletModel::fireWorld( trx ))
        return;

    trace_t& tr = trx.data;

	if ( tr.surfaceFlags & SURF_NOIMPACT )
		return;

	// no contact
	if(tr.fraction == 1.0f)
		return;

	if(tr.entityNum >= MAX_CLIENTS) {	// world brush or non-player entity (no blood)
		tent = G_TempEntity( tr.endpos, EV_MISSILE_MISS );
	} else {							// other player
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
	}

	tent->s.otherEntityNum = tr.entityNum;
	tent->s.eventParm = DirToByte( tr.plane.normal );
	tent->s.weapon = ent->s.weapon;
	tent->s.clientNum = ent->r.ownerNum;

	if(tr.entityNum == ENTITYNUM_WORLD)	// don't worry about doing any damage
		return;

	traceEnt = &g_entities[ tr.entityNum ];

	if(!(traceEnt->takedamage))
		return;

	damage = G_GetWeaponDamage(ent->s.weapon); // JPW		// default knife damage for frontal attacks

	/* CHECK WITH PAUL */
	if( ent->client->sess.playerType == PC_COVERTOPS )
		damage *= 2;	// Watch it - you could hurt someone with that thing!

	// CHRUKER: b002 - Only do backstabs if the body is standing up (ie. alive)
	if(traceEnt->client && traceEnt->health > 0) 
	{
		AngleVectors (ent->client->ps.viewangles,		pforward, NULL, NULL);
		AngleVectors (traceEnt->client->ps.viewangles,	eforward, NULL, NULL);

		if( DotProduct( eforward, pforward ) > 0.6f )		// from behind(-ish)
		{
			damage = 100;	// enough to drop a 'normal' (100 health) human with one jab
			mod = MOD_KNIFE;

			if ( ent->client->sess.skill[SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS] >= 4 ) {
				damage = traceEnt->health;
			}
		}
	}

	G_Damage( traceEnt, ent, ent, vec3_origin, tr.endpos, (damage + rand()%5), 0, mod);
}

// JPW NERVE

//make it TR_LINEAR so it doesnt chew bandwidth...
void MagicSink( gentity_t *self ) {
    self->clipmask = 0;
    self->r.contents = 0;

    self->nextthink = level.time + 4000;
    self->think = G_FreeEntity;

    self->s.pos.trType = TR_LINEAR;
    self->s.pos.trTime = level.time;
    VectorCopy( self->r.currentOrigin, self->s.pos.trBase );
    VectorSet( self->s.pos.trDelta, 0, 0, -5 );
}

/*
======================
  Weapon_Class_Special
	class-specific in multiplayer
======================
*/
// JPW NERVE
void Weapon_Medic( gentity_t *ent ) {
	gitem_t *item;
	gentity_t *ent2;
	vec3_t	velocity, offset;
	vec3_t	angles,mins,maxs;
	vec3_t	tosspos, viewpos;
	trace_t	tr;

	if (level.time - ent->client->ps.classWeaponTime > level.medicChargeTime[ent->client->sess.sessionTeam-1]) {
		ent->client->ps.classWeaponTime = level.time - level.medicChargeTime[ent->client->sess.sessionTeam-1];
	}

	if (ent->client->sess.skill[SK_FIRST_AID] >= 5 && (cvars::bg_sk5_medic.ivalue & SK5_MED_CHARGE))
		ent->client->ps.classWeaponTime += int( level.medicChargeTime[ent->client->sess.sessionTeam-1] * 0.15f * SK5G_CHARGE_FACTOR );
	else if (ent->client->sess.skill[SK_FIRST_AID] >= 2)
		ent->client->ps.classWeaponTime += int( level.medicChargeTime[ent->client->sess.sessionTeam-1] * 0.15f );
	else
		ent->client->ps.classWeaponTime += int( level.medicChargeTime[ent->client->sess.sessionTeam-1] * 0.25f );

	item = BG_FindItemForClassName("item_health");
	VectorCopy( ent->client->ps.viewangles, angles );

	// clamp pitch
	// Jaybird - g_misc allowance
	if (!(cvars::bg_misc.ivalue & MISC_PACKZ)) {
		if ( angles[PITCH] < -30 ) {
			angles[PITCH] = -30;
		} else if ( angles[PITCH] > 30 ) {
			angles[PITCH] = 30;
		}
	}

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 64, offset);
	offset[2] += ent->client->ps.viewheight/2;
	// Jaybird - packdistance
	VectorScale( velocity, 75 * g_packDistance.integer, velocity );
	velocity[2] += 50 + crandom() * 25;

	VectorCopy( __muzzleEffect, tosspos );
	VectorMA( tosspos, 48, __forward, tosspos );
	VectorCopy( ent->client->ps.origin, viewpos );

	VectorSet( mins, -(ITEM_RADIUS + 8), -(ITEM_RADIUS+8), 0 );
	VectorSet( maxs, (ITEM_RADIUS + 8), (ITEM_RADIUS+8), 2*(ITEM_RADIUS+8) );

	trap_EngineerTrace( &tr, viewpos, mins, maxs, tosspos, ent->s.number, MASK_MISSILESHOT );
	if( tr.startsolid ) {
		// Arnout: this code is a bit more solid than the previous code
		VectorCopy( __forward, viewpos );
		VectorNormalizeFast( viewpos );
		VectorMA( ent->r.currentOrigin, -24.f, viewpos, viewpos ); 

		trap_EngineerTrace(&tr, viewpos, mins, maxs, tosspos, ent->s.number, MASK_MISSILESHOT);

		VectorCopy( tr.endpos, tosspos );
	} else if( tr.fraction < 1 ) {	// oops, bad launch spot
		VectorCopy( tr.endpos, tosspos );
		SnapVectorTowards( tosspos, viewpos );
	}

    ent2 = LaunchItem( item, tosspos, velocity, ent->s.number );
    ent2->think = MagicSink;
    ent2->nextthink = level.time + 30000;
//	ent2->timestamp = level.time + 31200;

	ent2->parent = ent; // JPW NERVE so we can score properly later
	//ent2->count = 20;

	// Omni-bot - Send a fire event.
	Bot_Event_FireWeapon(ent-g_entities, Bot_WeaponGameToBot(ent->s.weapon), ent2);
}

/*
==========
G_PlaceTripmine
==========
*/
void G_PlaceTripmine(gentity_t* ent) {
	vec3_t start, end;
	trace_t trace;
	gentity_t* bomb;
	vec3_t forward;

	VectorCopy( ent->client->ps.origin, start );
	start[2] += ent->client->ps.viewheight;

	AngleVectors(ent->client->ps.viewangles, forward, NULL, NULL);

	VectorMA(start, 64, forward, end);

	trap_Trace(&trace, start, NULL, NULL, end, ent->s.number, MASK_SHOT);

	bomb = G_Spawn();
	bomb->r.svFlags	= SVF_BROADCAST;
	bomb->s.eType = ET_BOMB;
	bomb->s.eFlags = 0;
	bomb->s.weapon = WP_TRIPMINE;
	bomb->parent = ent;
	bomb->think = G_TripMinePrime;
	bomb->nextthink = level.time + 2000;
	bomb->splashDamage = 300;
	bomb->splashRadius = 300;
	bomb->methodOfDeath = MOD_TRIPMINE;
	bomb->splashMethodOfDeath = MOD_TRIPMINE;
	bomb->r.contents = CONTENTS_CORPSE;	// (player can walk through)

	VectorSet(bomb->r.mins, -12, -12, 0);
	VectorCopy(bomb->r.mins, bomb->r.absmin);
	VectorSet(bomb->r.maxs, 12, 12, 20);
	VectorCopy(bomb->r.maxs, bomb->r.absmax);

	VectorMA(trace.endpos, 1, trace.plane.normal, start);
	G_SetOrigin(bomb, start);
	G_SetAngle(bomb, vec3_origin);

	VectorCopy(trace.plane.normal, bomb->s.origin2);

	trap_LinkEntity(bomb);
}

/*void Weapon_SatchelCharge(gentity_t *ent) {
	gitem_t *item;
	gentity_t *ent2;
	vec3_t	velocity, org, offset;
	vec3_t	angles,mins,maxs;
	trace_t	tr;

	item = BG_FindItem("Satchel Charge");
	VectorCopy( ent->client->ps.viewangles, angles );

	// clamp pitch
	if ( angles[PITCH] < -30 )
		angles[PITCH] = -30;
	else if ( angles[PITCH] > 30 )
		angles[PITCH] = 30;

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 64, offset);
	offset[2] += ent->client->ps.viewheight/2;
	VectorScale( velocity, 75, velocity );
	velocity[2] += 50 + crandom() * 25;

	VectorAdd(ent->client->ps.origin,offset,org);

	VectorSet( mins, -ITEM_RADIUS, -ITEM_RADIUS, 0 );
	VectorSet( maxs, ITEM_RADIUS, ITEM_RADIUS, 2*ITEM_RADIUS );

	trap_Trace (&tr, ent->client->ps.origin, mins, maxs, org, ent->s.number, MASK_SOLID);
	VectorCopy( tr.endpos, org );

	ent2 = LaunchItem( item, org, velocity, ent->s.number );
	ent2->think = MagicSink;
	ent2->timestamp = level.time + 4000;
	ent2->parent = ent;
	ent2->s.eType = ET_MISSILE;
	ent2->methodOfDeath = MOD_SATCHEL;
	ent2->splashMethodOfDeath = MOD_SATCHEL;
	ent2->s.weapon = WP_SATCHEL;
	ent2->touch = 0;
}
*/

// JPW NERVE
/*
==================
Weapon_MagicAmmo
==================
*/
void Weapon_MagicAmmo( gentity_t *ent )  {
	gitem_t *item;
	gentity_t *ent2;
	vec3_t	velocity, offset;
	vec3_t	tosspos, viewpos;
	vec3_t	angles,mins,maxs;
	trace_t	tr;

	if (level.time - ent->client->ps.classWeaponTime > level.lieutenantChargeTime[ent->client->sess.sessionTeam-1])
		ent->client->ps.classWeaponTime = level.time - level.lieutenantChargeTime[ent->client->sess.sessionTeam-1];

	if (ent->client->sess.skill[SK_SIGNALS] >= 5 && (cvars::bg_sk5_fdops.ivalue & SK5_FDO_CHARGE))
		ent->client->ps.classWeaponTime += int( level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] * 0.15f * SK5G_CHARGE_FACTOR );
	else if (ent->client->sess.skill[SK_SIGNALS] >= 1)
		ent->client->ps.classWeaponTime += int( level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] * 0.15f );
	else
		ent->client->ps.classWeaponTime += int( level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] * 0.25f );

	item = BG_FindItem( ent->client->sess.skill[SK_SIGNALS] >= 1 ? "Mega Ammo Pack" : "Ammo Pack" );	
	VectorCopy( ent->client->ps.viewangles, angles );

	// clamp pitch
	// Jaybird - g_misc allowance
	if (!(cvars::bg_misc.ivalue & MISC_PACKZ)) {
		if ( angles[PITCH] < -30 )
			angles[PITCH] = -30;
		else if ( angles[PITCH] > 30 )
			angles[PITCH] = 30;
	}

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 64, offset);
	offset[2] += ent->client->ps.viewheight/2;
	// Jaybird - pack distance
	VectorScale( velocity, 75 * g_packDistance.integer, velocity );
	velocity[2] += 50 + crandom() * 25;

	VectorCopy( __muzzleEffect, tosspos );
	VectorMA( tosspos, 48, __forward, tosspos );
	VectorCopy( ent->client->ps.origin, viewpos );

	VectorSet( mins, -(ITEM_RADIUS + 8), -(ITEM_RADIUS+8), 0 );
	VectorSet( maxs, (ITEM_RADIUS + 8), (ITEM_RADIUS+8), 2*(ITEM_RADIUS+8) );

	trap_EngineerTrace( &tr, viewpos, mins, maxs, tosspos, ent->s.number, MASK_MISSILESHOT );
	if( tr.startsolid ) {
		// Arnout: this code is a bit more solid than the previous code
		VectorCopy( __forward, viewpos );
		VectorNormalizeFast( viewpos );
		VectorMA( ent->r.currentOrigin, -24.f, viewpos, viewpos ); 

		trap_EngineerTrace (&tr, viewpos, mins, maxs, tosspos, ent->s.number, MASK_MISSILESHOT);

		VectorCopy( tr.endpos, tosspos );
	} else if( tr.fraction < 1 ) {	// oops, bad launch spot
		VectorCopy( tr.endpos, tosspos );
		SnapVectorTowards( tosspos, viewpos );
	}

    ent2 = LaunchItem( item, tosspos, velocity, ent->s.number );
    ent2->think = MagicSink;
    ent2->nextthink = level.time + 30000;
//	ent2->timestamp = level.time + 31200;

	ent2->parent = ent;

	if( ent->client->sess.skill[SK_SIGNALS] >= 1 ) {
		ent2->count = 2;
		ent2->s.density = 2;
	} else {
		ent2->count = 1;
		ent2->s.density = 1;
	}

	// Omni-bot - Send a fire event.
	Bot_Event_FireWeapon(ent-g_entities, Bot_WeaponGameToBot(ent->s.weapon), ent2);
}
// jpw



// START - Mad Doc - TDF
// took this out of Weapon_Syringe so we can use it from other places
qboolean ReviveEntity(gentity_t *ent, gentity_t *traceEnt)
{
	vec3_t		org;
	trace_t		tr;
	int			healamt, headshot, oldweapon,oldweaponstate,oldclasstime=0;
	qboolean	usedSyringe = qfalse;		// DHM - Nerve
	int			ammo[MAX_WEAPONS];		// JPW NERVE total amount of ammo
	int			ammoclip[MAX_WEAPONS];	// JPW NERVE ammo in clip
	int			weapons[MAX_WEAPONS/(sizeof(int)*8)];	// JPW NERVE 64 bits for weapons held
//	gentity_t	*traceEnt,
	gentity_t	*te;




	// heal the dude
	// copy some stuff out that we'll wanna restore
	VectorCopy(traceEnt->client->ps.origin, org);
	headshot = traceEnt->client->ps.eFlags & EF_HEADSHOT;

	// Jaybird - battle sense revive (bsrevive)
	if( ent->client->sess.skill[SK_FIRST_AID] >= 3 || (traceEnt->client->sess.skill[SK_BATTLE_SENSE] >= 4 && (cvars::bg_misc.ivalue & MISC_BSREVIVE)))
		healamt = traceEnt->client->ps.stats[STAT_MAX_HEALTH];
	else
		healamt = int( traceEnt->client->ps.stats[STAT_MAX_HEALTH] * 0.5f );
	oldweapon = traceEnt->client->ps.weapon;
	oldweaponstate = traceEnt->client->ps.weaponstate;

	// keep class special weapon time to keep them from exploiting revives
	oldclasstime = traceEnt->client->ps.classWeaponTime;

	memcpy(ammo,traceEnt->client->ps.ammo,sizeof(int)*MAX_WEAPONS);
	memcpy(ammoclip,traceEnt->client->ps.ammoclip,sizeof(int)*MAX_WEAPONS);
	memcpy(weapons,traceEnt->client->ps.weapons,sizeof(int)*(MAX_WEAPONS/(sizeof(int)*8)));

	ClientSpawn(traceEnt, qtrue);

	Bot_Event_Revived(traceEnt-g_entities, ent);

	traceEnt->client->ps.stats[STAT_PLAYER_CLASS] = traceEnt->client->sess.playerType;
	memcpy(traceEnt->client->ps.ammo,ammo,sizeof(int)*MAX_WEAPONS);
	memcpy(traceEnt->client->ps.ammoclip,ammoclip,sizeof(int)*MAX_WEAPONS);
	memcpy(traceEnt->client->ps.weapons,weapons,sizeof(int)*(MAX_WEAPONS/(sizeof(int)*8)));

	if( headshot ) {
		traceEnt->client->ps.eFlags |= EF_HEADSHOT;
	}
	traceEnt->client->ps.weapon = oldweapon;
	traceEnt->client->ps.weaponstate = oldweaponstate;

	// set idle animation on weapon
	traceEnt->client->ps.weapAnim = ( ( traceEnt->client->ps.weapAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | PM_IdleAnimForWeapon( traceEnt->client->ps.weapon );

	traceEnt->client->ps.classWeaponTime = oldclasstime;

	traceEnt->health = healamt;
	VectorCopy(org,traceEnt->s.origin);
	VectorCopy(org,traceEnt->r.currentOrigin);
	VectorCopy(org,traceEnt->client->ps.origin);

	trap_Trace(&tr, traceEnt->client->ps.origin, traceEnt->client->ps.mins, traceEnt->client->ps.maxs, traceEnt->client->ps.origin, traceEnt->s.number, MASK_PLAYERSOLID);
	if ( tr.allsolid ) {
		traceEnt->client->ps.pm_flags |= PMF_DUCKED;
	}

	traceEnt->r.contents = CONTENTS_CORPSE;
	trap_LinkEntity( ent );


	// DHM - Nerve :: Let the person being revived know about it
	trap_SendServerCommand( traceEnt-g_entities, va("cp \"You have been revived by [lof]%s[lon] [lof]%s!\n\"", ent->client->sess.sessionTeam == TEAM_ALLIES ? rankNames_Allies[ ent->client->sess.rank ] : rankNames_Axis[ ent->client->sess.rank ], ent->client->pers.netname) );
	traceEnt->props_frame_state = ent->s.number;

	// DHM - Nerve :: Mark that the medicine was indeed dispensed
	usedSyringe = qtrue;

	// sound
	te = G_TempEntity( traceEnt->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = G_SoundIndex( "sound/misc/vo_revive.wav" );

	// Xian -- This was gay and I always hated it.
	if ( g_fastres.integer > 0 )
		BG_AnimScriptEvent( &traceEnt->client->ps, traceEnt->client->pers.character->animModelInfo, ANIM_ET_JUMP, qfalse, qtrue );
	else {
		// DHM - Nerve :: Play revive animation
		BG_AnimScriptEvent( &traceEnt->client->ps, traceEnt->client->pers.character->animModelInfo, ANIM_ET_REVIVE, qfalse, qtrue );
		traceEnt->client->ps.pm_flags |= PMF_TIME_LOCKPLAYER;
		traceEnt->client->ps.pm_time = 2100;
	}

	// Tell the caller if we actually used a syringe
	return usedSyringe;

}
// END - Mad Doc


// JPW NERVE Weapon_Syringe:
/*
======================
  Weapon_Syringe
	shoot the syringe, do the old lazarus bit
======================
*/
void Weapon_Syringe(gentity_t *ent) {
	vec3_t		end;
	trace_t		tr;
	qboolean	usedSyringe = qfalse;		// DHM - Nerve
	gentity_t	*traceEnt;

	AngleVectors (ent->client->ps.viewangles, __forward, __right, __up);
	CalcMuzzlePointForActivate( ent, __forward, __right, __up, __muzzleTrace );
	// Jaybird - use the define
	VectorMA (__muzzleTrace, CH_REVIVE_DIST, __forward, end);			// CH_ACTIVATE_DIST
	//VectorMA (muzzleTrace, -16, forward, muzzleTrace);	// DHM - Back up the start point in case medic is
														// right on top of intended revivee.
	trap_Trace (&tr, __muzzleTrace, NULL, NULL, end, ent->s.number, MASK_SHOT);

	if (tr.startsolid) {
		VectorMA (__muzzleTrace, 8, __forward, end);			// CH_ACTIVATE_DIST
		trap_Trace(&tr, __muzzleTrace, NULL, NULL, end, ent->s.number, MASK_SHOT);
	}

	if (tr.fraction < 1.0) {
		traceEnt = &g_entities[ tr.entityNum ];
		if (traceEnt->client != NULL) {

			if ( traceEnt->client->ps.pm_type == PM_DEAD && traceEnt->client->sess.sessionTeam == ent->client->sess.sessionTeam ) {
				// Mad Doc - TDF moved all the revive stuff into its own function
				usedSyringe = ReviveEntity( ent, traceEnt );

				if (usedSyringe) {
					// Jaybird - log the revive for shrubbot shortcuts
					Q_strncpyz(traceEnt->client->pers.lastrevive, ent->client->pers.netname, sizeof(traceEnt->client->pers.lastrevive));

					// Stats
					ent->client->sess.revives++;
				}

				// OSP - syringe "hit"
				if(cvars::gameState.ivalue == GS_PLAYING) ent->client->sess.aWeaponStats[WS_SYRINGE].hits++;
				if(ent && ent->client) G_LogPrintf("Medic_Revive: %d %d\n", ent - g_entities, traceEnt - g_entities);	// OSP

				if( !traceEnt->isProp ) { // Gordon: flag for if they were teamkilled or not
					AddScore(ent, WOLF_MEDIC_BONUS); // JPW NERVE props to the medic for the swift and dexterous bit o healitude

					G_AddSkillPoints( ent, SK_FIRST_AID, 4.f );
					G_DebugAddSkillPoints( ent, SK_FIRST_AID, 4.f, "reviving a player" );
				}

				// Arnout: calculate ranks to update numFinalDead arrays. Have to do it manually as addscore has an early out
				if( g_gametype.integer == GT_WOLF_LMS ) {
					CalculateRanks();
				}
			}
		}
	}

	// DHM - Nerve :: If the medicine wasn't used, give back the ammo
	if (!usedSyringe)
		ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDIC_SYRINGE)] += 1;

}
// jpw

/*
======================
  Weapon_AdrenalineSyringe
	Hmmmm. Needles. With stuff in it. Woooo.
======================
*/
void Weapon_AdrenalineSyringe(gentity_t *ent) {
	ent->client->ps.powerups[PW_ADRENALINE] = level.time + 10000;
}

/*
======================
  Weapon_AdrenalineShare
	shoot the syringe, help a teammate ;]
======================
*/
void Weapon_AdrenalineShare(gentity_t *ent) {
	vec3_t		end;
	trace_t		tr;
	gentity_t	*traceEnt;
	gentity_t	*te;

	if (!(g_medics.integer & MEDIC_SHAREADRENALINE))
		return;

	AngleVectors (ent->client->ps.viewangles, __forward, __right, __up);
	CalcMuzzlePointForActivate( ent, __forward, __right, __up, __muzzleTrace );
	// Jaybird - use the define
	VectorMA (__muzzleTrace, CH_REVIVE_DIST, __forward, end);			// CH_ACTIVATE_DIST
	//VectorMA (muzzleTrace, -16, forward, muzzleTrace);	// DHM - Back up the start point in case medic is
														// right on top of intended revivee.
	trap_Trace (&tr, __muzzleTrace, NULL, NULL, end, ent->s.number, MASK_SHOT);

	if (tr.startsolid) {
		VectorMA (__muzzleTrace, 8, __forward, end);			// CH_ACTIVATE_DIST
		trap_Trace(&tr, __muzzleTrace, NULL, NULL, end, ent->s.number, MASK_SHOT);
	}

	if (tr.fraction < 1.0) {
		traceEnt = &g_entities[ tr.entityNum ];
		if (traceEnt->client != NULL) {

			// Jaybird - playdead check
			if ( ISONTEAM(traceEnt) && OnSameTeam(ent, traceEnt) && traceEnt->client->ps.stats[STAT_HEALTH] > 0 && !traceEnt->client->ps.powerups[PW_ADRENALINE]) {
				ent->client->ps.classWeaponTime = level.time;
				Weapon_AdrenalineSyringe(traceEnt);
				te = G_TempEntity( traceEnt->r.currentOrigin, EV_GENERAL_SOUND );
				te->s.eventParm = G_SoundIndex( "sound/misc/vo_revive.wav" );
				return;
			}
		}
	}
	ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDIC_SYRINGE)] += 1;
}

/*
======================
  Weapon_PoisonSyringe
	shoot the syringe, give someone the shits ;]
======================
*/
void Weapon_PoisonSyringe(gentity_t *ent) {
    // Default to adding ammo back.
    const weapon_t clip = BG_FindClipForWeapon( WP_POISON_SYRINGE );
	ent->client->ps.ammoclip[clip]++;

    AngleVectors( ent->client->ps.viewangles, __forward, __right, __up );
    CalcMuzzlePointForActivate( ent, __forward, __right, __up, __muzzleTrace );

    vec3_t end;
    VectorMA( __muzzleTrace, CH_REVIVE_DIST, __forward, end );

    TraceContext trx(
        *ent,
        *ent,
        MASK_SHOT,
        reinterpret_cast<vec_t(&)[3]>(*__muzzleTrace),
        reinterpret_cast<vec_t(&)[3]>(*end) );

    // skip if nothing hit
    if (!AbstractBulletModel::fireWorld( trx ))
        return;

    // skip if player not hit
    if (!trx.resultIsPlayer())
        return;

    Client& victim = g_clientObjects[trx.data.entityNum];

    // skip if victim is god
    if (victim.gentity.flags & FL_GODMODE)
        return;

    // skip if victim is dead
    if (victim.gclient.ps.stats[STAT_HEALTH] <= 0)
        return;

    // skip if victim is invulnerable
    if (victim.gclient.ps.powerups[PW_INVULNERABLE])
        return;

    // skip if no friendly-fire and victim is on same team
    if (!g_friendlyFire.integer && OnSameTeam( ent, &victim.gentity ))
        return;

    // all criteria satisfied, proceed
	ent->client->ps.ammoclip[clip]--;
    G_AddPoisonEvent( &victim.gentity, ent );

    // play sound
    gentity_t& te = *G_TempEntity( victim.gentity.r.currentOrigin, EV_GENERAL_SOUND );
    te.s.eventParm = G_SoundIndex( "sound/misc/vo_revive.wav" );
}

void G_ExplodeMissile( gentity_t *ent );
void DynaSink(gentity_t* self );

// Arnout: crude version of G_RadiusDamage to see if the dynamite can damage a func_constructible
int EntsThatRadiusCanDamage( vec3_t origin, float radius, int *damagedList ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	int			i, e;
	float		boxradius;
	vec3_t		dest; 
	trace_t		tr;
	vec3_t		midpoint;
	int			numDamaged = 0;

	if ( radius < 1 ) {
		radius = 1;
	}

	boxradius = 1.41421356 * radius; // radius * sqrt(2) for bounding box enlargement -- 
	// bounding box was checking against radius / sqrt(2) if collision is along box plane
	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - boxradius;
		maxs[i] = origin[i] + boxradius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (!ent->r.bmodel)
			VectorSubtract(ent->r.currentOrigin,origin,v);
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

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		if( CanDamage (ent, origin) ) {
			damagedList[numDamaged++] = entityList[e];
		} else {
			VectorAdd (ent->r.absmin, ent->r.absmax, midpoint);
			VectorScale (midpoint, 0.5, midpoint);
			VectorCopy (midpoint, dest);
			
			trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
			if (tr.fraction < 1.0) {
				VectorSubtract(dest,origin,dest);
				dist = VectorLength(dest);
				if (dist < radius*0.2f) { // closer than 1/4 dist
					damagedList[numDamaged++] = entityList[e];
				}
			}
		}
	}

	return( numDamaged );
}

void G_LandminePrime( gentity_t *self );
extern void explosive_indicator_think( gentity_t *ent );

#define MIN_BLOCKINGWARNING_INTERVAL 5000

static void MakeTemporarySolid( gentity_t *ent ) {
	if( ent->entstate == STATE_UNDERCONSTRUCTION ) {
		ent->clipmask = ent->realClipmask;
		ent->r.contents = ent->realContents;
		if( !ent->realNonSolidBModel )
			ent->s.eFlags &= ~EF_NONSOLID_BMODEL;
	}

	trap_LinkEntity( ent );
}

static void UndoTemporarySolid( gentity_t *ent ) {
	ent->entstate = STATE_UNDERCONSTRUCTION;
	ent->s.powerups = STATE_UNDERCONSTRUCTION;
	ent->realClipmask = ent->clipmask;
	ent->clipmask = 0;
	ent->realContents = ent->r.contents;
	ent->r.contents = 0;
	if( ent->s.eFlags & EF_NONSOLID_BMODEL )
		ent->realNonSolidBModel = qtrue;
	else
		ent->s.eFlags |= EF_NONSOLID_BMODEL;

	trap_LinkEntity( ent );
}

// handleBlockingEnts = kill players, return flags, remove entities
// warnBlockingPlayers = warn any players that are in the constructible area
static void HandleEntsThatBlockConstructible( gentity_t *constructor, gentity_t *constructible, qboolean handleBlockingEnts, qboolean warnBlockingPlayers ) {
	// check if something blocks us
	int constructibleList[MAX_GENTITIES];
	int entityList[MAX_GENTITIES];
	int blockingList[MAX_GENTITIES];
	int constructibleEntities = 0;
	int listedEntities, e;
	int blockingEntities = 0;
	gentity_t *check, *block;

	// backup...
	int constructibleModelindex = constructible->s.modelindex;
	int constructibleClipmask = constructible->clipmask;
	int constructibleContents = constructible->r.contents;
	int constructibleNonSolidBModel = (constructible->s.eFlags & EF_NONSOLID_BMODEL);

	trap_SetBrushModel( constructible, va( "*%i", constructible->s.modelindex2 ) );

	// ...and restore
	constructible->clipmask = constructibleClipmask;
	constructible->r.contents = constructibleContents;
	if( !constructibleNonSolidBModel )
		constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;
	trap_LinkEntity( constructible );

	// store our origin
	VectorCopy( constructible->r.absmin, constructible->s.origin2 );
	VectorAdd( constructible->r.absmax, constructible->s.origin2, constructible->s.origin2 );
	VectorScale( constructible->s.origin2, 0.5, constructible->s.origin2 );

	// get all the entities that make up the constructible
	if( constructible->track && constructible->track[0] ) {
		vec3_t mins, maxs;

		VectorCopy( constructible->r.absmin, mins );
		VectorCopy( constructible->r.absmax, maxs );

		check = NULL;

		while(1) {
			check = G_Find( check, FOFS(track), constructible->track );

			if( check == constructible )
				continue;

			if (!check ) {
				break;
			}

			if( constructible->count2 ) {
				if( check->partofstage != constructible->grenadeFired )
					continue;
			}

			// get the bounding box of all entities in the constructible together
			AddPointToBounds( check->r.absmin, mins, maxs );
			AddPointToBounds( check->r.absmax, mins, maxs );

			constructibleList[constructibleEntities++] = check->s.number;
		}

		listedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

		// make our constructible entities solid so we can check against them
		//trap_LinkEntity( constructible );
		MakeTemporarySolid( constructible );
		for( e = 0; e < constructibleEntities; e++ ) {
			check = &g_entities[constructibleList[e]];

			//trap_LinkEntity( check );
			MakeTemporarySolid( check );
		}

	} else {
		// Gordon: changed * to abs*
		listedEntities = trap_EntitiesInBox( constructible->r.absmin, constructible->r.absmax, entityList, MAX_GENTITIES );

		// make our constructible solid so we can check against it
		//trap_LinkEntity( constructible );
		MakeTemporarySolid( constructible );
	}

	for( e = 0; e < listedEntities; e++ ) {
		check = &g_entities[entityList[e]];

		// ignore everything but items, players and missiles (grenades too)
		if( check->s.eType != ET_MISSILE && check->s.eType != ET_ITEM && check->s.eType != ET_PLAYER && !check->physicsObject ) {
			continue;
		}

		// remove any corpses, this includes dynamite
		if( check->r.contents == CONTENTS_CORPSE ) {
			blockingList[blockingEntities++] = entityList[e];
			continue;
		}

		// FIXME : dynamite seems to test out of position?
		// see if the entity is in a solid now
		if((block = G_TestEntityPosition( check )) == NULL)
			continue;

		// the entity is blocked and it is a player, then warn the player
		if( warnBlockingPlayers && check->s.eType == ET_PLAYER ) {
			if( (level.time - check->client->lastConstructibleBlockingWarnTime) >= MIN_BLOCKINGWARNING_INTERVAL ) {
				trap_SendServerCommand( check->s.number, "cp \"Warning, leave the construction area...\" 1" );
				// Gordon: store the entity num to warn the bot
				check->client->lastConstructibleBlockingWarnEnt = constructible - g_entities;
				check->client->lastConstructibleBlockingWarnTime = level.time;
			}

			// unlink our entities again
			/*trap_UnlinkEntity( constructible );

			if( constructible->track && constructible->track[0] ) {
				for( e = 0; e < constructibleEntities; e++ ) {
					check = &g_entities[constructibleList[e]];

					trap_UnlinkEntity( check );
				}
			}
			return;*/
		}

		blockingList[blockingEntities++] = entityList[e];
	}

	// undo the temporary solid for our entities
	UndoTemporarySolid( constructible );
	if( constructible->track && constructible->track[0] ) {
		for( e = 0; e < constructibleEntities; e++ ) {
			check = &g_entities[constructibleList[e]];

			//trap_UnlinkEntity( check );
			UndoTemporarySolid( check );
		}
	}

	if( handleBlockingEnts ) {
		for( e = 0; e < blockingEntities; e++ ) {
			block = &g_entities[blockingList[e]];

			if( block->client || block->s.eType == ET_CORPSE ) {
				G_Damage( block, constructible, constructor, NULL, NULL, 9999, DAMAGE_NO_PROTECTION, MOD_CRUSH_CONSTRUCTION );
			} else if( block->s.eType == ET_ITEM && block->item->giType == IT_TEAM ) {
				// see if it's a critical entity, one that we can't just simply kill (basically flags)
				Team_DroppedFlagThink( block );
			} else {
				// remove the landmine from both teamlists
				if ( block->s.eType == ET_MISSILE && (block->methodOfDeath == MOD_LANDMINE || block->methodOfDeath == MOD_POISON_GAS )) {
					mapEntityData_t	*mEnt;

					if((mEnt = G_FindMapEntityData(&mapEntityData[0], block-g_entities)) != NULL) {
						G_FreeMapEntityData( &mapEntityData[0], mEnt );
					}

					if((mEnt = G_FindMapEntityData(&mapEntityData[1], block-g_entities)) != NULL) {
						G_FreeMapEntityData( &mapEntityData[1], mEnt );
					}
				}

				// just get rid of it
				G_TempEntity( block->s.origin, EV_ITEM_POP );
				G_FreeEntity( block );
			}
		}
	}

	if( constructibleModelindex ) {
		trap_SetBrushModel( constructible, va( "*%i", constructibleModelindex ) );
		// ...and restore
		constructible->clipmask = constructibleClipmask;
		constructible->r.contents = constructibleContents;
		if( !constructibleNonSolidBModel )
			constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;
		trap_LinkEntity( constructible );
	} else {
		constructible->s.modelindex = 0;
		//constructible->clipmask = constructibleClipmask;
		//constructible->r.contents = constructibleContents;
		//if( !constructibleNonSolidBModel )
		//	constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;
		trap_LinkEntity( constructible );
	}
}

#define CONSTRUCT_POSTDECAY_TIME 500

// !! NOTE !!: if the conditions here of a buildable constructible change, then BotIsConstructible() must reflect those changes

// returns qfalse when it couldn't build
static qboolean TryConstructing( gentity_t *ent ) {
	gentity_t *check;
	gentity_t *constructible = ent->client->touchingTOI->target_ent;
	int i;

    Entity& constructibleEntity = g_entityObjects[constructible->s.number];

	// no construction during prematch
	if( level.warmupEndTime )
		return( qfalse );

	// see if we are in a trigger_objective_info targetting multiple func_constructibles
	if( constructible->s.eType == ET_CONSTRUCTIBLE && ent->client->touchingTOI->chain ) {
		gentity_t *otherconstructible = NULL;

		// use the target that has the same team as the player
		if( constructible->s.teamNum != ent->client->sess.sessionTeam ) {
			constructible = ent->client->touchingTOI->chain;
		}

		otherconstructible = constructible->chain;

		// make sure the other constructible isn't built/underconstruction/something
		if( otherconstructible->s.angles2[0] ||
			otherconstructible->s.angles2[1] ||
			( otherconstructible->count2 && otherconstructible->grenadeFired ) ) {

			return( qfalse );
		}
	}

	// see if we are in a trigger_objective_info targetting a func_constructible
	if( constructible->s.eType == ET_CONSTRUCTIBLE &&
		constructible->s.teamNum == ent->client->sess.sessionTeam ) {

		if( constructible->s.angles2[0] >= 250 ) // have to do this so we don't score multiple times
			return( qfalse );

		if( constructible->s.angles2[1] != 0 )
			return( qfalse );

		// Check if we can construct - updates the classWeaponTime as well
		if (!ReadyToConstruct(ent, constructible, qtrue))
			return qtrue;

		// try to start building
		if( constructible->s.angles2[0] <= 0 ) {
			// wait a bit, this prevents network spam
			if( level.time - constructible->lastHintCheckTime < CONSTRUCT_POSTDECAY_TIME )
				return( qtrue );	// likely will come back soon - so override other plier bits anyway

			// Gordon: are we scripted only?
			if( !(ent->spawnflags & CONSTRUCTIBLE_AAS_SCRIPTED) ) {
				if ( !(ent->spawnflags & CONSTRUCTIBLE_NO_AAS_BLOCKING) ) {
					// RF, if we are blocking AAS areas when built, then clear AAS blocking so we can set it again after the stage has been increased
					if( constructible->spawnflags & CONSTRUCTIBLE_BLOCK_PATHS_WHEN_BUILD ) {
						G_SetAASBlockingEntity( ent, AAS_AREA_ENABLED );
					}
				}
			}

			// swap brushmodels if staged
			if( constructible->count2 ) {
				constructible->grenadeFired++;
				constructible->s.modelindex2 = constructible->conbmodels[constructible->grenadeFired-1];
				//trap_SetBrushModel( constructible, va( "*%i", constructible->conbmodels[constructible->grenadeFired-1] ) );
			}

			G_SetEntState( constructible, STATE_UNDERCONSTRUCTION );

			if( !constructible->count2 ) {
				// call script
				G_Script_ScriptEvent( constructible, "buildstart", "final" );
				constructible->s.frame = 1;
			} else {
				if( constructible->grenadeFired == constructible->count2 ) {
					G_Script_ScriptEvent( constructible, "buildstart", "final" );
					constructible->s.frame = constructible->grenadeFired;
				} else {
					switch( constructible->grenadeFired ) {
					case 1: G_Script_ScriptEvent( constructible, "buildstart", "stage1" ); constructible->s.frame = 1; break;
					case 2: G_Script_ScriptEvent( constructible, "buildstart", "stage2" ); constructible->s.frame = 2; break;
					case 3: G_Script_ScriptEvent( constructible, "buildstart", "stage3" ); constructible->s.frame = 3; break;
					}
				}
			}

			// Play sound
			{
				vec3_t mid;
				gentity_t* te;

				VectorAdd( constructible->parent->r.absmin, constructible->parent->r.absmax, mid );
				VectorScale( mid, 0.5f, mid );

				te = G_TempEntity( mid, EV_GENERAL_SOUND );
				te->s.eventParm = G_SoundIndex( "sound/world/build.wav" );
			}
			
			if( ent->client->touchingTOI->chain && ent->client->touchingTOI->count2 ) {
				// find the constructible indicator and change team
				mapEntityData_t	*mEnt;
				mapEntityData_Team_t *teamList;
				gentity_t *indicator = &g_entities[ent->client->touchingTOI->count2];

				indicator->s.teamNum = constructible->s.teamNum;

				// update the map for the other team
				teamList = indicator->s.teamNum == TEAM_AXIS ? &mapEntityData[1] : &mapEntityData[0]; // inversed
				if((mEnt = G_FindMapEntityData( teamList, indicator-g_entities)) != NULL) {
					G_FreeMapEntityData( teamList, mEnt );
				}
			}

			if( !constructible->count2 || constructible->grenadeFired == 1 ) {
				// link in if we just started building
				G_UseEntity( constructible, ent->client->touchingTOI, ent );
			}

			// setup our think function for decaying
			constructible->think = func_constructible_underconstructionthink;
			constructible->nextthink = level.time + FRAMETIME;

			G_PrintClientSpammyCenterPrint( ent-g_entities, "Constructing..." );
		}

		// Give health until it is full, don't continue
        float progress = (255.f/(constructible->constructibleStats.duration/(float)FRAMETIME));
		constructible->s.angles2[0] += progress;

        // Shared constructible sharing
        if (g_engineers.integer & ENGI_SHAREXP) {
            float xpbonus = constructible->constructibleStats.constructxpbonus * (progress / 255.f);
            constructibleEntity.sharedBuildXP[ent->s.number] += xpbonus;
        }

        if ( constructible->s.angles2[0] >= 250 ) {
			constructible->s.angles2[0] = 0;
			HandleEntsThatBlockConstructible( ent, constructible, qtrue, qfalse );
		} else {
			constructible->lastHintCheckTime = level.time;
			HandleEntsThatBlockConstructible( ent, constructible, qfalse, qtrue );
            return( qtrue );	// properly constructed
		}

		if( constructible->count2 ) {
			// backup...
			//int constructibleModelindex = constructible->s.modelindex;
			int constructibleClipmask = constructible->clipmask;
			int constructibleContents = constructible->r.contents;
			int constructibleNonSolidBModel = (constructible->s.eFlags & EF_NONSOLID_BMODEL);

			constructible->s.modelindex2 = 0;
			trap_SetBrushModel( constructible, va( "*%i", constructible->conbmodels[constructible->grenadeFired-1] ) );

			// ...and restore
			constructible->clipmask = constructibleClipmask;
			constructible->r.contents = constructibleContents;
			if( !constructibleNonSolidBModel )
				constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;

			if( constructible->grenadeFired == constructible->count2 ) {
				constructible->s.angles2[1] = 1;
			}
		} else {
			// backup...
			//int constructibleModelindex = constructible->s.modelindex;
			int constructibleClipmask = constructible->clipmask;
			int constructibleContents = constructible->r.contents;
			int constructibleNonSolidBModel = (constructible->s.eFlags & EF_NONSOLID_BMODEL);

			constructible->s.modelindex2 = 0;
			trap_SetBrushModel( constructible, constructible->model );

			// ...and restore
			constructible->clipmask = constructibleClipmask;
			constructible->r.contents = constructibleContents;
			if( !constructibleNonSolidBModel )
				constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;

			constructible->s.angles2[1] = 1;
		}

		AddScore( ent, int(constructible->accuracy) ); // give drop score to guy who built it

        if (g_engineers.integer & ENGI_SHAREXP) {
            // Give XP to those who earned it
            map<uint8,float>::const_iterator max = constructibleEntity.sharedBuildXP.end();
            for (map<uint8,float>::const_iterator it = constructibleEntity.sharedBuildXP.begin(); it != max; it++) {
                int i = (int)it->first;
                float xp = it->second;
       		    G_AddSkillPoints( &g_entities[i], SK_EXPLOSIVES_AND_CONSTRUCTION, xp );
        	    G_DebugAddSkillPoints( &g_entities[i], SK_EXPLOSIVES_AND_CONSTRUCTION, xp, "finishing a construction" );
            }
            // Clear out list
            constructibleEntity.sharedBuildXP.clear();
        } else {
		    G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, constructible->constructibleStats.constructxpbonus );
		    G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, constructible->constructibleStats.constructxpbonus, "finishing a construction" );
        }

		// unlink the objective info to get rid of the indicator for now
		// Arnout: don't unlink, we still want the location popup. Instead, constructible_indicator_think got changed to free
		// the indicator when the constructible is constructed
//			if( constructible->parent )
//				trap_UnlinkEntity( constructible->parent );

		G_SetEntState( constructible, STATE_DEFAULT );

		// make destructable
		if( !(constructible->spawnflags & 2) ) {
			constructible->takedamage = qtrue;
			constructible->health = constructible->sound1to2;
		}

		// Stop thinking
		constructible->think = NULL;
		constructible->nextthink = 0;

		if( !constructible->count2 ) {
			// call script
			G_Script_ScriptEvent( constructible, "built", "final" );
		} else {
			if( constructible->grenadeFired == constructible->count2 ) {
				G_Script_ScriptEvent( constructible, "built", "final" );
			} else {
				switch( constructible->grenadeFired ) {
				case 1: G_Script_ScriptEvent( constructible, "built", "stage1" ); break;
				case 2: G_Script_ScriptEvent( constructible, "built", "stage2" ); break;
				case 3: G_Script_ScriptEvent( constructible, "built", "stage3" ); break;
				}
			}
		}

		// Stop sound
		if( constructible->parent->spawnflags & 8 ) {
			constructible->parent->s.loopSound = 0;
		} else {
			constructible->s.loopSound = 0;
		}

		//ent->client->ps.classWeaponTime = level.time; // Out of "ammo"

		// if not invulnerable and dynamite-able, create a 'destructable' marker for the other team
		if( !(constructible->spawnflags & CONSTRUCTIBLE_INVULNERABLE) && (constructible->constructibleStats.weaponclass >= 1) ) {
			if( !constructible->count2 || constructible->grenadeFired == 1 ) {
				gentity_t* tent = NULL;
				gentity_t *e;
				e = G_Spawn();

				e->r.svFlags = SVF_BROADCAST;
				e->classname = "explosive_indicator";
				e->s.pos.trType = TR_STATIONARY;
				e->s.eType = ET_EXPLOSIVE_INDICATOR;

				while((tent = G_Find (tent, FOFS(target), constructible->targetname)) != NULL) {
					if(tent->s.eType == ET_OID_TRIGGER) {
						if(tent->spawnflags & 8) {
							e->s.eType = ET_TANK_INDICATOR;
						}
					}
				}

				// Find the trigger_objective_info that targets us (if not set before)
				{
					gentity_t* tent = NULL;
					while((tent = G_Find (tent, FOFS(target), constructible->targetname)) != NULL) {
						if((tent->s.eType == ET_OID_TRIGGER)) {
							e->parent = tent;
						}
					}
				}

				if ( constructible->spawnflags & AXIS_CONSTRUCTIBLE )
					e->s.teamNum = TEAM_AXIS;
				else if ( constructible->spawnflags & ALLIED_CONSTRUCTIBLE )
					e->s.teamNum = TEAM_ALLIES;

				e->s.modelindex2 = ent->client->touchingTOI->s.teamNum;
				e->r.ownerNum = constructible->s.number;
				e->think = explosive_indicator_think;
				e->nextthink = level.time + FRAMETIME;

				e->s.effect1Time = constructible->constructibleStats.weaponclass;

				if(constructible->parent->tagParent) {
					e->tagParent = constructible->parent->tagParent;
					Q_strncpyz( e->tagName, constructible->parent->tagName, MAX_QPATH );
				} else {
					VectorCopy( constructible->r.absmin, e->s.pos.trBase );
					VectorAdd( constructible->r.absmax, e->s.pos.trBase, e->s.pos.trBase );
					VectorScale( e->s.pos.trBase, 0.5, e->s.pos.trBase );
				}

				SnapVector( e->s.pos.trBase );

				trap_LinkEntity( e );
			} else {
				// find our marker and update it's coordinates
				for( i = 0, check = g_entities; i < level.num_entities; i++, check++) {
					if( check->s.eType != ET_EXPLOSIVE_INDICATOR && check->s.eType != ET_TANK_INDICATOR && check->s.eType != ET_TANK_INDICATOR_DEAD )
						continue;

					if( check->r.ownerNum == constructible->s.number ) {
						// found it!
						if(constructible->parent->tagParent) {
							check->tagParent = constructible->parent->tagParent;
							Q_strncpyz( check->tagName, constructible->parent->tagName, MAX_QPATH );
						} else {
							VectorCopy( constructible->r.absmin, check->s.pos.trBase );
							VectorAdd( constructible->r.absmax, check->s.pos.trBase, check->s.pos.trBase );
							VectorScale( check->s.pos.trBase, 0.5, check->s.pos.trBase );

							SnapVector( check->s.pos.trBase );
						}

						trap_LinkEntity( check );
						break;
					}
				}
			}
		}

		// Gordon: are we scripted only?
		if( !(ent->spawnflags & CONSTRUCTIBLE_AAS_SCRIPTED) ) {
			if ( !(ent->spawnflags & CONSTRUCTIBLE_NO_AAS_BLOCKING) ) {
				// RF, a stage has been completed, either enable or disable AAS areas appropriately
				if( !(constructible->spawnflags & CONSTRUCTIBLE_BLOCK_PATHS_WHEN_BUILD) ) {
					// builing creates AAS paths
					// Gordon: HACK from ryan
	//				if( !constructible->count2 || ( constructible->grenadeFired == constructible->count2 ) )
					{
						// completely built, enable paths
						G_SetAASBlockingEntity( constructible, AAS_AREA_ENABLED );
					}
				} else {
					// builing blocks AAS paths
					G_SetAASBlockingEntity( constructible, AAS_AREA_DISABLED );
				}
			}
		}

		return( qtrue );	// building
	}

	return( qfalse );
}

void AutoBuildConstruction( gentity_t* constructible ) {
	int i;
	gentity_t* check;

	HandleEntsThatBlockConstructible( NULL, constructible, qtrue, qfalse );
	if( constructible->count2 ) {
		// backup...
		//int constructibleModelindex = constructible->s.modelindex;
		int constructibleClipmask = constructible->clipmask;
		int constructibleContents = constructible->r.contents;
		int constructibleNonSolidBModel = (constructible->s.eFlags & EF_NONSOLID_BMODEL);

		constructible->s.modelindex2 = 0;
		trap_SetBrushModel( constructible, va( "*%i", constructible->conbmodels[constructible->grenadeFired-1] ) );

		// ...and restore
		constructible->clipmask = constructibleClipmask;
		constructible->r.contents = constructibleContents;
		if( !constructibleNonSolidBModel )
			constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;

		if( constructible->grenadeFired == constructible->count2 ) {
			constructible->s.angles2[1] = 1;
		}
	} else {
		// backup...
		//int constructibleModelindex = constructible->s.modelindex;
		int constructibleClipmask = constructible->clipmask;
		int constructibleContents = constructible->r.contents;
		int constructibleNonSolidBModel = (constructible->s.eFlags & EF_NONSOLID_BMODEL);

		constructible->s.modelindex2 = 0;
		trap_SetBrushModel( constructible, constructible->model );

		// ...and restore
		constructible->clipmask = constructibleClipmask;
		constructible->r.contents = constructibleContents;
		if( !constructibleNonSolidBModel )
			constructible->s.eFlags &= ~EF_NONSOLID_BMODEL;

		constructible->s.angles2[1] = 1;
	}

	// unlink the objective info to get rid of the indicator for now
	// Arnout: don't unlink, we still want the location popup. Instead, constructible_indicator_think got changed to free
	// the indicator when the constructible is constructed
//			if( constructible->parent )
//				trap_UnlinkEntity( constructible->parent );

	G_SetEntState( constructible, STATE_DEFAULT );

	// make destructable
	if( !(constructible->spawnflags & CONSTRUCTIBLE_INVULNERABLE) ) {
		constructible->takedamage = qtrue;
		constructible->health = constructible->constructibleStats.health;
	}

	// Stop thinking
	constructible->think = NULL;
	constructible->nextthink = 0;

	if( !constructible->count2 ) {
		// call script
		G_Script_ScriptEvent( constructible, "built", "final" );
	} else {
		if( constructible->grenadeFired == constructible->count2 ) {
			G_Script_ScriptEvent( constructible, "built", "final" );
		} else {
			switch( constructible->grenadeFired ) {
			case 1: G_Script_ScriptEvent( constructible, "built", "stage1" ); break;
			case 2: G_Script_ScriptEvent( constructible, "built", "stage2" ); break;
			case 3: G_Script_ScriptEvent( constructible, "built", "stage3" ); break;
			}
		}
	}

	// Stop sound
	if( constructible->parent->spawnflags & 8 ) {
		constructible->parent->s.loopSound = 0;
	} else {
		constructible->s.loopSound = 0;
	}

	//ent->client->ps.classWeaponTime = level.time; // Out of "ammo"

	// if not invulnerable and dynamite-able, create a 'destructable' marker for the other team
	if( !(constructible->spawnflags & CONSTRUCTIBLE_INVULNERABLE) && (constructible->constructibleStats.weaponclass >= 1) ) {
		if( !constructible->count2 || constructible->grenadeFired == 1 ) {
			gentity_t* tent = NULL;
			gentity_t *e;
			e = G_Spawn();

			e->r.svFlags = SVF_BROADCAST;
			e->classname = "explosive_indicator";
			e->s.pos.trType = TR_STATIONARY;
			e->s.eType = ET_EXPLOSIVE_INDICATOR;

			while((tent = G_Find(tent, FOFS(target), constructible->targetname)) != NULL) {
				if((tent->s.eType == ET_OID_TRIGGER)) {
					if(tent->spawnflags & 8) {
						e->s.eType = ET_TANK_INDICATOR;
					}
				}
			}

			// Find the trigger_objective_info that targets us (if not set before)
			{
				gentity_t* tent = NULL;
				while((tent = G_Find (tent, FOFS(target), constructible->targetname)) != NULL) {
					if((tent->s.eType == ET_OID_TRIGGER)) {
						e->parent = tent;
					}
				}
			}

			if ( constructible->spawnflags & AXIS_CONSTRUCTIBLE )
				e->s.teamNum = TEAM_AXIS;
			else if ( constructible->spawnflags & ALLIED_CONSTRUCTIBLE )
				e->s.teamNum = TEAM_ALLIES;

			e->s.modelindex2 = constructible->parent->s.teamNum == TEAM_AXIS ? TEAM_ALLIES : TEAM_AXIS;
			e->r.ownerNum = constructible->s.number;
			e->think = explosive_indicator_think;
			e->nextthink = level.time + FRAMETIME;

			e->s.effect1Time = constructible->constructibleStats.weaponclass;

			if(constructible->parent->tagParent) {
				e->tagParent = constructible->parent->tagParent;
				Q_strncpyz( e->tagName, constructible->parent->tagName, MAX_QPATH );
			} else {
				VectorCopy( constructible->r.absmin, e->s.pos.trBase );
				VectorAdd( constructible->r.absmax, e->s.pos.trBase, e->s.pos.trBase );
				VectorScale( e->s.pos.trBase, 0.5, e->s.pos.trBase );
			}

			SnapVector( e->s.pos.trBase );

			trap_LinkEntity( e );
		} else {
			// find our marker and update it's coordinates
			for( i = 0, check = g_entities; i < level.num_entities; i++, check++) {
				if( check->s.eType != ET_EXPLOSIVE_INDICATOR && check->s.eType != ET_TANK_INDICATOR && check->s.eType != ET_TANK_INDICATOR_DEAD )
					continue;

				if( check->r.ownerNum == constructible->s.number ) {
					// found it!
					if(constructible->parent->tagParent) {
						check->tagParent = constructible->parent->tagParent;
						Q_strncpyz( check->tagName, constructible->parent->tagName, MAX_QPATH );
					} else {
						VectorCopy( constructible->r.absmin, check->s.pos.trBase );
						VectorAdd( constructible->r.absmax, check->s.pos.trBase, check->s.pos.trBase );
						VectorScale( check->s.pos.trBase, 0.5, check->s.pos.trBase );

						SnapVector( check->s.pos.trBase );
					}

					trap_LinkEntity( check );
					break;
				}
			}
		}
	}

	// Gordon: are we scripted only?
	if( !(constructible->spawnflags & CONSTRUCTIBLE_AAS_SCRIPTED) ) {
		if ( !(constructible->spawnflags & CONSTRUCTIBLE_NO_AAS_BLOCKING) ) {
			// RF, a stage has been completed, either enable or disable AAS areas appropriately
			if( !(constructible->spawnflags & CONSTRUCTIBLE_BLOCK_PATHS_WHEN_BUILD) ) {
				// builing creates AAS paths
				if( !constructible->count2 || ( constructible->grenadeFired == constructible->count2 ) ) {
					// completely built, enable paths
					G_SetAASBlockingEntity( constructible, AAS_AREA_ENABLED );
				}
			} else {
				// builing blocks AAS paths
				G_SetAASBlockingEntity( constructible, AAS_AREA_DISABLED );
			}
		}
	}
}

qboolean G_LandmineTriggered( gentity_t* ent ) {
	switch( ent->s.teamNum ) {
		case TEAM_AXIS + 8:
		case TEAM_ALLIES + 8:
			return qtrue;
	}

	return qfalse;
}

qboolean G_LandmineArmed( gentity_t* ent ) {
	switch( ent->s.teamNum ) {
		case TEAM_AXIS:
		case TEAM_ALLIES:
			return qtrue;
	}
	return qfalse;
}

qboolean G_LandmineUnarmed( gentity_t* ent ) {
	return (qboolean)( !G_LandmineArmed( ent ) && !G_LandmineTriggered( ent ) );
}

team_t G_LandmineTeam( gentity_t* ent ) {
	return (team_t)( ent->s.teamNum % 4 );
}

qboolean G_LandmineSpotted( gentity_t* ent ) {
	return ent->s.modelindex2 ? qtrue : qfalse;
}

void trap_EngineerTrace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	G_TempTraceIgnorePlayersAndBodies();
	trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );
	G_ResetTempTraceIgnoreEnts();
}

// DHM - Nerve
void Weapon_Engineer( gentity_t *ent ) {
	trace_t		tr;
	gentity_t	*traceEnt, *hit;
	vec3_t		mins, maxs; // JPW NERVE
	int			i, num, touch[MAX_GENTITIES], scored = 0; // JPW NERVE
	int			dynamiteDropTeam;
	vec3_t		end;
	vec3_t		origin;

	// DHM - Nerve :: Can't heal an MG42 if you're using one!
	if( ent->client->ps.persistant[PERS_HWEAPON_USE] ) {
		return;
	}

	if( ent->client->touchingTOI ) {
		if( TryConstructing( ent ) ) {
			return;
		}
	}

	AngleVectors (ent->client->ps.viewangles, __forward, __right, __up);
	VectorCopy( ent->client->ps.origin, __muzzleTrace );
	__muzzleTrace[2] += ent->client->ps.viewheight;
	
	VectorMA (__muzzleTrace, 64, __forward, end);			// CH_BREAKABLE_DIST
	trap_EngineerTrace( &tr, __muzzleTrace, NULL, NULL, end, ent->s.number, MASK_SHOT|CONTENTS_TRIGGER );

	if( tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	// no contact
	if( tr.fraction == 1.0f ) {
		return;
	}

	if( tr.entityNum == ENTITYNUM_NONE || tr.entityNum == ENTITYNUM_WORLD ) {
		return;
	}

	traceEnt = &g_entities[ tr.entityNum ];
	if( G_EmplacedGunIsRepairable( traceEnt, ent ) ) {
		// "Ammo" for this weapon is time based
		if ( ent->client->ps.classWeaponTime + level.engineerChargeTime[ent->client->sess.sessionTeam-1] < level.time ) {
			ent->client->ps.classWeaponTime = level.time - level.engineerChargeTime[ent->client->sess.sessionTeam-1];
		}

		if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CHARGE))
			ent->client->ps.classWeaponTime += int( .66f * 150 * SK5G_CHARGE_FACTOR );
		else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 3)
			ent->client->ps.classWeaponTime += int( .66f * 150 );
		else
			ent->client->ps.classWeaponTime += 150;

		if ( ent->client->ps.classWeaponTime > level.time ) {
			ent->client->ps.classWeaponTime = level.time;
			return;		// Out of "ammo"
		}

		if( traceEnt->health >= 255 ) {
			traceEnt->s.frame = 0;

			if ( traceEnt->mg42BaseEnt > 0 ) {
				g_entities[ traceEnt->mg42BaseEnt ].health = MG42_MULTIPLAYER_HEALTH;
				g_entities[ traceEnt->mg42BaseEnt ].takedamage = qtrue;
				traceEnt->health = 0;
			} else {
				traceEnt->health = MG42_MULTIPLAYER_HEALTH;
			}

			G_LogPrintf("Repair: %d\n", ent - g_entities);	// OSP

			if( traceEnt->sound3to2 != ent->client->sess.sessionTeam ) {
				AddScore( ent, WOLF_REPAIR_BONUS ); // JPW NERVE props to the E for the fixin'
				G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 3.f );
				G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 3.f, "repairing a MG42" );
			}

			traceEnt->takedamage = qtrue;
			traceEnt->s.eFlags &= ~EF_SMOKING;

			trap_SendServerCommand(ent-g_entities, "cp \"You have repaired the MG!\n\"");
			G_AddEvent( ent, EV_MG42_FIXED, 0 );
		} else {
			if (cvars::bg_sk5_eng.ivalue & SK5_ENG_CONSTRUCT)
				traceEnt->health += 4;
			else
				traceEnt->health += 3;
		}
	} else {
		trap_EngineerTrace( &tr, __muzzleTrace, NULL, NULL, end, ent->s.number, MASK_SHOT );
		if ( tr.surfaceFlags & SURF_NOIMPACT )
			return;
		if(tr.fraction == 1.0f)
			return;
		if ( tr.entityNum == ENTITYNUM_NONE || tr.entityNum == ENTITYNUM_WORLD )
			return;
		traceEnt = &g_entities[ tr.entityNum ];

		if ( traceEnt->methodOfDeath == MOD_LANDMINE || traceEnt->methodOfDeath == MOD_POISON_GAS ) {
			trace_t tr2;
			vec3_t base;
			vec3_t tr_down = {0, 0, 16};

			VectorSubtract(traceEnt->s.pos.trBase, tr_down, base);

			trap_EngineerTrace( &tr2, traceEnt->s.pos.trBase, NULL, NULL, base, traceEnt->s.number, MASK_SHOT );
			
			// ydnar: added "surfaceparm landmine" (SURF_LANDMINE) support
			//%	if(!(tr2.surfaceFlags & (SURF_GRASS | SURF_SNOW | SURF_GRAVEL)) || 
			if( !(tr2.surfaceFlags & SURF_LANDMINE) || (tr2.entityNum != ENTITYNUM_WORLD && (!g_entities[tr2.entityNum].inuse || g_entities[tr2.entityNum].s.eType != ET_CONSTRUCTIBLE))) {
				trap_SendServerCommand(ent-g_entities, "cp \"Landmine cannot be armed here...\" 1");

				G_FreeEntity( traceEnt );

				Add_Ammo(ent, traceEnt->s.weapon, 1, qfalse);

				// rain - #202 - give back the correct charge amount
				if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CHARGE))
					ent->client->ps.classWeaponTime -= int( .33f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
				else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 3)
					ent->client->ps.classWeaponTime -= int( .33f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );
				else
					ent->client->ps.classWeaponTime -= int( .5f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );

				ent->client->sess.aWeaponStats[WS_LANDMINE].atts--;
				return;
//bani
// rain - #384 - check landmine team so that enemy mines can be disarmed
// even if you're using all of yours :x
			} else if ( G_CountTeamLandmines(ent->client->sess.sessionTeam) >= MAX_TEAM_LANDMINES && G_LandmineTeam(traceEnt) == ent->client->sess.sessionTeam) {

				if(G_LandmineUnarmed(traceEnt)) {
// rain - should be impossible now
//					if ( G_LandmineTeam( traceEnt ) != ent->client->sess.sessionTeam )
//						return;

					trap_SendServerCommand(ent-g_entities, "cp \"Your team has too many landmines placed...\" 1");

					G_FreeEntity( traceEnt );

					Add_Ammo(ent, traceEnt->s.weapon, 1, qfalse);

					// rain - #202 - give back the correct charge amount
					if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CHARGE))
						ent->client->ps.classWeaponTime -= int( .33f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
					else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 3)
						ent->client->ps.classWeaponTime -= int( .33f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );
					else
						ent->client->ps.classWeaponTime -= int( .5f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );

					ent->client->sess.aWeaponStats[WS_LANDMINE].atts--;
					return;
				}
//bani - #471
				else {
					goto evilbanigoto;
				}
			} else {

				if(G_LandmineUnarmed(traceEnt)) {
					// Opposing team cannot accidentally arm it
					if( G_LandmineTeam(traceEnt) != ent->client->sess.sessionTeam )
						return;

					G_PrintClientSpammyCenterPrint(ent-g_entities, "Arming landmine...");

					// Give health until it is full, don't continue
					if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CONSTRUCT))
						traceEnt->health += 30;
					else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 2)
						traceEnt->health += 24;
					else
						traceEnt->health += 12;

					if ( traceEnt->health >= 250 ) {
						//traceEnt->health = 255;
						trap_SendServerCommand(ent-g_entities, "cp \"Landmine armed...\" 1");
					} else {
						return;
					}

					traceEnt->landmineSkillWhenPlanted = ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION];
					traceEnt->r.contents = 0;	// (player can walk through)
					trap_LinkEntity( traceEnt );

					// forty - mine id
					traceEnt->s.otherEntityNum = ent->s.number;

                    // Jaybird - #14 - change owner of the mine
                    traceEnt->parent = ent;

					// Don't allow disarming for sec (so guy that WAS arming doesn't start disarming it!
					traceEnt->timestamp = level.time + 1000;
					traceEnt->health = 0;

					// Jaybird - for fading effect
					traceEnt->s.effect1Time = level.time;

					traceEnt->s.teamNum = ent->client->sess.sessionTeam;
					traceEnt->s.modelindex2 = 0;

					traceEnt->nextthink = level.time + 2000;
					traceEnt->think = G_LandminePrime;
				} else {
//bani - #471
evilbanigoto:
					if (traceEnt->timestamp > level.time)
						return;
					if (traceEnt->health >= 250) // have to do this so we don't score multiple times
						return;

					if (traceEnt->landmineSkillWhenPlanted >= 5 && cvars::bg_sk5_eng.ivalue & SK5_ENG_MINE_DEFUSE) {
						if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CONSTRUCT))
							traceEnt->health += 6;
						else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 2)
							traceEnt->health += 4;
						else
							traceEnt->health += 2;
					}
					else {
						if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CONSTRUCT))
							traceEnt->health += 9;
						else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 2)
							traceEnt->health += 6;
						else
							traceEnt->health += 3;
					}

					G_PrintClientSpammyCenterPrint(ent-g_entities, "Defusing landmine");

					if ( traceEnt->health >= 250 ) {
/*						traceEnt->health = 255;
						traceEnt->think = G_FreeEntity;
						traceEnt->nextthink = level.time + FRAMETIME;*/

						trap_SendServerCommand(ent-g_entities, "cp \"Landmine defused...\" 1");

						Add_Ammo(ent, traceEnt->s.weapon, 1, qfalse);

						if( G_LandmineTeam( traceEnt ) != ent->client->sess.sessionTeam ) {
							G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 4.f );
							G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 4.f, "defusing an enemy landmine" );
						}

						// update our map
						/*{
							// if it's an enemy mine, update both teamlists
							int teamNum;
							mapEntityData_t	*mEnt;
							mapEntityData_Team_t *teamList;

							teamNum = traceEnt->s.teamNum % 4;

							teamList = ent->client->sess.sessionTeam == TEAM_AXIS ? &mapEntityData[0] : &mapEntityData[1];
							if((mEnt = G_FindMapEntityData(teamList, traceEnt-g_entities)) != NULL) {
								G_FreeMapEntityData( teamList, mEnt );
							}

							teamList = ent->client->sess.sessionTeam == TEAM_AXIS ? &mapEntityData[1] : &mapEntityData[0];	// inverted
							if((mEnt = G_FindMapEntityData(teamList, traceEnt-g_entities)) != NULL) {
								if( teamNum != ent->client->sess.sessionTeam ) {
									G_FreeMapEntityData( teamList, mEnt );
								} else {
								//	mEnt->type = ME_LANDMINE;	// set it back to this as it might have been set to 'about to explode'.
									mEnt->entNum = -1;
								}
							}
						}*/
						{
							mapEntityData_t	*mEnt;

							if((mEnt = G_FindMapEntityData(&mapEntityData[0], traceEnt-g_entities)) != NULL) {
								G_FreeMapEntityData( &mapEntityData[0], mEnt );
							}

							if((mEnt = G_FindMapEntityData(&mapEntityData[1], traceEnt-g_entities)) != NULL) {
								G_FreeMapEntityData( &mapEntityData[1], mEnt );
							}

							G_FreeEntity( traceEnt );
						}
					} else {
						return;
					}
				}	
			}
		} else if ( traceEnt->methodOfDeath == MOD_SATCHEL ) {
			if( traceEnt->health >= 250 ) // have to do this so we don't score multiple times
				return;

			// Give health until it is full, don't continue
			traceEnt->health += 3;

			G_PrintClientSpammyCenterPrint(ent-g_entities, "Disarming satchel charge...");

			if ( traceEnt->health >= 250 ) {

				traceEnt->health = 255;
				traceEnt->think = G_FreeEntity;
				traceEnt->nextthink = level.time + FRAMETIME;

				//bani - consistency with dynamite defusing
				G_PrintClientSpammyCenterPrint(ent-g_entities, "Satchel charge disarmed...");

				G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f );
				G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f, "disarming satchel charge" );
			} else {
				return;
			}
//bani - no tripmine...
#if 0
		} else if ( traceEnt->methodOfDeath == MOD_TRIPMINE ) {
			// Give health until it is full, don't continue
			traceEnt->health += 3;

			G_PrintClientSpammyCenterPrint(ent-g_entities, "Disarming tripmine...");

			if ( traceEnt->health >= 250 ) {
				traceEnt->health = 255;
				traceEnt->think = G_FreeEntity;
				traceEnt->nextthink = level.time + FRAMETIME;

				Add_Ammo(ent, WP_TRIPMINE, 1, qfalse);
			} else {
				return;
			}
#endif
		} else
		if ( traceEnt->methodOfDeath == MOD_DYNAMITE ) {

			// Not armed
			if ( traceEnt->s.teamNum >= 4 ) {
				//bani
				qboolean friendlyObj = qfalse;
				qboolean enemyObj = qfalse;
				int dynamiteTime = 30000;

				// Opposing team cannot accidentally arm it
				if ( (traceEnt->s.teamNum - 4) != ent->client->sess.sessionTeam )
					return;

				G_PrintClientSpammyCenterPrint(ent-g_entities, "Arming dynamite...");

				// Give health until it is full, don't continue
				if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CONSTRUCT))
					traceEnt->health += 21;
				else if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 2)
					traceEnt->health += 14;
				else
					traceEnt->health += 7;

				{
					int		entityList[MAX_GENTITIES];
					int		numListedEntities;
					int		e;
					vec3_t  org;

					VectorCopy( traceEnt->r.currentOrigin, org );
					org[2] += 4;	// move out of ground

					G_TempTraceIgnorePlayersAndBodies();
					numListedEntities = EntsThatRadiusCanDamage( org, traceEnt->splashRadius, entityList );
					G_ResetTempTraceIgnoreEnts();

					for( e = 0; e < numListedEntities; e++ ) {
						hit = &g_entities[entityList[ e ]];

						if( hit->s.eType != ET_CONSTRUCTIBLE ) {
							continue;
						}

						// invulnerable
						if( hit->spawnflags & CONSTRUCTIBLE_INVULNERABLE || (hit->parent && hit->parent->spawnflags & 8) ) {
							continue;
						}

						if( !G_ConstructionIsPartlyBuilt( hit ) ) {
							continue;
						}

						// is it a friendly constructible
						if( hit->s.teamNum == traceEnt->s.teamNum - 4 ) {
//bani
//							G_FreeEntity( traceEnt );
//							trap_SendServerCommand( ent-g_entities, "cp \"You cannot arm dynamite near a friendly construction!\" 1");
//							return;
							friendlyObj = qtrue;
						}
					}
				}

				VectorCopy( traceEnt->r.currentOrigin, origin );
				SnapVector( origin );
				VectorAdd( origin, traceEnt->r.mins, mins );
				VectorAdd( origin, traceEnt->r.maxs, maxs );
				num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
				VectorAdd( origin, traceEnt->r.mins, mins );
				VectorAdd( origin, traceEnt->r.maxs, maxs );

				for ( i=0 ; i<num ; i++ ) {
					hit = &g_entities[touch[i]];

					if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
						continue;
					}

					if ((hit->s.eType == ET_OID_TRIGGER)) {
						if ( !(hit->spawnflags & (AXIS_OBJECTIVE|ALLIED_OBJECTIVE)) )
							continue;

						// Arnout - only if it targets a func_explosive
						if( hit->target_ent && Q_stricmp( hit->target_ent->classname, "func_explosive" ) )
							continue;

						if ( ((hit->spawnflags & AXIS_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_AXIS)) || 
							 ((hit->spawnflags & ALLIED_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_ALLIES)) ) {
//bani
//							G_FreeEntity( traceEnt );
//							trap_SendServerCommand( ent-g_entities, "cp \"You cannot arm dynamite near a friendly objective!\" 1");
//							return;
							friendlyObj = qtrue;
						}

						//bani
						if ( ((hit->spawnflags & AXIS_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_ALLIES)) ||
							 ((hit->spawnflags & ALLIED_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_AXIS)) ) {
							enemyObj = qtrue;
						}
					}
				}

				//bani
				if( friendlyObj && !enemyObj ) {
					G_FreeEntity( traceEnt );
					trap_SendServerCommand( ent-g_entities, "cp \"You cannot arm dynamite near a friendly objective!\" 1");
					return;
				}

				if ( traceEnt->health >= 250 ) {
					traceEnt->health = 255;
				} else {
					return;
				}

				// Don't allow disarming for sec (so guy that WAS arming doesn't start disarming it!
				traceEnt->timestamp = level.time + 1000;
				traceEnt->health = 5;

				// set teamnum so we can check it for drop/defuse exploit
				traceEnt->s.teamNum = ent->client->sess.sessionTeam;
				// For dynamic light pulsing
				traceEnt->s.effect1Time = level.time;

				// ARM IT!
				// Jaybird - Allow custom arm time.
				if (cvars::bg_dynamiteTime.ivalue > 0)
					dynamiteTime = cvars::bg_dynamiteTime.ivalue * 1000;
				traceEnt->nextthink = level.time + dynamiteTime;
				traceEnt->think = G_ExplodeMissile;

				// Gordon: moved down here to prevent two prints when dynamite IS near objective

				trap_SendServerCommand( ent-g_entities, va("cp \"Dynamite is now armed with a %i second timer!\" 1",(dynamiteTime / 1000)));

				// check if player is in trigger objective field
				// NERVE - SMF - made this the actual bounding box of dynamite instead of range, also must snap origin to line up properly
				VectorCopy( traceEnt->r.currentOrigin, origin );
				SnapVector( origin );
				VectorAdd( origin, traceEnt->r.mins, mins );
				VectorAdd( origin, traceEnt->r.maxs, maxs );
				num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

				for ( i=0 ; i<num ; i++ ) {
					hit = &g_entities[touch[i]];

					if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
						continue;
					}
					if ((hit->s.eType == ET_OID_TRIGGER)) {

						if ( !(hit->spawnflags & (AXIS_OBJECTIVE|ALLIED_OBJECTIVE)) )
							continue;

						// Arnout - only if it targets a func_explosive
						if( hit->target_ent && Q_stricmp( hit->target_ent->classname, "func_explosive" ) )
							continue;

						if ( hit->spawnflags & AXIS_OBJECTIVE ) {
							if (ent->client->sess.sessionTeam == TEAM_ALLIES) { // transfer score info if this is a bomb scoring objective
								traceEnt->accuracy = hit->accuracy;
							}
						} else if (hit->spawnflags & ALLIED_OBJECTIVE) {
							if (ent->client->sess.sessionTeam == TEAM_AXIS) { // ditto other team
								traceEnt->accuracy = hit->accuracy;
							}
						}

						// rain - spawnflags 128 = disabled (#309)
						if (!(hit->spawnflags & 128) && (((hit->spawnflags & AXIS_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_ALLIES)) ||
							 ((hit->spawnflags & ALLIED_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_AXIS))) ) 
						{
							gentity_t* pm = G_PopupMessage( PM_DYNAMITE );
							const char *Goalname = _GetEntityName( hit );
							pm->s.effect2Time = 0;
							pm->s.effect3Time = hit->s.teamNum;
							pm->s.teamNum = ent->client->sess.sessionTeam;

							G_Script_ScriptEvent( hit, "dynamited", "" );

							// notify omni-bot framework of planted dynamite
							hit->numPlanted += 1;							
							Bot_AddDynamiteGoal(traceEnt, traceEnt->s.teamNum, va("%s_%i", Goalname, hit->numPlanted));

							if ( !(hit->spawnflags & OBJECTIVE_DESTROYED) ) {
								AddScore(traceEnt->parent, WOLF_DYNAMITE_PLANT); // give drop score to guy who dropped it
								if(traceEnt->parent && traceEnt->parent->client) {
									G_LogPrintf("Dynamite_Plant: %d\n", traceEnt->parent - g_entities);	// OSP
								}
								traceEnt->parent = ent; // give explode score to guy who armed it
							}
							//bani - fix #238
							traceEnt->etpro_misc_1 |= 1;
						}
//bani
//						i = num;
						return;	//bani - bail out here because primary obj's take precendence over constructibles
					}		
				}

//bani - reordered this check so its AFTER the primary obj check
				// Arnout - first see if the dynamite is planted near a constructable object that can be destroyed
				{
					int		entityList[MAX_GENTITIES];
					int		numListedEntities;
					int		e;
					vec3_t  org;

					VectorCopy( traceEnt->r.currentOrigin, org );
					org[2] += 4;	// move out of ground

					G_TempTraceIgnorePlayersAndBodies();
					numListedEntities = EntsThatRadiusCanDamage( org, traceEnt->splashRadius, entityList );
					G_ResetTempTraceIgnoreEnts();

					for( e = 0; e < numListedEntities; e++ ) {
						hit = &g_entities[entityList[ e ]];

						if( hit->s.eType != ET_CONSTRUCTIBLE )
							continue;

						// invulnerable
						if( hit->spawnflags & CONSTRUCTIBLE_INVULNERABLE )
							continue;

						if( !G_ConstructionIsPartlyBuilt( hit ) ) {
							continue;
						}

						// is it a friendly constructible
						if( hit->s.teamNum == traceEnt->s.teamNum ) {
//bani - er, didnt we just pass this check earlier?
//							G_FreeEntity( traceEnt );
//							trap_SendServerCommand( ent-g_entities, "cp \"You cannot arm dynamite near a friendly construction!\" 1");
//							return;
							continue;
						}

						// not dynamite-able
						if( hit->constructibleStats.weaponclass < 1 ) {
							continue;
						}

						if( hit->parent )
						{
							const char *Goalname = _GetEntityName( hit->parent );
							gentity_t* pm = G_PopupMessage( PM_DYNAMITE );
							pm->s.effect2Time = 0; // 0 = planted
							pm->s.effect3Time = hit->parent->s.teamNum;
							pm->s.teamNum = ent->client->sess.sessionTeam;

							G_Script_ScriptEvent( hit, "dynamited", "" );

							// notify omni-bot framework of planted dynamite
							hit->numPlanted += 1;
							Bot_AddDynamiteGoal(traceEnt, traceEnt->s.teamNum, va("%s_%i", Goalname, hit->numPlanted));

							if( (!(hit->parent->spawnflags & OBJECTIVE_DESTROYED)) && 
								hit->s.teamNum && (hit->s.teamNum == ent->client->sess.sessionTeam) ) {	// ==, as it's inverse
								AddScore(traceEnt->parent, WOLF_DYNAMITE_PLANT); // give drop score to guy who dropped it
								if( traceEnt->parent && traceEnt->parent->client ) {
									G_LogPrintf("Dynamite_Plant: %d\n", traceEnt->parent - g_entities);	// OSP
								}
								traceEnt->parent = ent; // give explode score to guy who armed it
							}
							//bani - fix #238
							traceEnt->etpro_misc_1 |= 1;
						}
						return;
					}
				}
			} else {
				if (traceEnt->timestamp > level.time)
					return;
				if (traceEnt->health >= 248) // have to do this so we don't score multiple times
					return;
				dynamiteDropTeam = traceEnt->s.teamNum; // set this here since we wack traceent later but want teamnum for scoring
				
				// Jaybird - disallow disarming of dynamite by enemy objectives
				// g_engineers flag 2
				if ((g_engineers.integer & ENGI_FRIENDLYDYNO) && dynamiteDropTeam == ent->client->sess.sessionTeam && traceEnt->parent != ent) {
					qboolean enemyObj = qfalse;
					int		entityList[MAX_GENTITIES];
					int		numListedEntities;
					int		e;
					vec3_t  org;

					VectorCopy( traceEnt->r.currentOrigin, org );
					org[2] += 4;	// move out of ground

					G_TempTraceIgnorePlayersAndBodies();
					numListedEntities = EntsThatRadiusCanDamage( org, traceEnt->splashRadius, entityList );
					G_ResetTempTraceIgnoreEnts();

					// Jaybird - first check for constructibles
					for( e = 0; e < numListedEntities; e++ ) {
						hit = &g_entities[entityList[ e ]];

						if( hit->s.eType != ET_CONSTRUCTIBLE ) {
							continue;
						}

						// invulnerable
						if( hit->spawnflags & CONSTRUCTIBLE_INVULNERABLE || (hit->parent && hit->parent->spawnflags & 8) ) {
							continue;
						}

						if( !G_ConstructionIsPartlyBuilt( hit ) ) {
							continue;
						}

						// is it a friendly constructible
						if( hit->s.teamNum != traceEnt->s.teamNum ) {
							enemyObj = qtrue;
						}
					}

					VectorCopy( traceEnt->r.currentOrigin, origin );
					SnapVector( origin );
					VectorAdd( origin, traceEnt->r.mins, mins );
					VectorAdd( origin, traceEnt->r.maxs, maxs );
					num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );
					VectorAdd( origin, traceEnt->r.mins, mins );
					VectorAdd( origin, traceEnt->r.maxs, maxs );

					// Now check for objectives
					for ( i=0 ; i<num ; i++ ) {
						hit = &g_entities[touch[i]];

						if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
							continue;
						}

						if ((hit->s.eType == ET_OID_TRIGGER)) {
							if ( !(hit->spawnflags & (AXIS_OBJECTIVE|ALLIED_OBJECTIVE)) )
								continue;

							// Arnout - only if it targets a func_explosive
							if( hit->target_ent && Q_stricmp( hit->target_ent->classname, "func_explosive" ) )
								continue;

							//bani
							if ( ((hit->spawnflags & AXIS_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_ALLIES)) ||
								 ((hit->spawnflags & ALLIED_OBJECTIVE) && (ent->client->sess.sessionTeam == TEAM_AXIS)) ) {
								enemyObj = qtrue;
							}
						}
					}

					// Jaybird - if we found one, bail outl.
					if( enemyObj ) {
						trap_SendServerCommand( ent-g_entities, "cp \"You cannot disarm a friendly dynamite near an objective!\" 1");
						return;
					}
				}

				if( ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 2 )
					traceEnt->health += 6;
				else
					traceEnt->health += 3;

				G_PrintClientSpammyCenterPrint(ent-g_entities, "Defusing dynamite...");

				if ( traceEnt->health >= 248 ) {
//bani
					qboolean defusedObj = qfalse;

					traceEnt->health = 255;
					// Need some kind of event/announcement here

//					Add_Ammo( ent, WP_DYNAMITE, 1, qtrue );

					traceEnt->think = G_FreeEntity;
					traceEnt->nextthink = level.time + FRAMETIME;

					VectorCopy( traceEnt->r.currentOrigin, origin );
					SnapVector( origin );
					VectorAdd( origin, traceEnt->r.mins, mins );
					VectorAdd( origin, traceEnt->r.maxs, maxs );
					num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

					// don't report if not disarming *enemy* dynamite in field
/*					if (dynamiteDropTeam == ent->client->sess.sessionTeam)
						return;*/

					//bani - eh, why was this commented out? it makes sense, and prevents a sploit.
					if (dynamiteDropTeam == ent->client->sess.sessionTeam)
						return;

					for ( i=0 ; i<num ; i++ ) {
						hit = &g_entities[touch[i]];

						if ( !( hit->r.contents & CONTENTS_TRIGGER ) ) {
							continue;
						}
						if ((hit->s.eType == ET_OID_TRIGGER)) {

							if ( !(hit->spawnflags & (AXIS_OBJECTIVE | ALLIED_OBJECTIVE)) )
								continue;

							// rain - spawnflags 128 = disabled (#309)
							if (hit->spawnflags & 128)
								continue;

							//bani - prevent plant/defuse exploit near a/h cabinets or non-destroyable locations (bank doors on goldrush)
							if( !hit->target_ent || hit->target_ent->s.eType != ET_EXPLOSIVE ) {
								continue;
							}

							if (ent->client->sess.sessionTeam == TEAM_AXIS) {
								if ((hit->spawnflags & AXIS_OBJECTIVE) && (!scored)) {
									AddScore(ent,WOLF_DYNAMITE_DIFFUSE);
									G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f );
									G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f, "defusing enemy dynamite" );
									scored++;
								}
								if(hit->target_ent) {
									G_Script_ScriptEvent( hit->target_ent, "defused", "" );
								}

								{
									gentity_t* pm = G_PopupMessage( PM_DYNAMITE );
									pm->s.effect2Time = 1; // 1 = defused
									pm->s.effect3Time = hit->s.teamNum;
									pm->s.teamNum = ent->client->sess.sessionTeam;
								}

//								trap_SendServerCommand(-1, "cp \"Axis engineer disarmed the Dynamite!\n\"");
								//bani
								defusedObj = qtrue;
							} else { // TEAM_ALLIES
								if ((hit->spawnflags & ALLIED_OBJECTIVE) && (!scored)) {
									AddScore(ent,WOLF_DYNAMITE_DIFFUSE);
									G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f );
									G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f, "defusing enemy dynamite" );
									scored++; 
									hit->spawnflags &= ~OBJECTIVE_DESTROYED; // "re-activate" objective since it wasn't destroyed
								}
								if(hit->target_ent) {
									G_Script_ScriptEvent( hit->target_ent, "defused", "" );
								}

								{
									gentity_t* pm = G_PopupMessage( PM_DYNAMITE );
									pm->s.effect2Time = 1; // 1 = defused
									pm->s.effect3Time = hit->s.teamNum;
									pm->s.teamNum = ent->client->sess.sessionTeam;
								}

//								trap_SendServerCommand(-1, "cp \"Allied engineer disarmed the Dynamite!\n\"");
								//bani
								defusedObj = qtrue;
							}
						}
					}
//bani - prevent multiple messages here
					if( defusedObj )
						return;

//bani - reordered this check so its AFTER the primary obj check
					// Gordon - first see if the dynamite was planted near a constructable object that would have been destroyed
					{
						int		entityList[MAX_GENTITIES];
						int		numListedEntities;
						int		e;
						vec3_t  org;

						VectorCopy( traceEnt->r.currentOrigin, org );
						org[2] += 4;	// move out of ground

						numListedEntities = EntsThatRadiusCanDamage( org, traceEnt->splashRadius, entityList );

						for( e = 0; e < numListedEntities; e++ ) {
							hit = &g_entities[entityList[ e ]];

							if( hit->s.eType != ET_CONSTRUCTIBLE )
								continue;

							// not completely build yet - NOTE: don't do this, in case someone places dynamite before construction is complete
							//if( hit->s.angles2[0] < 255 )
							//	continue;

							// invulnerable
							if( hit->spawnflags & CONSTRUCTIBLE_INVULNERABLE )
								continue;

							// not dynamite-able
							if( hit->constructibleStats.weaponclass < 1 ) {
								continue;
							}

							// we got somthing to destroy
							if (ent->client->sess.sessionTeam == TEAM_AXIS) {
								if ( hit->s.teamNum == TEAM_AXIS && (!scored)) {
									AddScore(ent,WOLF_DYNAMITE_DIFFUSE);
									if(ent && ent->client) G_LogPrintf("Dynamite_Diffuse: %d\n", ent - g_entities);	// OSP
									G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f );
									G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f, "defusing enemy dynamite" );
									scored++;
								}
								G_Script_ScriptEvent( hit, "defused", "" );

								{
									gentity_t* pm = G_PopupMessage( PM_DYNAMITE );
									pm->s.effect2Time = 1; // 1 = defused
									pm->s.effect3Time = hit->parent->s.teamNum;
									pm->s.teamNum = ent->client->sess.sessionTeam;
								}

//								trap_SendServerCommand(-1, "cp \"Axis engineer disarmed the Dynamite!\" 2");
							} else { // TEAM_ALLIES
								if ( hit->s.teamNum == TEAM_ALLIES && (!scored)) {
									AddScore(ent,WOLF_DYNAMITE_DIFFUSE);
									if(ent && ent->client) G_LogPrintf("Dynamite_Diffuse: %d\n", ent - g_entities);	// OSP
									G_AddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f );
									G_DebugAddSkillPoints( ent, SK_EXPLOSIVES_AND_CONSTRUCTION, 6.f, "defusing enemy dynamite" );
									scored++; 
								}
								G_Script_ScriptEvent( hit, "defused", "" );

								{
									gentity_t* pm = G_PopupMessage( PM_DYNAMITE );
									pm->s.effect2Time = 1; // 1 = defused
									pm->s.effect3Time = hit->parent->s.teamNum;
									pm->s.teamNum = ent->client->sess.sessionTeam;
								}

//								trap_SendServerCommand(-1, "cp \"Allied engineer disarmed the Dynamite!\" 2");
							}

							return;
						}
					}
				}
	// jpw
			}
		}
	}
}


// JPW NERVE -- launch airstrike as line of bombs mostly-perpendicular to line of grenade travel
// (close air support should *always* drop parallel to friendly lines, tho accidents do happen)
extern void G_ExplodeMissile( gentity_t *ent );

void G_AirStrikeExplode( gentity_t *self ) {

	self->r.svFlags &= ~SVF_NOCLIENT;
	self->r.svFlags |= SVF_BROADCAST;

	self->think = G_ExplodeMissile;
	self->nextthink = level.time + 50;
}

qboolean G_AvailableAirstrikes( gentity_t* ent ) {
	
	// Jaybird - Check for disabled Airstrikes
	if (G_TeamMaxArtillery() == 0)
		return qfalse;

	if( ent->client->sess.sessionTeam == TEAM_AXIS ) {
		if( level.axisBombCounter >= 60 * 1000 ) {
			return qfalse;
		}
	} else {
		if( level.alliedBombCounter >= 60 * 1000 ) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
========================
G_Update_CS_Airstrikes

Check whether team can launch airstrikes or artillery.
Also updates config string if appropriate.
========================
*/
void G_Update_CS_Airstrikes( void ) {
	char info[MAX_INFO_STRING];
	char newinfo[MAX_INFO_STRING];
	int i;
 
	newinfo[0] = 0;
	info[0] = 0;

	Info_SetValueForKey(newinfo, "axis", "1");
	Info_SetValueForKey(newinfo, "allies", "1");

	Info_SetValueForKey( newinfo, "enabled", va( "%d", G_TeamMaxArtillery() ));
	for (i = TEAM_AXIS; i <= TEAM_ALLIES; i++) {
		if (G_TeamMaxArtillery() == 0)
			Info_SetValueForKey(newinfo, i==TEAM_AXIS?"axis":"allies", "0");
		else if( i == TEAM_AXIS ) {
			if( level.axisBombCounter >= 60 * 1000 ) {
				Info_SetValueForKey(newinfo, "axis", "0");
			}
		} else {
			if( level.alliedBombCounter >= 60 * 1000 ) {
				Info_SetValueForKey(newinfo, "allies", "0");
			}
		}
	}

	trap_GetConfigstring(CS_AVAILABLESTRIKES, info, sizeof(info));
	if (Q_stricmp(info, newinfo)) {
		trap_SetConfigstring( CS_AVAILABLESTRIKES, newinfo );
	}
}

void G_AddAirstrikeToCounters( gentity_t* ent ) {
	const int max = G_TeamMaxArtillery();

	if( ent->client->sess.sessionTeam == TEAM_AXIS ) {
		level.axisBombCounter += ( 60 * 1000 / max );
	} else {
		level.alliedBombCounter += ( 60 * 1000 / max );
	}
}

#define NUMBOMBS 10
#define BOMBSPREAD 150

void weapon_checkAirStrikeThink1( gentity_t *ent ) {
	if( !weapon_checkAirStrike( ent ) ) {
		ent->think = G_ExplodeMissile;
		ent->nextthink = level.time + 1000;
		return;
	}

	ent->think = weapon_callAirStrike;
	ent->nextthink = level.time + 1500;
}

void weapon_checkAirStrikeThink2( gentity_t *ent ) {
	if( !weapon_checkAirStrike( ent ) ) {
		ent->think = G_ExplodeMissile;
		ent->nextthink = level.time + 1000;
		return;
	}

	ent->think = weapon_callSecondPlane;
	ent->nextthink = level.time + 500;
}

void weapon_callSecondPlane( gentity_t *ent ) {
	gentity_t* te;
	
	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
	te->s.eventParm = G_SoundIndex( "sound/weapons/airstrike/airstrike_plane.wav" );
	te->r.svFlags |= SVF_BROADCAST;

	ent->nextthink = level.time + 1000;
	ent->think = weapon_callAirStrike;
}

qboolean weapon_checkAirStrike( gentity_t *ent ) {
	if( ent->s.teamNum == TEAM_AXIS ) {
		level.numActiveAirstrikes[0]++;
	} else {
		level.numActiveAirstrikes[1]++;
	}

	// cancel the airstrike if FF off and player joined spec
	// FIXME: this is a stupid workaround. Just store the parent team in the enitity itself and use that - no need to look up the parent
	if (!g_friendlyFire.integer && ent->parent->client && ent->parent->client->sess.sessionTeam == TEAM_SPECTATOR)
	{
		ent->splashDamage = 0;	// no damage
		ent->think = G_ExplodeMissile;
		ent->nextthink = int( level.time + crandom()*50 );
		
		ent->active = qfalse;
		if( ent->s.teamNum == TEAM_AXIS ) {
			level.numActiveAirstrikes[0]--;
		} else {
			level.numActiveAirstrikes[1]--; 
		}
		return qfalse; // do nothing, don't hurt anyone 
	}

	if( ent->s.teamNum == TEAM_AXIS ) {
		if( level.numActiveAirstrikes[0] > 6 || !G_AvailableAirstrikes( ent->parent ) ) {
			G_SayTo( ent->parent, ent->parent, 2, COLOR_YELLOW, "HQ: ", "All available planes are already en-route.", true );

			G_GlobalClientEvent( EV_AIRSTRIKEMESSAGE, 0, ent->parent-g_entities );

			ent->active = qfalse;
			if( ent->s.teamNum == TEAM_AXIS ) {
				level.numActiveAirstrikes[0]--;
			} else {
				level.numActiveAirstrikes[1]--;
			}
			// g_weapons handling
			if (cvars::bg_weapons.ivalue & SBW_FULLBAR ||
				cvars::bg_weapons.ivalue & SBW_HALFBAR) {
				float factor = 0;
				if (cvars::bg_weapons.ivalue & SBW_FULLBAR)
					factor = 1;
				else
					factor = 0.5f;
				if (ent->parent->client->sess.skill[SK_SIGNALS] >= 2) {
					ent->parent->client->ps.classWeaponTime -= int( factor * 0.66f * level.lieutenantChargeTime[ent->parent->client->sess.sessionTeam-1] );
				}
				else {
					ent->parent->client->ps.classWeaponTime -= int( factor * level.lieutenantChargeTime[ent->parent->client->sess.sessionTeam-1] );
				}
			}
			return qfalse;
		}
	} else {
		if( level.numActiveAirstrikes[1] > 6 || !G_AvailableAirstrikes( ent->parent ) ) {
			G_SayTo( ent->parent, ent->parent, 2, COLOR_YELLOW, "HQ: ", "All available planes are already en-route.", true );

			G_GlobalClientEvent( EV_AIRSTRIKEMESSAGE, 0, ent->parent-g_entities );

			ent->active = qfalse;
			if( ent->s.teamNum == TEAM_AXIS ) {
				level.numActiveAirstrikes[0]--;
			} else {
				level.numActiveAirstrikes[1]--;
			}
			// g_weapons handling
			if (cvars::bg_weapons.ivalue & SBW_FULLBAR ||
				cvars::bg_weapons.ivalue & SBW_HALFBAR) {
				float factor = 0;
				if (cvars::bg_weapons.ivalue & SBW_FULLBAR)
					factor = 1;
				else
					factor = 0.5f;
				if (ent->parent->client->sess.skill[SK_SIGNALS] >= 2) {
					ent->parent->client->ps.classWeaponTime -= int( factor * 0.66f * level.lieutenantChargeTime[ent->parent->client->sess.sessionTeam-1] );
				}
				else {
					ent->parent->client->ps.classWeaponTime -= int( factor * level.lieutenantChargeTime[ent->parent->client->sess.sessionTeam-1] );
				}
			}
			return qfalse;
		}
	}

	return qtrue;
}

void G_RailTrail( vec_t* start, vec_t* end );


void weapon_callAirStrike( gentity_t *ent ) {
	int i, j;
	vec3_t bombaxis, lookaxis, pos, bomboffset, fallaxis, temp, dir, skypoint;
	gentity_t *bomb;
	trace_t	tr;
	float traceheight, bottomtraceheight;

	VectorCopy( ent->s.pos.trBase,bomboffset );
	bomboffset[2] += 4096.f;

	// turn off smoke grenade
	ent->think = G_ExplodeMissile;
	ent->nextthink = int( level.time + 950 + NUMBOMBS*100 + crandom()*50 ); // 950 offset is for aircraft flyby

	ent->active = qtrue;

	G_AddAirstrikeToCounters( ent->parent );

	{
		gentity_t* te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		te->s.eventParm = G_SoundIndex( "sound/weapons/airstrike/airstrike_plane.wav");
		te->r.svFlags |= SVF_BROADCAST;
	}

	trap_Trace( &tr, ent->s.pos.trBase, NULL, NULL, bomboffset, ent->s.number, MASK_SHOT );
	if ((tr.fraction < 1.0) && (!(tr.surfaceFlags & SURF_NOIMPACT)) ) { //SURF_SKY)) ) { // JPW NERVE changed for trenchtoast foggie prollem
		G_SayTo( ent->parent, ent->parent, 2, COLOR_YELLOW, "Pilot: ", "Aborting, can't see target.", true );

		G_GlobalClientEvent( EV_AIRSTRIKEMESSAGE, 1, ent->parent-g_entities );

		if( ent->s.teamNum == TEAM_AXIS ) {
			level.numActiveAirstrikes[0]--;
		} else {
			level.numActiveAirstrikes[1]--;
		}
		ent->active = qfalse;
		return;
	}

	G_GlobalClientEvent( EV_AIRSTRIKEMESSAGE, 2, ent->parent-g_entities );

/*	te = G_TempEntity( ent->parent->s.pos.trBase, EV_GLOBAL_CLIENT_SOUND );
	if ( ent->parent->client->sess.sessionTeam == TEAM_ALLIES ) {
		te->s.eventParm = G_SoundIndex( "allies_hq_airstrike" );
	} else {
		te->s.eventParm = G_SoundIndex( "axis_hq_airstrike" );
	}
	te->s.teamNum = ent->parent->s.clientNum;*/
//	te->s.effect1Time = 1;	// don't buffer

	VectorCopy( tr.endpos, bomboffset );
	VectorCopy( tr.endpos, skypoint );	
	traceheight = bomboffset[2];
	bottomtraceheight = traceheight - 8192.f;

	VectorSubtract( ent->s.pos.trBase, ent->parent->client->ps.origin, lookaxis );
	lookaxis[2] = 0;
	VectorNormalize( lookaxis );

	dir[0] = 0;
	dir[1] = 0;
	dir[2] = crandom(); // generate either up or down vector
	VectorNormalize( dir ); // which adds randomness to pass direction below

	for( j = 0; j < ent->count; j++ ) {		
		RotatePointAroundVector( bombaxis, dir, lookaxis, 90 + crandom() * 30 ); // munge the axis line a bit so it's not totally perpendicular
		VectorNormalize( bombaxis );

		VectorCopy( bombaxis, pos );
		VectorScale( pos,(float)(-.5f * BOMBSPREAD * NUMBOMBS ), pos );
		VectorAdd( ent->s.pos.trBase, pos, pos ); // first bomb position
		VectorScale( bombaxis, BOMBSPREAD, bombaxis ); // bomb drop direction offset

		for( i = 0; i < NUMBOMBS; i++ ) {
			bomb = G_Spawn();
			bomb->nextthink		= int( level.time + i * 100 + crandom() * 50 + 1000 + ( j * 2000 ) ); // 1000 for aircraft flyby, other term for tumble stagger
			bomb->think			= G_AirStrikeExplode;
			bomb->s.eType		= ET_MISSILE;
			bomb->r.svFlags		= SVF_NOCLIENT;
			bomb->s.weapon		= WP_SMOKE_MARKER; // might wanna change this
			bomb->r.ownerNum	= ent->s.number;
			bomb->parent		= ent->parent;
			bomb->s.teamNum		= ent->s.teamNum;
			bomb->damage		= 400; // maybe should un-hard-code these?
			bomb->splashDamage  = 400;

			// Gordon: for explosion type
			bomb->accuracy				= 2;
			bomb->classname				= "air strike";
			bomb->splashRadius			= 400;
			bomb->methodOfDeath			= MOD_AIRSTRIKE;
			bomb->splashMethodOfDeath	= MOD_AIRSTRIKE;
			bomb->clipmask		= MASK_MISSILESHOT;
			bomb->s.pos.trType	= TR_STATIONARY; // was TR_GRAVITY,  might wanna go back to this and drop from height
			//bomb->s.pos.trTime = level.time;		// move a bit on the very first frame
			bomboffset[0]		= crandom() * .5f * BOMBSPREAD;
			bomboffset[1]		= crandom() * .5f * BOMBSPREAD;
			bomboffset[2]		= 0.f;
			VectorAdd( pos, bomboffset, bomb->s.pos.trBase );

			VectorCopy( bomb->s.pos.trBase, bomboffset ); // make sure bombs fall "on top of" nonuniform scenery
			bomboffset[2]		= traceheight;

			VectorCopy( bomboffset, fallaxis );
			fallaxis[2]			= bottomtraceheight;


			trap_Trace( &tr, bomboffset, NULL, NULL, fallaxis, ent-g_entities, bomb->clipmask );
			if( tr.fraction != 1.0 ) {
				VectorCopy(tr.endpos,bomb->s.pos.trBase);

				// Snap origin!
				VectorMA( bomb->s.pos.trBase, 2.f, tr.plane.normal, temp );
				SnapVectorTowards( bomb->s.pos.trBase, temp );			// save net bandwidth

//				G_RailTrail( skypoint, bomb->s.pos.trBase );
				trap_TraceNoEnts( &tr, skypoint, NULL, NULL, bomb->s.pos.trBase, 0, CONTENTS_SOLID );
				if( tr.fraction < 1.f ) {
					G_FreeEntity( bomb );

					// move pos for next bomb
					VectorAdd( pos, bombaxis, pos );

					continue;
				}
			}

			VectorCopy( bomb->s.pos.trBase, bomb->r.currentOrigin );

			// move pos for next bomb
			VectorAdd( pos, bombaxis, pos );
		}
	}
}

// JPW NERVE -- sound effect for spotter round, had to do this as half-second bomb warning

void artilleryThink_real( gentity_t *ent ) {
	ent->freeAfterEvent = qtrue;
	trap_LinkEntity(ent);
	{
		int sfx = rand()%3;

		switch( sfx ) {
		case 0:	G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/artillery/artillery_fly_1.wav" )); break;
		case 1: G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/artillery/artillery_fly_2.wav" )); break;
		case 2: G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( "sound/weapons/artillery/artillery_fly_3.wav" )); break;
		}
	}
}
void artilleryThink( gentity_t *ent ) {
	// Jaybird - #43
	// If the player switched teams, do not allow the artillery strike
	if (!ent->parent->client || ent->parent->client->sess.sessionTeam != ent->s.teamNum) {
		ent->freeAfterEvent = qtrue;
		return;
	}

	ent->think = artilleryThink_real;
	ent->nextthink = level.time + 100;

	ent->r.svFlags = SVF_BROADCAST;
}

// JPW NERVE -- makes smoke disappear after a bit (just unregisters stuff)
void artilleryGoAway(gentity_t *ent) {
	ent->freeAfterEvent = qtrue;
	trap_LinkEntity(ent);
}

// JPW NERVE -- generates some smoke debris
void artillerySpotterThink( gentity_t *ent ) {
	gentity_t *bomb;
	vec3_t tmpdir;
	int i;
	ent->think = G_ExplodeMissile;
	ent->nextthink = level.time + 1;
	SnapVector( ent->s.pos.trBase );

	// Jaybird - #43
	// If the player switched teams, do not allow the artillery strike
	if (ent->parent->client && ent->parent->client->sess.sessionTeam != ent->s.teamNum) {
		ent->freeAfterEvent = qtrue;
		return;
	}

	for( i = 0; i < 7; i++ ) {
		bomb = G_Spawn();
		bomb->s.eType		= ET_MISSILE;
		bomb->r.svFlags		= 0;
		bomb->r.ownerNum	= ent->s.number;
		bomb->parent		= ent;
		bomb->s.teamNum		= ent->s.teamNum;
		bomb->nextthink		= int( level.time + 1000 + random() * 300 );
		bomb->classname		= "WP";				// WP == White Phosphorous, so we can check for bounce noise in grenade bounce routine
		bomb->damage		= 000;				// maybe should un-hard-code these?
		bomb->splashDamage  = 000;
		bomb->splashRadius	= 000;
		bomb->s.weapon		= WP_SMOKETRAIL;
		bomb->think			= artilleryGoAway;
		bomb->s.eFlags		|= EF_BOUNCE;
		bomb->clipmask		= MASK_MISSILESHOT;
		bomb->s.pos.trType	= TR_GRAVITY;		// was TR_GRAVITY,  might wanna go back to this and drop from height
		bomb->s.pos.trTime	= level.time;		// move a bit on the very first frame
		bomb->s.otherEntityNum2	= ent->s.otherEntityNum2;
		VectorCopy( ent->s.pos.trBase, bomb->s.pos.trBase );
		tmpdir[0]			= crandom();
		tmpdir[1]			= crandom();
		tmpdir[2]			= 1;
		VectorNormalize( tmpdir );
		tmpdir[2]			= 1; // extra up
		VectorScale( tmpdir, 500 + random() * 500, tmpdir );
		VectorCopy( tmpdir,bomb->s.pos.trDelta );
		SnapVector( bomb->s.pos.trDelta );			// save net bandwidth
		VectorCopy( ent->s.pos.trBase, bomb->s.pos.trBase );
		VectorCopy( ent->s.pos.trBase, bomb->r.currentOrigin );
	}
}

void G_GlobalClientEvent( int event, int param, int client ) {
	gentity_t* tent = G_TempEntity( vec3_origin, event );
	tent->s.density = param;
	tent->r.singleClient = client;
	tent->r.svFlags = SVF_SINGLECLIENT | SVF_BROADCAST;
}

/*
==================
Weapon_Artillery
==================
*/
void Weapon_Artillery(gentity_t *ent) {
	trace_t trace;
	int i, count;	
	vec3_t muzzlePoint, end, bomboffset, pos, fallaxis;
	float traceheight, bottomtraceheight;
	gentity_t *bomb, *bomb2;

	if( ent->client->ps.stats[STAT_PLAYER_CLASS] != PC_FIELDOPS ) {
		G_Printf("not a fieldops, you can't shoot this!\n");
		return;
	}

	// TAT - 10/27/2002 - moved energy check into a func, so I can use same check in bot code
	if( !ReadyToCallArtillery(ent) ) {
		return;
	}

	if( ent->client->sess.sessionTeam == TEAM_AXIS ) {
		if( !G_AvailableAirstrikes( ent ) ) {
			G_SayTo( ent, ent, 2, COLOR_YELLOW, "Fire Mission: ", "Insufficient fire support.", true );
			ent->active = qfalse;

			G_GlobalClientEvent( EV_ARTYMESSAGE, 0, ent-g_entities );

			return;
		}
	} else {
		if( !G_AvailableAirstrikes( ent ) ) {
			G_SayTo( ent, ent, 2, COLOR_YELLOW, "Fire Mission: ", "Insufficient fire support.", true );
			ent->active = qfalse;

			G_GlobalClientEvent( EV_ARTYMESSAGE, 0, ent-g_entities );

			return;
		}
	}

	AngleVectors (ent->client->ps.viewangles, __forward, __right, __up);

	VectorCopy( ent->r.currentOrigin, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;

	VectorMA (muzzlePoint, 8192, __forward, end);
	trap_Trace (&trace, muzzlePoint, NULL, NULL, end, ent->s.number, MASK_SHOT);

	if (trace.surfaceFlags & SURF_NOIMPACT)
		return;

	VectorCopy(trace.endpos,pos);
	VectorCopy(pos,bomboffset);
	bomboffset[2] += 4096;

	trap_Trace(&trace, pos, NULL, NULL, bomboffset, ent->s.number, MASK_SHOT);
	if ((trace.fraction < 1.0) && (!(trace.surfaceFlags & SURF_NOIMPACT)) ) { // JPW NERVE was SURF_SKY)) ) {
		G_SayTo( ent, ent, 2, COLOR_YELLOW, "Fire Mission: ", "Aborting, can't see target.", true );

		G_GlobalClientEvent( EV_ARTYMESSAGE, 1, ent-g_entities );

		return;
	}

	G_AddAirstrikeToCounters( ent );

	G_SayTo( ent, ent, 2, COLOR_YELLOW, "Fire Mission: ", "Firing for effect!", true );

	G_GlobalClientEvent( EV_ARTYMESSAGE, 2, ent-g_entities );

	VectorCopy( trace.endpos, bomboffset );
	traceheight = bomboffset[2];
	bottomtraceheight = traceheight - 8192;


// "spotter" round (i == 0)
// i == 1->4 is regular explosives
	if( ent->client->sess.skill[SK_SIGNALS] >= 3 ) {
		count = 9;
	} else {
		count = 5;
	}

	for( i = 0; i < count; i++ ) {
		bomb				= G_Spawn();
		bomb->think			= G_AirStrikeExplode;
		bomb->s.eType		= ET_MISSILE;
		bomb->r.svFlags		= SVF_NOCLIENT;
		bomb->s.weapon		= WP_ARTY; // might wanna change this
		bomb->r.ownerNum	= ent->s.number;
		bomb->s.clientNum	= ent->s.number;
		bomb->parent		= ent;
		bomb->s.teamNum		= ent->client->sess.sessionTeam;

		if (i == 0) {
			bomb->nextthink		= level.time + 5000;
			bomb->r.svFlags		= SVF_BROADCAST;
			bomb->classname		= "props_explosion"; // was "air strike"
			bomb->damage		= 0; // maybe should un-hard-code these?
			bomb->splashDamage  = 90;
			bomb->splashRadius	= 50;
			bomb->count			= 7;
			bomb->count2		= 1000;
			bomb->delay			= 300;
			bomb->s.otherEntityNum2 = 1;	// first bomb

			bomb->think = artillerySpotterThink;
		} else {
			bomb->nextthink		= int( level.time + 8950 + 2000 * i + crandom() * 800 );

			// Gordon: for explosion type
			bomb->accuracy		= 2;
			bomb->classname		= "air strike";
			bomb->damage		= 0;
			bomb->splashDamage  = 400;
			bomb->splashRadius	= 400;
		}
		bomb->methodOfDeath			= MOD_ARTY;
		bomb->splashMethodOfDeath	= MOD_ARTY;
		bomb->clipmask				= MASK_MISSILESHOT;
		bomb->s.pos.trType			= TR_STATIONARY; // was TR_GRAVITY,  might wanna go back to this and drop from height
		bomb->s.pos.trTime			= level.time;		// move a bit on the very first frame
		if( i ) { // spotter round is always dead on (OK, unrealistic but more fun)
			bomboffset[0] = crandom()*250;
			bomboffset[1] = crandom()*250;
		} else {
			bomboffset[0] = crandom()*50; // was 0; changed per id request to prevent spotter round assassinations
			bomboffset[1] = crandom()*50; // was 0;
		}
		bomboffset[2] = 0;
		VectorAdd(pos,bomboffset,bomb->s.pos.trBase);

		VectorCopy(bomb->s.pos.trBase,bomboffset); // make sure bombs fall "on top of" nonuniform scenery
		bomboffset[2] = traceheight;

		VectorCopy(bomboffset, fallaxis);
		fallaxis[2] = bottomtraceheight;

		trap_Trace(&trace, bomboffset, NULL, NULL, fallaxis, ent->s.number, MASK_SHOT);
		if (trace.fraction != 1.0)
			VectorCopy(trace.endpos,bomb->s.pos.trBase);	

		bomb->s.pos.trDelta[0] = 0; // might need to change this
		bomb->s.pos.trDelta[1] = 0;
		bomb->s.pos.trDelta[2] = 0;
		SnapVector( bomb->s.pos.trDelta );			// save net bandwidth
		VectorCopy (bomb->s.pos.trBase, bomb->r.currentOrigin);

// build arty falling sound effect in front of bomb drop
		bomb2 = G_Spawn();
		bomb2->think = artilleryThink;
		bomb2->s.eType	= ET_MISSILE;
		bomb2->r.svFlags	= SVF_NOCLIENT;
		bomb2->r.ownerNum	= ent->s.number;
		bomb2->parent		= ent;
		bomb2->s.teamNum	= ent->client->sess.sessionTeam;
		bomb2->damage		= 0;
		bomb2->nextthink = bomb->nextthink-600;
		bomb2->classname = "air strike";
		bomb2->clipmask = MASK_MISSILESHOT;
		bomb2->s.pos.trType = TR_STATIONARY; // was TR_GRAVITY,  might wanna go back to this and drop from height
		bomb2->s.pos.trTime = level.time;		// move a bit on the very first frame
		VectorCopy(bomb->s.pos.trBase,bomb2->s.pos.trBase);
		VectorCopy(bomb->s.pos.trDelta,bomb2->s.pos.trDelta);
		VectorCopy(bomb->s.pos.trBase,bomb2->r.currentOrigin);
	}

    if( ent->client->sess.skill[SK_SIGNALS] >= 5 && (cvars::bg_sk5_fdops.ivalue & SK5_FDO_CHARGE)) {
		if (level.time - ent->client->ps.classWeaponTime > level.lieutenantChargeTime[ent->client->sess.sessionTeam-1])
			ent->client->ps.classWeaponTime = level.time - level.lieutenantChargeTime[ent->client->sess.sessionTeam-1];
		
		ent->client->ps.classWeaponTime += int( 0.66f * level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
    }
	else if( ent->client->sess.skill[SK_SIGNALS] >= 2 ) {
		if (level.time - ent->client->ps.classWeaponTime > level.lieutenantChargeTime[ent->client->sess.sessionTeam-1])
			ent->client->ps.classWeaponTime = level.time - level.lieutenantChargeTime[ent->client->sess.sessionTeam-1];
		
		ent->client->ps.classWeaponTime += int( 0.66f * level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] );
    }
	else {
		ent->client->ps.classWeaponTime = level.time;
	}

	// OSP -- weapon stats
#ifndef DEBUG_STATS
	if(cvars::gameState.ivalue == GS_PLAYING)
#endif
		ent->client->sess.aWeaponStats[WS_ARTILLERY].atts++;

	// Omni-bot - Send a fire event.
	Bot_Event_FireWeapon(ent-g_entities, Bot_WeaponGameToBot(WP_ARTY), 0);
}


#define SMOKEBOMB_GROWTIME 1000
#define SMOKEBOMB_SMOKETIME 15000
#define SMOKEBOMB_POSTSMOKETIME 2000	
// xkan, 11/25/2002 - increases postsmoke time from 2000->32000, this way, the entity 
// is still around while the smoke is around, so we can check if it blocks bot's vision 
// Arnout: eeeeeh this is wrong. 32 seconds is way too long. Also - we shouldn't be
// rendering the grenade anymore after the smoke stops and definately not send it to the client
// xkan, 12/06/2002 - back to the old value 2000, now that it looks like smoke disappears more
// quickly

static void weapon_smokeBombExplode( gentity_t *ent ) {
	int lived = 0;

	if( !ent->grenadeExplodeTime )
		ent->grenadeExplodeTime = level.time;

	lived = level.time - ent->grenadeExplodeTime;
	ent->nextthink = level.time + FRAMETIME;

	if( lived < SMOKEBOMB_GROWTIME ) {
		// Just been thrown, increase radius
		ent->s.effect1Time = int( 16 + lived * ((640.f-16.f)/(float)SMOKEBOMB_GROWTIME) );
	} else if( lived < SMOKEBOMB_SMOKETIME + SMOKEBOMB_GROWTIME ) {
		// Smoking
		ent->s.effect1Time = 640;
	} else if( lived < SMOKEBOMB_SMOKETIME + SMOKEBOMB_GROWTIME + SMOKEBOMB_POSTSMOKETIME ) {
		// Dying out
		ent->s.effect1Time = -1;
	} else {
		// Poof and it's gone
		G_FreeEntity( ent );
	}
}

void G_PoisonGasExplode(gentity_t* ent) {
    int lived = 0;

    if (!ent->grenadeExplodeTime)
        ent->grenadeExplodeTime = level.time;

    lived = level.time - ent->grenadeExplodeTime;
    ent->nextthink = level.time + FRAMETIME;

    if (lived < SMOKEBOMB_GROWTIME) {
        // Just been thrown, increase radius
        ent->s.effect1Time = int( 16 + lived * ((640.f-16.f)/(float)SMOKEBOMB_GROWTIME) );
    }
    else if (lived < SMOKEBOMB_SMOKETIME + SMOKEBOMB_GROWTIME) {
        // Smoking
        ent->s.effect1Time = 640;

        if (level.time >= ent->poisonGasAlarm) {
            ent->poisonGasAlarm = level.time + 1500;
            etpro_RadiusDamage(
                ent->r.currentOrigin,
                ent,
                ent->parent,
                ent->poisonGasDamage,
                ent->poisonGasRadius,
                ent,
                MOD_POISON_GAS,
                RADIUS_SCOPE_CLIENTS );

            if (ent->parent->client)
	            ent->parent->client->sess.aWeaponStats[BG_WeapStatForWeapon( (weapon_t)ent->s.weapon )].subshots++;
        }
    }
    else if (lived < SMOKEBOMB_SMOKETIME + SMOKEBOMB_GROWTIME + SMOKEBOMB_POSTSMOKETIME) {
        // Dying out
        ent->s.effect1Time = -1;
    }
    else {
        // Poof and it's gone
        G_FreeEntity( ent );
    }
}

void G_PoisonGasTrip(gentity_t* ent) {
    gentity_t* landmine;

    ent->freeAfterEvent = qtrue;

    landmine = G_Spawn();

    landmine->r.ownerNum    = ent->r.ownerNum;
    landmine->r.svFlags     = SVF_BROADCAST;
    landmine->s.clientNum   = ent->s.clientNum;
    landmine->s.eFlags      = EF_BOUNCE_HALF | EF_BOUNCE;
    landmine->s.eType       = ET_MISSILE;
    landmine->s.effect1Time = 16;
    landmine->s.teamNum     = ent->s.teamNum;
    landmine->s.weapon      = WP_POISON_GAS;

    landmine->nextthink = level.time;
    landmine->think     = G_PoisonGasExplode;

    landmine->classname     = "poison_gas";
    landmine->clipmask      = MASK_MISSILESHOT;
    landmine->methodOfDeath = MOD_POISON_GAS;
    landmine->parent        = ent->parent;

    landmine->poisonGasAlarm  = level.time + SMOKEBOMB_GROWTIME;
    landmine->poisonGasDamage = 30;
    landmine->poisonGasRadius = 300;
    
    landmine->s.pos.trType = TR_GRAVITY;
    landmine->s.pos.trTime = level.time;

    VectorCopy( ent->r.currentOrigin, landmine->r.currentOrigin );
    VectorCopy( ent->s.pos.trBase, landmine->s.pos.trBase );

    // Try to behave a little like a bouncing-betty.
    landmine->s.pos.trDelta[0] = 0.0f;
    landmine->s.pos.trDelta[1] = 0.0f;
    landmine->s.pos.trDelta[2] = 400.0f;

    G_AddEvent(ent, EV_LANDMINE_LAUNCH, 0);
}

void G_BouncingBettyTrip(gentity_t* ent) {
    ent->think     = G_ExplodeMissile;
    ent->nextthink = level.time + 400;

    // Try to behave a little like a bouncing-betty.
    ent->s.pos.trType = TR_GRAVITY;
    ent->s.pos.trTime = level.time;

    ent->s.pos.trDelta[0] = 0.0f;
    ent->s.pos.trDelta[1] = 0.0f;
    ent->s.pos.trDelta[2] = 400.0f;

    G_AddEvent(ent, EV_LANDMINE_LAUNCH, 0);
}

gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity, int ownerNum );
// jpw

/*
======================================================================

MACHINEGUN

======================================================================
*/

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/

// (SA) modified so it doesn't have trouble with negative locations (quadrant problems)
//			(this was causing some problems with bullet marks appearing since snapping
//			too far off the target surface causes the the distance between the transmitted impact
//			point and the actual hit surface larger than the mark radius.  (so nothing shows) )

void
SnapVectorTowards( vec3_t v, const vec3_t to )
{
    v[0] = (to[0] <= v[0]) ? floor( v[0] ) : ceil( v[0] );
    v[1] = (to[1] <= v[1]) ? floor( v[1] ) : ceil( v[1] );
    v[2] = (to[2] <= v[2]) ? floor( v[2] ) : ceil( v[2] );
}

// JPW
// mechanism allows different weapon damage for single/multiplayer; we want "balanced" weapons
// in multiplayer but don't want to alter the existing single-player damage items that have already
// been changed
//
// KLUDGE/FIXME: also modded #defines below to become macros that call this fn for minimal impact elsewhere
//
int G_GetWeaponDamage( int weapon ) {
		switch (weapon) {
		default:
			return 1;
		case WP_KNIFE: 
		case WP_M97:
			return 12;
		case WP_STEN: 
			return 14;
		case WP_CARBINE:
		case WP_GARAND:
		case WP_KAR98:
		case WP_K43:
			return 34;
		case WP_FG42: 
			return 15;
		case WP_LUGER:
		case WP_SILENCER:
		case WP_AKIMBO_LUGER:
		case WP_AKIMBO_SILENCEDLUGER:
		case WP_COLT:
		case WP_SILENCED_COLT:
		case WP_AKIMBO_COLT:
		case WP_AKIMBO_SILENCEDCOLT: 
		case WP_THOMPSON: 
		case WP_MP40: 
		case WP_MOBILE_MG42: 
		case WP_MOBILE_MG42_SET:
			return 18;
		case WP_FG42SCOPE: 
			return 30;
		case WP_GARAND_SCOPE: 
		case WP_K43_SCOPE: 
			return 50;
		case WP_SMOKE_MARKER: 
			return 140; // just enough to kill somebody standing on it
		case WP_MAPMORTAR: 
		case WP_GRENADE_LAUNCHER: 
		case WP_GRENADE_PINEAPPLE: 
		case WP_GPG40:
		case WP_M7: 
		case WP_LANDMINE: 
		case WP_SATCHEL:
			return 250;
		case WP_TRIPMINE: 
			return 300;
		case WP_PANZERFAUST: 
		case WP_MORTAR_SET: 
		case WP_DYNAMITE: 
			return 400;
		case WP_LANDMINE_BBETTY: 
			return 350;
	}
}


float G_GetWeaponSpread( int weapon ) {
	switch (weapon) {
		case WP_LUGER:
		case WP_SILENCER:
		case WP_AKIMBO_LUGER:
		case WP_AKIMBO_SILENCEDLUGER:
			return 600;
		case WP_COLT:
		case WP_SILENCED_COLT:
		case WP_AKIMBO_COLT:
		case WP_AKIMBO_SILENCEDCOLT:
			return 600;
		case WP_MP40:
		case WP_THOMPSON:
			return 400;
		case WP_STEN:
			return 200;
		case WP_FG42SCOPE:
			return 200;
		case WP_FG42:
			return 500;
		case WP_GARAND:
		case WP_CARBINE:
		case WP_KAR98:
		case WP_K43:
			return 250;
		case WP_GARAND_SCOPE:
		case WP_K43_SCOPE:
			return 1000;
		case WP_MOBILE_MG42:
		case WP_MOBILE_MG42_SET:
			return 2500;
	}

	G_Printf( "shouldn't ever get here (weapon %d)\n", weapon );
	// jpw
	return 0;	// shouldn't get here
}

#define LUGER_SPREAD	G_GetWeaponSpread(WP_LUGER)
#define LUGER_DAMAGE	G_GetWeaponDamage(WP_LUGER) // JPW

#define SILENCER_DAMAGE		G_GetWeaponDamage(WP_SILENCER)
#define SILENCER_SPREAD		G_GetWeaponSpread(WP_SILENCER)

#define AKIMBO_LUGER_DAMAGE			G_GetWeaponDamage(WP_AKIMBO_LUGER)
#define AKIMBO_LUGER_SPREAD			G_GetWeaponSpread(WP_AKIMBO_LUGER)

#define AKIMBO_SILENCEDLUGER_DAMAGE	G_GetWeaponDamage(WP_AKIMBO_SILENCEDLUGER)
#define AKIMBO_SILENCEDLUGER_SPREAD	G_GetWeaponSpread(WP_AKIMBO_SILENCEDLUGER)

#define COLT_SPREAD		G_GetWeaponSpread(WP_COLT)
#define	COLT_DAMAGE		G_GetWeaponDamage(WP_COLT) // JPW

#define SILENCED_COLT_DAMAGE	G_GetWeaponDamage(WP_SILENCED_COLT)
#define SILENCED_COLT_SPREAD	G_GetWeaponSpread(WP_SILENCED_COLT)

#define AKIMBO_COLT_DAMAGE	G_GetWeaponDamage(WP_AKIMBO_COLT)
#define AKIMBO_COLT_SPREAD	G_GetWeaponSpread(WP_AKIMBO_COLT)

#define AKIMBO_SILENCEDCOLT_DAMAGE	G_GetWeaponDamage(WP_AKIMBO_SILENCEDCOLT)
#define AKIMBO_SILENCEDCOLT_SPREAD	G_GetWeaponSpread(WP_AKIMBO_SILENCEDCOLT)

#define MP40_SPREAD		G_GetWeaponSpread(WP_MP40)
#define	MP40_DAMAGE		G_GetWeaponDamage(WP_MP40) // JPW
#define THOMPSON_SPREAD	G_GetWeaponSpread(WP_THOMPSON)
#define	THOMPSON_DAMAGE	G_GetWeaponDamage(WP_THOMPSON) // JPW
#define STEN_SPREAD		G_GetWeaponSpread(WP_STEN)
#define	STEN_DAMAGE		G_GetWeaponDamage(WP_STEN) // JPW
#define	M97_DAMAGE		G_GetWeaponDamage(WP_M97)	// Jaybird

#define GARAND_SPREAD	G_GetWeaponSpread(WP_GARAND)
#define	GARAND_DAMAGE	G_GetWeaponDamage(WP_GARAND) // JPW

#define KAR98_SPREAD	G_GetWeaponSpread(WP_KAR98)
#define	KAR98_DAMAGE	G_GetWeaponDamage(WP_KAR98)

#define CARBINE_SPREAD	G_GetWeaponSpread(WP_CARBINE)
#define	CARBINE_DAMAGE	G_GetWeaponDamage(WP_CARBINE)

#define	KAR98_GREN_DAMAGE	G_GetWeaponDamage(WP_GREN_KAR98)

#define MOBILE_MG42_SPREAD	G_GetWeaponSpread(WP_MOBILE_MG42)
#define	MOBILE_MG42_DAMAGE	G_GetWeaponDamage(WP_MOBILE_MG42)

#define FG42_SPREAD		G_GetWeaponSpread(WP_FG42)
#define	FG42_DAMAGE		G_GetWeaponDamage(WP_FG42) // JPW

#define FG42SCOPE_SPREAD	G_GetWeaponSpread(WP_FG42SCOPE)
#define	FG42SCOPE_DAMAGE	G_GetWeaponDamage(WP_FG42SCOPE) // JPW
#define K43_SPREAD	G_GetWeaponSpread(WP_K43)
#define	K43_DAMAGE	G_GetWeaponDamage(WP_K43)

#define GARANDSCOPE_SPREAD	G_GetWeaponSpread(WP_GARAND_SCOPE)
#define GARANDSCOPE_DAMAGE	G_GetWeaponDamage(WP_GARAND_SCOPE)

#define K43SCOPE_SPREAD	G_GetWeaponSpread(WP_K43_SCOPE)
#define K43SCOPE_DAMAGE G_GetWeaponDamage(WP_K43_SCOPE)

static void
RubbleFlagCheck( const gentity_t& ent, const trace_t& tr )
{
#if 0
    qboolean    is_valid = qfalse;
    int         type = 0;

    // (SA) moving client-side

    return;




    if (tr.surfaceFlags & SURF_RUBBLE || tr.surfaceFlags & SURF_GRAVEL)
    {
        is_valid = qtrue;
        type = 4;
    }
    else if (tr.surfaceFlags & SURF_METAL)
    {
//----(SA)  removed
//      is_valid = qtrue;
//      type = 2;
    }
    else if (tr.surfaceFlags & SURF_WOOD)
    {
        is_valid = qtrue;
        type = 1;
    }
    
    if (is_valid && ent.client && ( ent.client->ps.persistant[PERS_HWEAPON_USE] ) )
    {
        if (rand()%100 > 75)
        {
            gentity_t   *sfx;
            vec3_t      start;
            vec3_t      dir;

            sfx = G_Spawn ();

            sfx->s.density = type; 

            VectorCopy (tr.endpos, start);

            VectorCopy (__muzzleTrace, dir);
            VectorNegate (dir, dir);

            G_SetOrigin (sfx, start); 
            G_SetAngle (sfx, dir);

            G_AddEvent( sfx, EV_SHARD, DirToByte( dir ));

            sfx->think = G_FreeEntity;
            sfx->nextthink = level.time + 1000;

            sfx->s.frame = 3 + (rand()%3) ;
                                
            trap_LinkEntity (sfx);

        }
    }
#endif

    int type = 0;

    if (tr.surfaceFlags & (SURF_RUBBLE | SURF_GRAVEL))
        type = 4;
    else if (tr.surfaceFlags & SURF_WOOD)
        type = 1;

    if (!type || !ent.client || !ent.client->ps.persistant[PERS_HWEAPON_USE] || !(rand() % 100 > 75))
        return;

    gentity_t& fx = *G_Spawn();
    fx.s.density = type;
    fx.s.frame   = 3 + (rand() % 3);
    fx.think     = G_FreeEntity;
    fx.nextthink = level.time + 1000;

    vec3_t start;
    VectorCopy( tr.endpos, start );

    vec3_t dir;
    VectorCopy( __muzzleTrace, dir );
    VectorNegate( dir, dir );

    G_SetOrigin( &fx, start );
    G_SetAngle( &fx, dir );

    G_AddEvent( &fx, EV_SHARD, DirToByte( dir ));

    trap_LinkEntity( &fx );
}

/*
==============
EmitterCheck
	see if a new particle emitter should be created at the bullet impact point
==============
*/
static void
EmitterCheck( const gentity_t& ent, gentity_t& attacker, trace_t& tr )
{
	vec3_t origin;
	VectorCopy(tr.endpos, origin);
	SnapVectorTowards( tr.endpos, attacker.s.origin );

	if (Q_stricmp( ent.classname, "func_leaky" ) == 0) {
		gentity_t& fx = *G_TempEntity( origin, EV_EMITTER );
		VectorCopy( origin, fx.s.origin );
		fx.s.time = 1234;
		fx.s.density = 9876;
		VectorCopy (tr.plane.normal, fx.s.origin2);
	}
}


/*
==============
Bullet_Endpos
	find target end position for bullet trace based on entities weapon and accuracy
==============
*/
void Bullet_Endpos(gentity_t *ent, float spread, vec3_t *end) {
	float		r, u;
	qboolean	randSpread = qtrue;
	int			dist = 8192;

	r = crandom()*spread;
	u = crandom()*spread;

	if(BG_IsScopedWeapon(ent->s.weapon)) {
		// aim dir already accounted for sway of scoped weapons in CalcMuzzlePoints()
		dist*= 2;
		randSpread = qfalse;
	}

	VectorMA (__muzzleTrace, dist, __forward, *end);

	if(randSpread) {
		VectorMA (*end, r, __right, *end);
		VectorMA (*end, u, __up, *end);
	}
}

/*
==============
Bullet_Fire_Extended
	A modified Bullet_Fire with more parameters.
	The original Bullet_Fire still passes through here and functions as it always has.

	uses for this include shooting through entities (windows, doors, other players, etc.) and reflecting bullets
==============
*/
static bool
Bullet_Fire_Extended(
    gentity_t* source,
    gentity_t* actor,
    vec3_t     start,
    vec3_t     end,
    int        damage,
    bool       distanceFalloff,
    bool       noEvents)
{
    // Give active bullet-model a chance to adjust start point.
    if (source->client)
        g_clientObjects[source->s.number].bulletModel->adjustStartPoint( start );
    else if (actor->client)
        g_clientObjects[actor->s.number].bulletModel->adjustStartPoint( start );

    // Do the trace
    TraceContext trx(*source, *actor, MASK_SHOT, reinterpret_cast<vec_t(&)[3]>(*start), reinterpret_cast<vec_t(&)[3]>(*end) );
    if (!AbstractBulletModel::fireWorld( trx ))
        return qfalse;

    gentity_t& traceEnt = g_entities[ trx.data.entityNum ];
    RubbleFlagCheck( *actor, trx.data );
    EmitterCheck( traceEnt, *actor, trx.data );

    // snap the endpos to integers, but nudged towards the line
    SnapVectorTowards( trx.data.endpos, start );

    // Allow for distance falloff
    if (distanceFalloff)
        damage = G_DistanceFalloffAdjustment( damage, trx.flen, 0.5f );

    bool hitClient = false;

    // Send bullet impact
    if (!noEvents) {
        if (traceEnt.takedamage && traceEnt.client) {
            gentity_t& effect = *G_TempEntity( trx.data.endpos, EV_BULLET_HIT_FLESH );
            effect.s.eventParm = traceEnt.s.number;
            effect.s.otherEntityNum = actor->s.number;

            if (AccuracyHit( &traceEnt, actor ))
                hitClient = true;
        } else {
            // Ridah, bullet impact should reflect off surface
            gentity_t& fx = *G_TempEntity( trx.data.endpos, EV_BULLET_HIT_WALL );
            fx.s.otherEntityNum  = actor->s.number;
            fx.s.otherEntityNum2 = ENTITYNUM_NONE;

            if (trx.water) {
                vec3_t v;
                VectorSubtract( trx.data.endpos, start, v );
                fx.s.origin2[0] = (8192 * trx.wdata.fraction) / VectorLength( v );
            }

            float dot = DotProduct( __forward, trx.data.plane.normal );

            vec3_t  reflect;
            VectorMA( __forward, -2*dot, trx.data.plane.normal, reflect );
            VectorNormalize( reflect );

            fx.s.eventParm = DirToByte( reflect );
        }
    }

    // Inflict Damage
    if (trx.resultIsPlayer()) {
        g_clientObjects[ traceEnt.s.number ].takeBulletDamageFrom(
            trx,
            g_clientObjects[ actor->s.number ],
            damage,
            (distanceFalloff ? DAMAGE_DISTANCEFALLOFF : 0 ),
            GetAmmoTableData(actor->s.weapon)->mod);
    } else if (traceEnt.takedamage) {
        G_Damage(
            &traceEnt,
            actor,
            actor,
            __forward,
            trx.data.endpos,
            damage,
            (distanceFalloff ? DAMAGE_DISTANCEFALLOFF : 0 ),
            GetAmmoTableData( actor->s.weapon )->mod );

        // allow bullets to "pass through" func_explosives if they break by taking another simultanious shot
        if (traceEnt.s.eType == ET_EXPLOSIVE) {
            if (traceEnt.health <= damage) {
                // start new bullet at position this hit the bmodel and continue to the end position
                // (ignoring shot-through bmodel in next trace)
                // spread = 0 as this is an extension of an already spread shot
                return Bullet_Fire_Extended( &traceEnt, actor, trx.data.endpos, end, damage, distanceFalloff, noEvents );
            }
        }
    }

    return hitClient;
}

/*
==============
Bullet_Fire
==============
*/
void Bullet_Fire (gentity_t *ent, float spread, int damage, qboolean distance_falloff) {
	vec3_t		end;

	// Gordon: skill thing should be here Arnout! 
	switch( ent->s.weapon ) {
		// light weapons
		case WP_LUGER:
		case WP_COLT:
		case WP_MP40:
		case WP_THOMPSON:
		case WP_STEN:
		case WP_SILENCER:
		case WP_SILENCED_COLT:
		case WP_AKIMBO_LUGER:			// CHRUKER: b045 - Akimbo weapons also need spread reduction
		case WP_AKIMBO_COLT:			// CHRUKER: b045 - Akimbo weapons also need spread reduction
		case WP_AKIMBO_SILENCEDLUGER:	// CHRUKER: b045 - Akimbo weapons also need spread reduction
		case WP_AKIMBO_SILENCEDCOLT:	// CHRUKER: b045 - Akimbo weapons also need spread reduction
			// CHRUKER: b007 - The reduction should kick in at level 3 not level 4
			if( ent->client->sess.skill[SK_LIGHT_WEAPONS] >= 3 )
				spread *= .65f;
			break;
	}

 	Bullet_Endpos(ent, spread, &end);

    Bullet_Fire_Extended( ent, ent, __muzzleTrace, end, damage, distance_falloff, false );
}

/*
==============
Weapon_M97
--------------
Jaybird
Note: need to optimize to not create
so many events for one shot.  This may
be ok, however, since the shotgun will
only be able to shoot once a second.
==============
*/
void Weapon_M97( gentity_t *ent ) {
	int			i;
	bool		hitClient = false;
	int			hits, totalHits, seed;
	vec3_t		end;

    // Send the shotgun event
    gentity_t* ev = G_TempEntity(__muzzleTrace, EV_M97);
    VectorScale( __forward, 4096, ev->s.origin2 );
    SnapVector( ev->s.origin2 );
    ev->s.otherEntityNum = ent->s.number;

    // Provide the seed
    seed = ev->s.eventParm = rand() % 255;

    // Back these up so that we only give them a single hit
	hits = ent->client->sess.aWeaponStats[BG_WeapStatForWeapon(WP_M97)].hits;
	totalHits = ent->client->sess.aWeaponStats[BG_WeapStatForWeapon(WP_M97)].totalHits;

	// Derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( ev->s.origin2, __forward );
	PerpendicularVector( __right, __forward );
	CrossProduct( __forward, __right, __up );

    // Fire the bullets in a spread
    for ( i = 0; i < M97_COUNT ; i++ ) {
        float		r, u;

        // Get the endpoint
		r = Q_crandom( &seed ) * M97_SPREAD * 16;
		u = Q_crandom( &seed ) * M97_SPREAD * 16;
		VectorMA( ev->s.pos.trBase, 8192 * 16, __forward, end);
		VectorMA (end, r, __right, end);
		VectorMA (end, u, __up, end);
    
        if (Bullet_Fire_Extended( ent, ent, __muzzleTrace, end, M97_DAMAGE, qtrue, true ) && !hitClient )
            hitClient = true;
    }

    // Now give stats
	if( hitClient ) {
		ent->client->sess.aWeaponStats[BG_WeapStatForWeapon(WP_M97)].hits = ++hits;
		ent->client->sess.aWeaponStats[BG_WeapStatForWeapon(WP_M97)].totalHits = ++totalHits;
	}
}


/*
======================================================================

GRENADE LAUNCHER

  700 has been the standard direction multiplier in fire_grenade()

======================================================================
*/
extern void G_ExplodeMissilePoisonGas (gentity_t *ent);

gentity_t *weapon_gpg40_fire (gentity_t *ent, int grenType) {
	gentity_t	*m/*, *te*/; // JPW NERVE
	trace_t		tr;
	vec3_t		viewpos;
//	float		upangle = 0, pitch;			//	start with level throwing and adjust based on angle
	vec3_t		tosspos;
	//bani - to prevent nade-through-teamdoor sploit
	vec3_t	orig_viewpos;

	AngleVectors(ent->client->ps.viewangles, __forward, NULL, NULL);

	VectorCopy(__muzzleEffect, tosspos);

	// check for valid start spot (so you don't throw through or get stuck in a wall)
	VectorCopy( ent->s.pos.trBase, viewpos );
	viewpos[2] += ent->client->ps.viewheight;
	VectorCopy( viewpos, orig_viewpos );	//bani - to prevent nade-through-teamdoor sploit
	VectorMA( viewpos, 32, __forward, viewpos);

	//bani - to prevent nade-through-teamdoor sploit
	trap_Trace( &tr, orig_viewpos, tv( -4.f, -4.f, 0.f ), tv( 4.f, 4.f, 6.f ), viewpos, ent->s.number, MASK_MISSILESHOT );
	if( tr.fraction < 1 ) { // oops, bad launch spot ) {
		VectorCopy( tr.endpos, tosspos );
		SnapVectorTowards( tosspos, orig_viewpos );
	} else {
		trap_Trace (&tr, viewpos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), tosspos, ent->s.number, MASK_MISSILESHOT );
		if( tr.fraction < 1 ) { // oops, bad launch spot
			VectorCopy(tr.endpos, tosspos);
			SnapVectorTowards( tosspos, viewpos );
		}
	}

	VectorScale(__forward, 2000, __forward);

	m = fire_grenade (ent, tosspos, __forward, grenType);

	m->damage = 0;
	
	// Ridah, return the grenade so we can do some prediction before deciding if we really want to throw it or not
	return m;
}

gentity_t *weapon_mortar_fire( gentity_t *ent, int grenType ) {
	gentity_t	*m;
	trace_t		tr;
	vec3_t		launchPos, testPos;
	vec3_t		angles;

	VectorCopy( ent->client->ps.viewangles, angles );
	angles[PITCH] -= 60.f;
/*	if( angles[PITCH] < -89.f )
		angles[PITCH] = -89.f;*/
	AngleVectors( angles, __forward, NULL, NULL );

	VectorCopy( __muzzleEffect, launchPos );

	// check for valid start spot (so you don't throw through or get stuck in a wall)
	VectorMA( launchPos, 32, __forward, testPos);

	// Gordon: hack so i can do inverse trajectory calcs easily :p
	__forward[0] *= 3000*1.1f;
	__forward[1] *= 3000*1.1f;
	__forward[2] *= 1500*1.1f;

	trap_Trace (&tr, testPos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), launchPos, ent->s.number, MASK_MISSILESHOT);

	if( tr.fraction < 1 ) {	// oops, bad launch spot
		VectorCopy( tr.endpos, launchPos );
		SnapVectorTowards( launchPos, testPos );
	}

	m = fire_grenade( ent, launchPos, __forward, grenType );

	return m;
}

gentity_t *weapon_grenadelauncher_fire (gentity_t *ent, int grenType) {
	gentity_t	*m;
	trace_t		tr;
	vec3_t		viewpos;
	float		upangle = 0, pitch;			//	start with level throwing and adjust based on angle
	vec3_t		tosspos;
	qboolean	underhand = qtrue;

	pitch = ent->s.apos.trBase[0];

	// JPW NERVE -- smoke grenades always overhand
	if( pitch >= 0 ) {
		__forward[2] += 0.5f;
		// Used later in underhand boost
		pitch = 1.3f;
	}
	else {
		pitch = -pitch;
		pitch = min( pitch, 30.0f );
		pitch /= 30.f;
		pitch = 1 - pitch;
		__forward[2] += (pitch * 0.5f);

		// Used later in underhand boost
		pitch *= 0.3f;
		pitch += 1.f;
	}

	VectorNormalizeFast( __forward );			//	make sure forward is normalized

	upangle = -(ent->s.apos.trBase[0]);	//	this will give between	-90 / 90
	upangle = min(upangle, 50.0f);
	upangle = max(upangle, -50.0f);		//	now clamped to	-50 / 50	(don't allow firing straight up/down)
	upangle = upangle/100.0f;			//				   -0.5 / 0.5
	upangle += 0.5f;					//				    0.0 / 1.0

	if(upangle < .1)
		upangle = .1;

	// pineapples are not thrown as far as mashers // Gordon: um, no?
    switch (grenType) {
        case WP_GRENADE_LAUNCHER:
        case WP_GRENADE_PINEAPPLE:
        case WP_SMOKE_MARKER:
        case WP_SMOKE_BOMB:
        case WP_POISON_GAS:
            upangle *= 900;
            break;

        default:
            upangle *= 400;
            break;
    }

	VectorCopy(__muzzleEffect, tosspos);

	if(underhand) {
		// move a little bit more away from the player (so underhand tosses don't get caught on nearby lips)
		VectorMA(__muzzleEffect, 8, __forward, tosspos);
		tosspos[2] -= 8;	// lower origin for the underhand throw
		upangle *= pitch;
		SnapVector( tosspos );
	}

	VectorScale(__forward, upangle, __forward);

	// check for valid start spot (so you don't throw through or get stuck in a wall)
	VectorCopy( ent->s.pos.trBase, viewpos );
	viewpos[2] += ent->client->ps.viewheight;

	switch (grenType) {
		case WP_DYNAMITE:
		case WP_SATCHEL:
			trap_Trace (&tr, viewpos, tv(-12.f, -12.f, 0.f), tv(12.f, 12.f, 20.f), tosspos, ent->s.number, MASK_MISSILESHOT);
			break;

		case WP_LANDMINE:
		case WP_LANDMINE_BBETTY:
		case WP_LANDMINE_PGAS:
			trap_Trace (&tr, viewpos, tv(-16.f, -16.f, 0.f), tv(16.f, 16.f, 16.f), tosspos, ent->s.number, MASK_MISSILESHOT);
			break;

		default:
			trap_Trace (&tr, viewpos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), tosspos, ent->s.number, MASK_MISSILESHOT);
			break;
	}

	if( tr.startsolid ) {
		// Arnout: this code is a bit more solid than the previous code
		VectorCopy( __forward, viewpos );
		VectorNormalizeFast( viewpos );
		VectorMA( ent->r.currentOrigin, -24.f, viewpos, viewpos ); 

		switch (grenType) {
			case WP_DYNAMITE:
			case WP_SATCHEL:
				trap_Trace (&tr, viewpos, tv(-12.f, -12.f, 0.f), tv(12.f, 12.f, 20.f), tosspos, ent->s.number, MASK_MISSILESHOT);
				break;

			case WP_LANDMINE:
			case WP_LANDMINE_BBETTY:
			case WP_LANDMINE_PGAS:
				trap_Trace (&tr, viewpos, tv(-16.f, -16.f, 0.f), tv(16.f, 16.f, 16.f), tosspos, ent->s.number, MASK_MISSILESHOT);
				break;

			default:
				trap_Trace (&tr, viewpos, tv(-4.f, -4.f, 0.f), tv(4.f, 4.f, 6.f), tosspos, ent->s.number, MASK_MISSILESHOT);
				break;
		}
		VectorCopy( tr.endpos, tosspos );
	} else if( tr.fraction < 1 ) {	// oops, bad launch spot
		VectorCopy( tr.endpos, tosspos );
		SnapVectorTowards( tosspos, viewpos );
	}

	m = fire_grenade (ent, tosspos, __forward, grenType);

	m->damage = 0;	// Ridah, grenade's don't explode on contact

    switch (grenType) {
        case WP_LANDMINE:
		case WP_LANDMINE_BBETTY:
		case WP_LANDMINE_PGAS:
            if (ent->client->sess.sessionTeam == TEAM_AXIS) // store team so we can generate red or blue smoke
                m->s.otherEntityNum2 = 1;
            else
                m->s.otherEntityNum2 = 0;
            break;

        case WP_SMOKE_BOMB:
            m->s.effect1Time = 16;
            m->think = weapon_smokeBombExplode;
            break;

        case WP_POISON_GAS:
            m->s.effect1Time = 16;
            m->think = G_PoisonGasExplode;
            m->poisonGasAlarm  = level.time + SMOKEBOMB_GROWTIME;
            m->poisonGasDamage = 30;
            m->poisonGasRadius = 300;
            break;

        case WP_SMOKE_MARKER:
		    m->s.teamNum = ent->client->sess.sessionTeam;	// store team so we can generate red or blue smoke
            if( ent->client->sess.skill[SK_SIGNALS] >= 3 ) {
                m->count = 2;
                m->nextthink = level.time + 3500;
                m->think = weapon_checkAirStrikeThink2;
            }
            else {
                m->count = 1;
                m->nextthink = level.time + 2500;
                m->think = weapon_checkAirStrikeThink1;
		    }
            break;

        default:
            break;
	}

	//----(SA)	adjust for movement of character.  TODO: Probably comment in later, but only for forward/back not strafing
	//VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics

	// Ridah, return the grenade so we can do some prediction before deciding if we really want to throw it or not
	return m;
}

/*
======================================================================

ROCKET

======================================================================
*/

gentity_t *Weapon_Panzerfaust_Fire( gentity_t *ent ) {
	gentity_t	*m = fire_rocket (ent, __muzzleEffect, __forward);
//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
	return m;
}


/*
======================================================================

SPEARGUN

======================================================================
*/
/*void Weapon_Speargun_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_speargun (ent, muzzleEffect, forward);
}*/


/*
======================================================================

LIGHTNING GUN

======================================================================
*/

// rain - BurnMeGood now takes the flamechunk separately, because
// the old 'set-self-in-flames' method doesn't have a flamechunk to
// pass, and deaths were getting blamed on the world/player 0
void G_BurnMeGood( gentity_t *self, gentity_t *body, gentity_t *chunk )
{
	vec3_t origin;
	
	// rain - fill in our own origin if we have no flamechunk
	if (chunk != NULL)
		VectorCopy(chunk->r.currentOrigin, origin);
	else
		VectorCopy(self->r.currentOrigin, origin);
	
	// add the new damage
	body->flameQuota += 5;
	body->flameQuotaTime = level.time;
	
	// JPW NERVE -- yet another flamethrower damage model, trying to find a feels-good damage combo that isn't overpowered
	if (body->lastBurnedFrameNumber != level.framenum) {
		G_Damage( body, self, self, vec3_origin, self->r.currentOrigin, 5, 0, MOD_FLAMETHROWER ); // was 2 dmg in release ver, hit avg. 2.5 times per frame
		body->lastBurnedFrameNumber = level.framenum;
	}
	// jpw
	
	// make em burn
	if( body->client && (body->health <= 0 || body->flameQuota > 0) ) { // JPW NERVE was > FLAME_THRESHOLD
		if (body->s.onFireEnd < level.time)
			body->s.onFireStart = level.time;

		body->s.onFireEnd = level.time + FIRE_FLASH_TIME;
		// rain - use ourself as the attacker if we have no flamechunk
		body->flameBurnEnt = chunk != NULL ? chunk->r.ownerNum : self->s.number;
		// add to playerState for client-side effect
		body->client->ps.onFireStart = level.time;
	}
}

// TTimo - for traces calls
static vec3_t	flameChunkMins = {-4, -4, -4};
static vec3_t	flameChunkMaxs = { 4,  4,  4};

gentity_t *Weapon_FlamethrowerFire( gentity_t *ent ) {
	gentity_t	*traceEnt;
	vec3_t		start;
	vec3_t		trace_start;
	vec3_t		trace_end;
	trace_t 	trace;

	VectorCopy( ent->r.currentOrigin, start );
	start[2] += ent->client->ps.viewheight;
	VectorCopy( start, trace_start );

	VectorMA( start, -8, __forward, start );
	VectorMA( start, 10, __right, start );
	VectorMA( start, -6, __up, start );
	
	// prevent flame thrower cheat, run & fire while aiming at the ground, don't get hurt
	// 72 total box height, 18 xy -> 77 trace radius (from view point towards the ground) is enough to cover the area around the feet
	VectorMA( trace_start, 77.0, __forward, trace_end);
	trap_Trace( &trace, trace_start, flameChunkMins, flameChunkMaxs, trace_end, ent->s.number, MASK_SHOT | MASK_WATER );	
	if (trace.fraction != 1.0)
	{
		// additional checks to filter out false positives
		if (trace.endpos[2] > (ent->r.currentOrigin[2]+ent->r.mins[2]-8) && trace.endpos[2] < ent->r.currentOrigin[2]) 
		{
			// trigger in a 21 radius around origin
			trace_start[0] -= trace.endpos[0];
			trace_start[1] -= trace.endpos[1];
			if (trace_start[0]*trace_start[0]+trace_start[1]*trace_start[1] < 441)
			{
				// set self in flames
				// rain - fixed BurnMeGood
				G_BurnMeGood( ent, ent, NULL );
			}
		}
	}

	traceEnt = fire_flamechunk ( ent, start, __forward );
	return traceEnt;
}

//======================================================================


/*
==============
AddLean
	add leaning offset
==============
*/
void AddLean(gentity_t *ent, vec3_t point)
{
	if(ent->client)
	{
		if(ent->client->ps.leanf)
		{
			vec3_t	right;
			AngleVectors(ent->client->ps.viewangles, NULL, right, NULL);
			VectorMA(point, ent->client->ps.leanf, right, point);
		}
	}
}

/*
===============
AccuracyHit
===============
*/
qboolean AccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if( !attacker ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, int weapon, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->r.currentOrigin, muzzlePoint );
	// Jaybird - adjust for lower bbox
	if (ent->client->ps.viewangles[PITCH] > 30 && (ent->client->ps.eFlags & EF_CROUCHING))
		muzzlePoint[2] += 30;
	else
		muzzlePoint[2] += ent->client->ps.viewheight;
	// Ridah, this puts the start point outside the bounding box, isn't necessary
//	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// done.

	// Ridah, offset for more realistic firing from actual gun position
	//----(SA) modified
	switch(weapon)	// Ridah, changed this so I can predict weapons
	{
		case WP_PANZERFAUST:
			VectorMA(muzzlePoint,10,right,muzzlePoint);
			break;
		case WP_DYNAMITE:
		case WP_GRENADE_PINEAPPLE:
		case WP_GRENADE_LAUNCHER:
		case WP_MOLOTOV:
		case WP_SATCHEL:
		case WP_SMOKE_BOMB:
		case WP_POISON_GAS:
			VectorMA( muzzlePoint, 20, right, muzzlePoint );
			break;
		case WP_AKIMBO_COLT:
		case WP_AKIMBO_SILENCEDCOLT:
		case WP_AKIMBO_LUGER:
		case WP_AKIMBO_SILENCEDLUGER:
			VectorMA( muzzlePoint, -6, right, muzzlePoint );
			VectorMA( muzzlePoint, -4, up, muzzlePoint );
			break;
		default:
			VectorMA( muzzlePoint, 6, right, muzzlePoint );
			VectorMA( muzzlePoint, -4, up, muzzlePoint );
			break;
	}

	// done.

	// (SA) actually, this is sort of moot right now since
	// you're not allowed to fire when leaning.  Leave in
	// in case we decide to enable some lean-firing.
	// (SA) works with gl now
	//AddLean(ent, muzzlePoint);

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}

// Rafael - for activate
void CalcMuzzlePointForActivate ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	// Jaybird - adjust for lower bbox
	if (ent->client->ps.viewangles[PITCH] > 30 && (ent->client->ps.eFlags & EF_CROUCHING))
		muzzlePoint[2] += 30;
	else
		muzzlePoint[2] += ent->client->ps.viewheight;

	AddLean(ent, muzzlePoint);

	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}
// done.

// Ridah
void CalcMuzzlePoints(gentity_t *ent, int weapon) {
	vec3_t	viewang;

	VectorCopy(ent->client->ps.viewangles, viewang);

	{	// non ai's take into account scoped weapon 'sway' (just another way aimspread is visualized/utilized)
		float spreadfrac, phase;

		if(BG_IsScopedWeapon(weapon)) {
			float pitchMinAmp, yawMinAmp;
            int basetime;

			spreadfrac = ent->client->currentAimSpreadScale;

			if( weapon == WP_FG42SCOPE ) {
				pitchMinAmp = 4*ZOOM_PITCH_MIN_AMPLITUDE;
				yawMinAmp = 4*ZOOM_YAW_MIN_AMPLITUDE;
			} else {
				pitchMinAmp = ZOOM_PITCH_MIN_AMPLITUDE;
				yawMinAmp = ZOOM_YAW_MIN_AMPLITUDE;
			}

            basetime = ent->client->pers.cmd.serverTime;

			// rotate 'forward' vector by the sway
			phase = basetime / 1000.0 * ZOOM_PITCH_FREQUENCY * M_PI * 2;
			viewang[PITCH] += ZOOM_PITCH_AMPLITUDE * sin( phase ) * (spreadfrac+pitchMinAmp);

			phase = basetime / 1000.0 * ZOOM_YAW_FREQUENCY * M_PI * 2;
			viewang[YAW] += ZOOM_YAW_AMPLITUDE * sin( phase ) * (spreadfrac+yawMinAmp);
		}
	}


	// set aiming directions
	AngleVectors (viewang, __forward, __right, __up);

//----(SA)	modified the muzzle stuff so that weapons that need to fire down a perfect trace
//			straight out of the camera (SP5, Mauser right now) can have that accuracy, but
//			weapons that need an offset effect (bazooka/grenade/etc.) can still look like
//			they came out of the weap.
	CalcMuzzlePointForActivate( ent, __forward, __right, __up, __muzzleTrace );
	CalcMuzzlePoint ( ent, weapon, __forward, __right, __up, __muzzleEffect );
}

qboolean G_PlayerCanBeSeenByOthers( gentity_t *ent ) {
	int			i;
	gentity_t	*ent2;
	vec3_t		pos[3];

	VectorCopy( ent->client->ps.origin, pos[0] );
	pos[0][2] += ent->client->ps.mins[2];
	VectorCopy( ent->client->ps.origin, pos[1] );
	VectorCopy( ent->client->ps.origin, pos[2] );
	pos[2][2] += ent->client->ps.maxs[2];

	for( i = 0, ent2 = g_entities; i < level.maxclients; i++, ent2++ ) {
		if( !ent2->inuse || ent2 == ent ) {
			continue;
		}

		if( ent2->client->sess.sessionTeam == TEAM_SPECTATOR )
			continue;

		if( ent2->health <= 0 ||
			ent2->client->sess.sessionTeam == ent->client->sess.sessionTeam ) {
			continue;
		}

		if( ent2->client->ps.eFlags & EF_ZOOMING ) {
			G_SetupFrustum_ForBinoculars( ent2 );
		} else {
			G_SetupFrustum( ent2 );
		}

		if( G_VisibleFromBinoculars( ent2, ent, pos[0] ) ||
			G_VisibleFromBinoculars( ent2, ent, pos[1] ) ||
			G_VisibleFromBinoculars( ent2, ent, pos[2] ) ) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {
	float	aimSpreadScale;
	int		shots = 1;
	gentity_t *pFiredShot = 0; // Omni-bot To tell bots about projectiles
	qboolean callEvent = qtrue;
	
	// ydnar: dead guys don't fire guns
	if( ent->client->ps.pm_type == PM_DEAD )
		return;
	
    // Playdead players do not fire guns either
    if (ent->client->ps.eFlags & EF_PLAYDEAD)
        return;

	// Rafael mg42
	if (ent->client->ps.persistant[PERS_HWEAPON_USE] && ent->active) {
		return;
	}

	// Ridah, need to call this for AI prediction also
	CalcMuzzlePoints(ent, ent->s.weapon);

	aimSpreadScale = ent->client->currentAimSpreadScale;
	// Ridah, add accuracy factor for AI
	aimSpreadScale+= 0.15f;	// (SA) just adding a temp /maximum/ accuracy for player (this will be re-visited in greater detail :)
	if(aimSpreadScale > 1)
		aimSpreadScale = 1.0f;	// still cap at 1.0

	if( (ent->client->ps.eFlags & EF_ZOOMING) && (ent->client->ps.stats[STAT_KEYS] & (1 << INV_BINOCS)) ) {
		if( ent->client->sess.playerType == PC_FIELDOPS) {
			if( !(ent->client->ps.leanf) ) {
				Weapon_Artillery(ent);
			}
			return;
		}
	}

	if( ent->client->ps.groundEntityNum == ENTITYNUM_NONE ) {
		aimSpreadScale = 2.0f;
	}

	/* Covertops disguise handling. Purpose of this code block is to
	 * lose disguise under certain weapons.
	 */
	if( ent->client->ps.powerups[PW_OPS_DISGUISED]) {
		switch (ent->s.weapon) {
			case WP_ADRENALINE_SHARE:
			case WP_BINOCULARS: // CHRUKER: b054 - Binoculars are also a stealthy tool
			case WP_MEDIC_ADRENALINE: // Jaybird - should be able to adrenaline as well.
			case WP_MOLOTOV: // mike - funny to not lose disguise with this
			case WP_SATCHEL:
			case WP_SATCHEL_DET:
			case WP_SMOKE_BOMB:
			case WP_POISON_GAS:
				break;

			case WP_AKIMBO_SILENCEDCOLT:
			case WP_AKIMBO_SILENCEDLUGER:
			case WP_GARAND:
			case WP_GARAND_SCOPE:
			case WP_GRENADE_LAUNCHER:
			case WP_GRENADE_PINEAPPLE:
			case WP_K43:
			case WP_K43_SCOPE:
			case WP_SILENCED_COLT:
			case WP_SILENCER:
			case WP_STEN:
				if (G_PlayerCanBeSeenByOthers( ent ))
					ent->client->ps.powerups[PW_OPS_DISGUISED] = 0;
				break;

			case WP_MEDIC_SYRINGE:
			case WP_MEDKIT:
	   			if (!(cvars::bg_covertops.ivalue & COPS_MEDKIT))
					ent->client->ps.powerups[PW_OPS_DISGUISED] = 0;
				break;

			case WP_LANDMINE:
			case WP_LANDMINE_BBETTY:
			case WP_LANDMINE_PGAS:
			case WP_PLIERS:
				if (!(cvars::bg_covertops.ivalue & COPS_MINES))
					ent->client->ps.powerups[PW_OPS_DISGUISED] = 0;
				break;

			case WP_AMMO:
				if (!(cvars::bg_covertops.ivalue & COPS_AMMOPACK))
					ent->client->ps.powerups[PW_OPS_DISGUISED] = 0;
				break;

			default:
				ent->client->ps.powerups[PW_OPS_DISGUISED] = 0;
				break;
		}
    }

	// fire the specific weapon
	switch( ent->s.weapon ) {
	case WP_KNIFE:
		Weapon_Knife( ent );
		break;
	// NERVE - SMF
	case WP_MEDKIT:
		callEvent = qfalse;
		Weapon_Medic( ent );
		break;
	case WP_PLIERS:
		Weapon_Engineer( ent );
		break;

	case WP_SMOKE_MARKER:
		if( level.time - ent->client->ps.classWeaponTime > level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] ) {
			ent->client->ps.classWeaponTime = level.time - level.lieutenantChargeTime[ent->client->sess.sessionTeam-1];
		}

		if (ent->client->sess.skill[SK_SIGNALS] >= 5 && (cvars::bg_sk5_fdops.ivalue & SK5_FDO_CHARGE))
			ent->client->ps.classWeaponTime += int( .66f * level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
		else if (ent->client->sess.skill[SK_SIGNALS] >= 2)
			ent->client->ps.classWeaponTime += int( .66f * level.lieutenantChargeTime[ent->client->sess.sessionTeam-1] );
		else
			ent->client->ps.classWeaponTime = level.time;

		pFiredShot = weapon_grenadelauncher_fire(ent, WP_SMOKE_MARKER);
		break;
	// -NERVE - SMF
	case WP_MEDIC_SYRINGE:
		Weapon_Syringe(ent);
		break;
	case WP_MEDIC_ADRENALINE:
		ent->client->ps.classWeaponTime = level.time;
		Weapon_AdrenalineSyringe(ent);
		break;
	case WP_ADRENALINE_SHARE:
		Weapon_AdrenalineShare(ent);
		break;
	case WP_POISON_SYRINGE:
		//ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_POISON_SYRINGE)] += 1;
		Weapon_PoisonSyringe(ent);
		break;
	case WP_AMMO:
		callEvent = qfalse;
		Weapon_MagicAmmo( ent );
		break;
	case WP_LUGER:
		Bullet_Fire( ent, LUGER_SPREAD*aimSpreadScale, LUGER_DAMAGE, qtrue );
		break;
	case WP_SILENCER:
		Bullet_Fire( ent, SILENCER_SPREAD*aimSpreadScale, SILENCER_DAMAGE, qtrue );
		break;
	case WP_AKIMBO_LUGER:
		Bullet_Fire( ent, AKIMBO_LUGER_SPREAD*aimSpreadScale, AKIMBO_LUGER_DAMAGE, qtrue );
		break;
	case WP_AKIMBO_SILENCEDLUGER:
		Bullet_Fire( ent, AKIMBO_SILENCEDLUGER_SPREAD*aimSpreadScale, AKIMBO_SILENCEDLUGER_DAMAGE, qtrue );
		break;
	case WP_COLT:
		Bullet_Fire( ent, COLT_SPREAD*aimSpreadScale, COLT_DAMAGE, qtrue );
		break;
	case WP_SILENCED_COLT:
		Bullet_Fire( ent, SILENCED_COLT_SPREAD*aimSpreadScale, SILENCED_COLT_DAMAGE, qtrue );
		break;
	case WP_AKIMBO_COLT:
		Bullet_Fire( ent, AKIMBO_COLT_SPREAD*aimSpreadScale, AKIMBO_COLT_DAMAGE, qtrue );
		break;
	case WP_AKIMBO_SILENCEDCOLT:
		Bullet_Fire( ent, AKIMBO_SILENCEDCOLT_SPREAD*aimSpreadScale, AKIMBO_SILENCEDCOLT_DAMAGE, qtrue );
		break;
	case WP_KAR98:
		aimSpreadScale = 1.0f;
		Bullet_Fire( ent, KAR98_SPREAD*aimSpreadScale, KAR98_DAMAGE, qfalse );
		break;
	case WP_CARBINE:
		aimSpreadScale = 1.0f;
		Bullet_Fire( ent, CARBINE_SPREAD*aimSpreadScale, CARBINE_DAMAGE, qfalse );
		break;
	case WP_FG42SCOPE:
		Bullet_Fire( ent, FG42SCOPE_SPREAD*aimSpreadScale, FG42SCOPE_DAMAGE, qfalse );
		break;
	case WP_FG42:
		Bullet_Fire( ent, FG42_SPREAD*aimSpreadScale, FG42_DAMAGE, qtrue );
		break;
	case WP_GARAND_SCOPE:
		Bullet_Fire( ent, GARANDSCOPE_SPREAD*aimSpreadScale, GARANDSCOPE_DAMAGE, qfalse );
		break;
	case WP_GARAND:
		aimSpreadScale = 1.0f;
		Bullet_Fire( ent, GARAND_SPREAD*aimSpreadScale, GARAND_DAMAGE, qfalse );
		break;
	case WP_SATCHEL_DET:
		if( G_ExplodeSatchels( ent ) ) {
			ent->client->ps.ammo[WP_SATCHEL_DET] = 0;
			ent->client->ps.ammoclip[WP_SATCHEL_DET] = 0;
			ent->client->ps.ammoclip[WP_SATCHEL] = 1;
			G_AddEvent( ent, EV_NOAMMO, 0 );
		}
		break;
	case WP_TRIPMINE:
		G_PlaceTripmine(ent);
		break;

	case WP_MOBILE_MG42_SET:
		Bullet_Fire( ent, MOBILE_MG42_SPREAD*0.05f*aimSpreadScale, MOBILE_MG42_DAMAGE, qfalse );
		break;
		
	case WP_MOBILE_MG42:
		if( ent->client->ps.pm_flags & PMF_DUCKED || ent->client->ps.eFlags & EF_PRONE ) {
			Bullet_Fire( ent, MOBILE_MG42_SPREAD*0.6f*aimSpreadScale, MOBILE_MG42_DAMAGE, qfalse );
		} else {
			Bullet_Fire( ent, MOBILE_MG42_SPREAD*aimSpreadScale, MOBILE_MG42_DAMAGE, qfalse );
		}
		break;
	case WP_K43_SCOPE:
		Bullet_Fire( ent, K43SCOPE_SPREAD*aimSpreadScale, K43SCOPE_DAMAGE, qfalse );
		break;
	case WP_K43:
		aimSpreadScale = 1.0;
		Bullet_Fire( ent, K43_SPREAD*aimSpreadScale, K43_DAMAGE, qfalse );
		break;
	case WP_STEN:
		Bullet_Fire( ent, STEN_SPREAD*aimSpreadScale, STEN_DAMAGE, qtrue );
		break;
	case WP_MP40:
		Bullet_Fire( ent, MP40_SPREAD*aimSpreadScale, MP40_DAMAGE, qtrue );
		break;
	case WP_THOMPSON:
		Bullet_Fire( ent, THOMPSON_SPREAD*aimSpreadScale, THOMPSON_DAMAGE, qtrue );
		break;
	case WP_M97:
		Weapon_M97( ent );
		break;
	case WP_PANZERFAUST:
		if( level.time - ent->client->ps.classWeaponTime > level.soldierChargeTime[ent->client->sess.sessionTeam-1] ) {
			ent->client->ps.classWeaponTime = level.time - level.soldierChargeTime[ent->client->sess.sessionTeam-1];
		}

		if (ent->client->sess.skill[SK_HEAVY_WEAPONS] >= 1 && (cvars::bg_sk5_soldier.ivalue & SK5_SOL_CHARGE))
			ent->client->ps.classWeaponTime += int( .66f * level.soldierChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
		else if (ent->client->sess.skill[SK_HEAVY_WEAPONS] >= 1)
			ent->client->ps.classWeaponTime += int( .66f * level.soldierChargeTime[ent->client->sess.sessionTeam-1] );
		else
			ent->client->ps.classWeaponTime = level.time;

		pFiredShot = Weapon_Panzerfaust_Fire(ent);
		if( ent->client ) {
			vec3_t forward;
			AngleVectors (ent->client->ps.viewangles, forward, NULL, NULL);
			VectorMA (ent->client->ps.velocity, -64, forward, ent->client->ps.velocity);
		}
		break;
	case WP_GPG40:
	case WP_M7:
		if( level.time - ent->client->ps.classWeaponTime > level.engineerChargeTime[ent->client->sess.sessionTeam-1] ) {
			ent->client->ps.classWeaponTime = level.time - level.engineerChargeTime[ent->client->sess.sessionTeam-1];
		}

		if (ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 5 && (cvars::bg_sk5_eng.ivalue & SK5_ENG_CHARGE))
			ent->client->ps.classWeaponTime += int( .5f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
		else
			ent->client->ps.classWeaponTime += int( .5f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );

		pFiredShot = weapon_gpg40_fire( ent, ent->s.weapon );
		break;
	case WP_MORTAR_SET:
		if( level.time - ent->client->ps.classWeaponTime > level.soldierChargeTime[ent->client->sess.sessionTeam-1] ) {
			ent->client->ps.classWeaponTime = level.time - level.soldierChargeTime[ent->client->sess.sessionTeam-1];
		}

		if (ent->client->sess.skill[SK_HEAVY_WEAPONS] >= 5 && (cvars::bg_sk5_soldier.ivalue & SK5_SOL_CHARGE)) {
			ent->client->ps.classWeaponTime += int( .33f * level.soldierChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
		}
		else if (ent->client->sess.skill[SK_HEAVY_WEAPONS] >= 1) {
			// CHRUKER: b068 - Was using "0.5f*(1-0.3f)", however the 0.33f is used everywhere else, and is more precise
			ent->client->ps.classWeaponTime += int( .33f * level.soldierChargeTime[ent->client->sess.sessionTeam-1] );
        }
		else {
			ent->client->ps.classWeaponTime += int( .5f * level.soldierChargeTime[ent->client->sess.sessionTeam-1] );
		}

		pFiredShot = weapon_mortar_fire( ent, ent->s.weapon );
		break;

	case WP_SATCHEL:
	case WP_SMOKE_BOMB:
	case WP_POISON_GAS:
		if( level.time - ent->client->ps.classWeaponTime > level.covertopsChargeTime[ent->client->sess.sessionTeam-1] )
			ent->client->ps.classWeaponTime = level.time - level.covertopsChargeTime[ent->client->sess.sessionTeam-1];

		if (ent->client->sess.skill[SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS] >= 5 && (cvars::bg_sk5_cvops.ivalue & SK5_CVO_CHARGE))
			ent->client->ps.classWeaponTime += int( .66f * level.covertopsChargeTime[ent->client->sess.sessionTeam-1] * SK5G_CHARGE_FACTOR );
		else if (ent->client->sess.skill[SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS] >= 2)
			ent->client->ps.classWeaponTime += int( .66f * level.covertopsChargeTime[ent->client->sess.sessionTeam-1] );
		else
			ent->client->ps.classWeaponTime = level.time;

		pFiredShot = weapon_grenadelauncher_fire( ent, ent->s.weapon );
		break;

	case WP_GRENADE_LAUNCHER:
	case WP_GRENADE_PINEAPPLE:
		pFiredShot = weapon_grenadelauncher_fire( ent, ent->s.weapon );
		break;

	case WP_MOLOTOV:
		pFiredShot = molotov::launch( *ent, 400 );
		break;

	case WP_LANDMINE:
	case WP_LANDMINE_BBETTY:
	case WP_LANDMINE_PGAS:
		if( level.time - ent->client->ps.classWeaponTime > level.engineerChargeTime[ent->client->sess.sessionTeam-1] )
			ent->client->ps.classWeaponTime = level.time - level.engineerChargeTime[ent->client->sess.sessionTeam-1];

		if( ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 3 )
			ent->client->ps.classWeaponTime += int( .33f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );
		else
			ent->client->ps.classWeaponTime += int( .5f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );

		pFiredShot = weapon_grenadelauncher_fire( ent, ent->s.weapon );
		break;

	case WP_DYNAMITE:
		if( level.time - ent->client->ps.classWeaponTime > level.engineerChargeTime[ent->client->sess.sessionTeam-1] )
			ent->client->ps.classWeaponTime = level.time - level.engineerChargeTime[ent->client->sess.sessionTeam-1];

		if( ent->client->sess.skill[SK_EXPLOSIVES_AND_CONSTRUCTION] >= 3 )
			ent->client->ps.classWeaponTime += int( .66f * level.engineerChargeTime[ent->client->sess.sessionTeam-1] );
		else
			ent->client->ps.classWeaponTime = level.time;

		pFiredShot = weapon_grenadelauncher_fire( ent, ent->s.weapon );
		break;

	case WP_FLAMETHROWER:
		// RF, this is done client-side only now
		// Gordon: um, no it isnt?
		pFiredShot = Weapon_FlamethrowerFire( ent );
		break;
	case WP_MAPMORTAR:
		break;
	default:
		break;
	}

	// Omni-bot - Send a fire event.
	if(callEvent)
		Bot_Event_FireWeapon(ent-g_entities, Bot_WeaponGameToBot(ent->s.weapon), pFiredShot);

	// OSP
#ifndef DEBUG_STATS
	if(cvars::gameState.ivalue == GS_PLAYING)
#endif
	ent->client->sess.aWeaponStats[BG_WeapStatForWeapon( (weapon_t)ent->s.weapon )].atts += shots;
}


//
// IsSilencedWeapon
//
// Description: Is the specified weapon a silenced weapon?
// Written: 12/26/2002
//
qboolean IsSilencedWeapon
(
	// The type of weapon in question.  Is it silenced?
	int weaponType
)
{
	// Local Variables ////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	// Return true for any of the silenced types
	switch (weaponType)
	{
	case WP_SILENCED_COLT:
	case WP_STEN:
	case WP_SILENCER:
		return qtrue;
	};

	// Otherwise, not silenced
	return qfalse;
}
//
// IsSilencedWeapon
//

///////////////////////////////////////////////////////////////////////////////

gentity_t*
G_ThrowingKnife_launch( gentity_t& actor, int scalarvel )
{
    vec3_t  velocity;
    vec3_t  offset;
    vec3_t  org;
    vec3_t  mins;
    vec3_t  maxs;
    trace_t tr;

    // Do nothing if dead.
    if (actor.client->ps.pm_type == PM_DEAD || actor.health <= 0) 
        return NULL;

    // Do nothing if playing dead.
    if (actor.client->ps.eFlags & EF_PLAYDEAD)
        return NULL;

    // Do nothing if using MG42.
    if (actor.client->ps.persistant[PERS_HWEAPON_USE] && actor.active)
        return NULL;

    AngleVectors( actor.client->ps.viewangles, velocity, NULL, NULL );
    VectorScale( velocity, 34, offset );
    offset[2] += actor.client->ps.viewheight - 5; // a little lower than view
    VectorScale( velocity, scalarvel, velocity );
    velocity[2] += 25 + rand()%35;

    VectorAdd( actor.s.pos.trDelta, velocity, velocity);
    VectorAdd( actor.client->ps.origin, offset, org);

    VectorSet( mins, -12, -12, 0 );
    VectorSet( maxs, 12, 12, 12 );
    trap_Trace( &tr, actor.client->ps.origin, mins, maxs, org, actor.s.number, MASK_SOLID );

    VectorCopy( tr.endpos, org );
    gentity_t& knife = *fire_grenade( &actor, org, velocity, WP_KNIFE );

	// Increase stats
	actor.client->sess.aWeaponStats[WS_THROWING_KNIFE].atts += 1;

    // Lose uniform if can be seen
    if (actor.client->ps.powerups[PW_OPS_DISGUISED] && G_PlayerCanBeSeenByOthers( &actor ))
        actor.client->ps.powerups[PW_OPS_DISGUISED] = 0;

    return &knife;
}

///////////////////////////////////////////////////////////////////////////////

int G_DistanceFalloffAdjustment(int damage, float distance, float falloff)
{
    // zinx - start at 100% at 1500 units (and before),
    // and go to falloff at 2500 units (and after)

    // 1500 to 2500 -> 0.0 to 1.0
    float scale = (distance - 1500.f) / (2500.f - 1500.f);
    // 0.0 to 1.0 -> 0.0 to (1 - falloff)
    scale *= (1 - falloff);
    // 0.0 to (1 - falloff) -> 1.0 to falloff
    scale = 1.0f - scale;

    // And, finally, cap it.
    if (scale >= 1.0f) {
        scale = 1.0f;
    }
    else if (scale < falloff) {
        scale = falloff;
    }

    return int( damage * scale );
}
