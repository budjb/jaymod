#include <bgame/impl.h>

void G_LogDeath( gentity_t* ent, weapon_t weap ) {
	weap = BG_DuplicateWeapon(weap);

	if(!ent->client) {
		return;
	}

	ent->client->pers.playerStats.weaponStats[weap].killedby++;

	trap_PbStat ( ent - g_entities , "death" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , weap ) ) ;
}

void G_LogKill( gentity_t* ent, weapon_t weap ) {
	weap = BG_DuplicateWeapon(weap);

	if(!ent->client) {
		return;
	}

	ent->client->pers.playerStats.weaponStats[weap].kills++;

	trap_PbStat ( ent - g_entities , "kill" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , weap ) ) ;
}

void G_LogTeamKill( gentity_t* ent, weapon_t weap ) {
	weap = BG_DuplicateWeapon(weap);

	if(!ent->client) {
		return;
	}

	ent->client->pers.playerStats.weaponStats[weap].teamkills++;

	trap_PbStat ( ent - g_entities , "tk" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , weap ) ) ;
}

void G_LogRegionHit( gentity_t* ent, hitRegion_t hr ) {
	if(!ent->client) {
		return;
	}
	ent->client->pers.playerStats.hitRegions[hr]++;

	trap_PbStat ( ent - g_entities , "hr" , 
		va ( "%d %d %d" , ent->client->sess.sessionTeam , ent->client->sess.playerType , hr ) ) ;
}

void G_PrintAccuracyLog( gentity_t *ent ) {
	int i;
	char buffer[2048];

	Q_strncpyz(buffer, "WeaponStats", 2048);

	for( i = 0; i < WP_NUM_WEAPONS; i++ ) {
		if(!BG_ValidStatWeapon( (weapon_t)i )) {
			continue;
		}

		Q_strcat(buffer, 2048, va(" %i %i %i", 
			ent->client->pers.playerStats.weaponStats[i].kills, 
			ent->client->pers.playerStats.weaponStats[i].killedby,
			ent->client->pers.playerStats.weaponStats[i].teamkills ));
	}

	Q_strcat( buffer, 2048, va(" %i", ent->client->pers.playerStats.suicides));

	for( i = 0; i < HR_NUM_HITREGIONS; i++ ) {
		Q_strcat( buffer, 2048, va(" %i", ent->client->pers.playerStats.hitRegions[i]));
	}

	Q_strcat( buffer, 2048, va(" %i", 6/*level.numOidTriggers*/ ));

	for( i = 0; i < 6/*level.numOidTriggers*/; i++ ) {
		Q_strcat( buffer, 2048, va(" %i", ent->client->pers.playerStats.objectiveStats[i]));
		Q_strcat( buffer, 2048, va(" %i", ent->client->sess.sessionTeam == TEAM_AXIS ? level.objectiveStatsAxis[i] : level.objectiveStatsAllies[i]));
	}

	trap_SendServerCommand( ent-g_entities, buffer );
}

void G_SetPlayerScore( gclient_t *client ) {
	int i;

	for( client->ps.persistant[PERS_SCORE] = 0, i = 0; i < SK_NUM_SKILLS; i++ ) {
		client->ps.persistant[PERS_SCORE] += int( client->sess.skillpoints[i] );
	}
}

void G_SetPlayerSkill( gclient_t *client, skillType_t skill ) {
	int i;

	for( i = NUM_SKILL_LEVELS - 1; i >= 0; i-- ) {
		// Jaybird - Custom levels
		if( skillLevels[skill][i] != -1 && client->sess.skillpoints[skill] >= skillLevels[skill][i] ) {
			client->sess.skill[skill] = i;
			break;
		}
	}

	G_SetPlayerScore( client );
}

extern qboolean AddWeaponToPlayer( gclient_t *client, weapon_t weapon, int ammo, int ammoclip, qboolean setcurrent );

// TAT 11/6/2002
//		Local func to actual do skill upgrade, used by both MP skill system, and SP scripted skill system
static void G_UpgradeSkill( gentity_t *ent, skillType_t skill ) {
	int i, cnt = 0;

	// See if this is the first time we've reached this skill level
	for( i = 0; i < SK_NUM_SKILLS; i++ ) {
		if( i == skill )
			continue;

		if( ent->client->sess.skill[skill] <= ent->client->sess.skill[i] )
			break;
	}

	G_DebugAddSkillLevel( ent, skill );

	if( i == SK_NUM_SKILLS ) {
		// increase rank
		ent->client->sess.rank++;
	}

	if( ent->client->sess.rank >=4 ) {
		// Gordon: count the number of maxed out skills
		for( i = 0; i < SK_NUM_SKILLS; i++ ) {
			if( ent->client->sess.skill[ i ] >= 4 ) {
				cnt++;
			}
		}

		ent->client->sess.rank = cnt + 3;
		if( ent->client->sess.rank > 10 ) {
			ent->client->sess.rank = 10;
		}
	}

	ClientUserinfoChanged( ent-g_entities );

	// Give em rightaway
	// Jaybird - Upgrade some skills IMMEDIATELY
	// Added akimbo upgrades immediately, and level 4 soldier thompson
	if( skill == SK_BATTLE_SENSE && ent->client->sess.skill[skill] == 1 ) {
		if( AddWeaponToPlayer( ent->client, WP_BINOCULARS, 1, 0, qfalse ) ) {
			ent->client->ps.stats[STAT_KEYS] |= ( 1 << INV_BINOCS );
		}
	} else if( skill == SK_FIRST_AID && !(cvars::bg_weapons.ivalue & SBW_NOADRENALINE) && ent->client->sess.playerType == PC_MEDIC && ent->client->sess.skill[skill] == 4 ) {
		AddWeaponToPlayer( ent->client, WP_MEDIC_ADRENALINE, ent->client->ps.ammo[BG_FindAmmoForWeapon(WP_MEDIC_ADRENALINE)], ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_MEDIC_ADRENALINE)], qfalse );
		if (g_medics.integer & MEDIC_SHAREADRENALINE) {
			AddWeaponToPlayer( ent->client, WP_ADRENALINE_SHARE, ent->client->ps.ammo[BG_FindAmmoForWeapon(WP_ADRENALINE_SHARE)], ent->client->ps.ammoclip[BG_FindClipForWeapon(WP_ADRENALINE_SHARE)], qfalse );
		}
	} else if( skill == SK_LIGHT_WEAPONS && ent->client->sess.skill[skill] == 4 && !cvars::bg_panzerWar.ivalue && !cvars::bg_sniperWar.ivalue) {
		if(ent->client->sess.playerType == PC_COVERTOPS) {
			if(ent->client->sess.sessionTeam == TEAM_AXIS) {
				COM_BitClear(ent->client->ps.weapons, WP_LUGER);
				AddWeaponToPlayer( ent->client, WP_AKIMBO_SILENCEDLUGER, GetAmmoTableData(WP_AKIMBO_SILENCEDLUGER)->defaultStartingAmmo, GetAmmoTableData(WP_AKIMBO_SILENCEDLUGER)->defaultStartingClip, qfalse );
				ent->client->sess.latchPlayerWeapon2 = WP_AKIMBO_SILENCEDLUGER;
			}
			if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
				COM_BitClear(ent->client->ps.weapons, WP_COLT);
				AddWeaponToPlayer( ent->client, WP_AKIMBO_SILENCEDCOLT, GetAmmoTableData(WP_AKIMBO_SILENCEDCOLT)->defaultStartingAmmo, GetAmmoTableData(WP_AKIMBO_SILENCEDCOLT)->defaultStartingClip, qfalse );
				ent->client->sess.latchPlayerWeapon2 = WP_AKIMBO_SILENCEDCOLT;
			}
		}
		else {
			if(ent->client->sess.sessionTeam == TEAM_AXIS) {
				COM_BitClear(ent->client->ps.weapons, WP_LUGER);
				AddWeaponToPlayer( ent->client, WP_AKIMBO_LUGER, GetAmmoTableData(WP_AKIMBO_LUGER)->defaultStartingAmmo, GetAmmoTableData(WP_AKIMBO_LUGER)->defaultStartingClip, qfalse );
				ent->client->sess.latchPlayerWeapon2 = WP_AKIMBO_LUGER;
			}
			if(ent->client->sess.sessionTeam == TEAM_ALLIES) {
				COM_BitClear(ent->client->ps.weapons, WP_COLT);
				AddWeaponToPlayer( ent->client, WP_AKIMBO_COLT, GetAmmoTableData(WP_AKIMBO_COLT)->defaultStartingAmmo, GetAmmoTableData(WP_AKIMBO_COLT)->defaultStartingClip, qfalse );
				ent->client->sess.latchPlayerWeapon2 = WP_AKIMBO_COLT;
 			}
 		}
		ClientUserinfoChanged(ent-g_entities);
	} else if( skill == SK_HEAVY_WEAPONS && ent->client->sess.playerType == PC_SOLDIER && ent->client->sess.skill[skill] == 4 && !cvars::bg_panzerWar.ivalue && !cvars::bg_sniperWar.ivalue) {
		if (ent->client->sess.sessionTeam == TEAM_AXIS) {
			COM_BitClear(ent->client->ps.weapons, WP_LUGER);
			COM_BitClear(ent->client->ps.weapons, WP_AKIMBO_LUGER);
			AddWeaponToPlayer( ent->client, WP_MP40, GetAmmoTableData(WP_MP40)->defaultStartingAmmo, GetAmmoTableData(WP_MP40)->defaultStartingClip, qfalse );
			ent->client->sess.latchPlayerWeapon2 = WP_MP40;
		}
		if (ent->client->sess.sessionTeam == TEAM_ALLIES) {
			COM_BitClear(ent->client->ps.weapons, WP_COLT);
			COM_BitClear(ent->client->ps.weapons, WP_AKIMBO_COLT);
			AddWeaponToPlayer( ent->client, WP_THOMPSON, GetAmmoTableData(WP_THOMPSON)->defaultStartingAmmo, GetAmmoTableData(WP_THOMPSON)->defaultStartingClip, qfalse );
			ent->client->sess.latchPlayerWeapon2 = WP_THOMPSON;
		}
		ClientUserinfoChanged(ent-g_entities);
	}
}

void G_LoseSkillPoints( gentity_t *ent, skillType_t skill, float points ) {
	int oldskill;
	float oldskillpoints;
	
	if( !ent->client ) {
		return;
	}

	// no skill loss during warmup
	if( cvars::gameState.ivalue != GS_PLAYING ) {
		return;
	}

	if( ent->client->sess.sessionTeam != TEAM_AXIS && ent->client->sess.sessionTeam != TEAM_ALLIES ) {
		return;
	}

	if( g_gametype.integer == GT_WOLF_LMS ) {
		return; // Gordon: no xp in LMS
	}

	oldskillpoints = ent->client->sess.skillpoints[skill];
	ent->client->sess.skillpoints[skill] -= points;

	// see if player increased in skill
	oldskill = ent->client->sess.skill[skill];
	G_SetPlayerSkill( ent->client, skill );
	// Jaybird - Custom levels
	if( skillLevels[skill][oldskill] != -1 && oldskill != ent->client->sess.skill[skill] ) {
		ent->client->sess.skill[skill] = oldskill;
		ent->client->sess.skillpoints[skill] = skillLevels[skill][oldskill];
	}

	// CHRUKER: b013 - Was printing this with many many decimals
	G_Printf( "%s just lost %.0f skill points for skill %s\n", ent->client->pers.netname, oldskillpoints - ent->client->sess.skillpoints[skill], skillNames[skill] );

	trap_PbStat ( ent - g_entities , "loseskill" , 
		va ( "%d %d %d %f" , ent->client->sess.sessionTeam , ent->client->sess.playerType , 
			skill , oldskillpoints - ent->client->sess.skillpoints[skill] ) ) ;

	level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] -= int( oldskillpoints - ent->client->sess.skillpoints[skill] );
	level.teamXP[ skill ][ ent->client->sess.sessionTeam - TEAM_AXIS ] -= oldskillpoints - ent->client->sess.skillpoints[skill];
}

void G_AddSkillPoints( gentity_t *ent, skillType_t skill, float points ) {
	int oldskill;
	float totalXP = 0;
	int i;

	if( !ent->client ) {
		return;
	}

    // no skill gaining during warmup
	if( cvars::gameState.ivalue != GS_PLAYING ) {
		return;
	}

	if( ent->client->sess.sessionTeam != TEAM_AXIS && ent->client->sess.sessionTeam != TEAM_ALLIES ) {
		return;
	}

	if( g_gametype.integer == GT_WOLF_LMS ) {
		return; // Gordon: no xp in LMS
	}

	// Jaybird - capped XP
	if( g_xpMax.integer > 0 ) {
		for( i = 0; i < SK_NUM_SKILLS; i++ ) {
			totalXP += ent->client->sess.skillpoints[i];
		}

		if( totalXP > g_xpMax.integer ) {
			if( g_xpCap.integer == XPCAP_ALLOWGAIN ) {
				if( ent->client->sess.skill[skill] >= NUM_SKILL_LEVELS - 1 ) {
					return;
				}
			}
			else if( g_xpCap.integer == XPCAP_STOPGAIN ) {
				return;
			}
			else if( g_xpCap.integer == XPCAP_RESETXP ) {
                g_clientObjects[ ent->s.number ].xpReset();
			}
		}
	}

	level.teamXP[ skill ][ ent->client->sess.sessionTeam - TEAM_AXIS ] += points;

	ent->client->sess.skillpoints[skill] += points;

	level.teamScores[ ent->client->ps.persistant[PERS_TEAM] ] += int( points );

	// CHRUKER: b013 - Was printing this with many many decimals
//	G_Printf( "%s just got %.0f skill points for skill %s\n", ent->client->pers.netname, points, skillNames[skill] );

	trap_PbStat ( ent - g_entities , "addskill" , 
		va ( "%d %d %d %f" , ent->client->sess.sessionTeam , ent->client->sess.playerType , 
			skill , points ) ) ;

	// see if player increased in skill
	oldskill = ent->client->sess.skill[skill];
	G_SetPlayerSkill( ent->client, skill );
	if( oldskill != ent->client->sess.skill[skill] ) {
		// TAT - call the new func that encapsulates the skill giving behavior
		G_UpgradeSkill( ent, skill );
	}
}

void G_LoseKillSkillPoints( gentity_t *tker, meansOfDeath_t mod, hitRegion_t hr, qboolean splash ) {
	// for evil tkers :E

	if( !tker->client ) {
		return;
	}

	switch( mod ) {
		// light weapons
		case MOD_KNIFE:
		case MOD_THROWING_KNIFE:
		case MOD_LUGER:
		case MOD_COLT:
		case MOD_MP40:
		case MOD_THOMPSON:
		case MOD_STEN:
		case MOD_GARAND:
		case MOD_SILENCER:
		case MOD_FG42:
//		case MOD_FG42SCOPE:
		case MOD_CARBINE:
		case MOD_KAR98:
		case MOD_SILENCED_COLT:
		case MOD_K43:
//bani - akimbo weapons lose score now as well
		case MOD_AKIMBO_COLT:
		case MOD_AKIMBO_LUGER:
		case MOD_AKIMBO_SILENCEDCOLT:
		case MOD_AKIMBO_SILENCEDLUGER:
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
//bani - airstrike marker kills
		case MOD_SMOKEGRENADE:
			G_LoseSkillPoints( tker, SK_LIGHT_WEAPONS, 3.f ); 
//			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, 2.f, "kill" );
			break;

		// scoped weapons
		case MOD_GARAND_SCOPE:
		case MOD_K43_SCOPE:
		case MOD_FG42SCOPE:
		case MOD_SATCHEL:
			G_LoseSkillPoints( tker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, 2.f, "legshot kill" );
			break;

		case MOD_MOBILE_MG42:
		case MOD_MACHINEGUN:
		case MOD_BROWNING:
		case MOD_MG42:
		case MOD_PANZERFAUST:
		case MOD_FLAMETHROWER:
		case MOD_MORTAR:
			G_LoseSkillPoints( tker, SK_HEAVY_WEAPONS, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, 3.f, "emplaced mg42 kill" );
			break;

		case MOD_DYNAMITE:
		case MOD_LANDMINE:
		case MOD_GPG40:
		case MOD_M7:
			G_LoseSkillPoints( tker, SK_EXPLOSIVES_AND_CONSTRUCTION, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, 4.f, "dynamite or landmine kill" );
			break;

		case MOD_ARTY:
		case MOD_AIRSTRIKE:
			G_LoseSkillPoints( tker, SK_SIGNALS, 3.f );
//			G_DebugAddSkillPoints( attacker, SK_SIGNALS, 4.f, "artillery kill" );
			break;

		case MOD_POISON_SYRINGE:
			G_LoseSkillPoints( tker, SK_FIRST_AID, 3.f);
			break;

		case MOD_POISON_GAS:
			G_LoseSkillPoints( tker, SK_FIRST_AID, 3.f);
			break;

		// no skills for anything else
		default:
			break;
	}
}

void G_AddKillSkillPoints( gentity_t *attacker, meansOfDeath_t mod, hitRegion_t hr, qboolean splash ) {
	qboolean damagexp = (qboolean)g_damagexp.integer;

	if( !attacker->client )
		return;

	switch( mod ) {
		// light weapons
		case MOD_KNIFE:
		case MOD_THROWING_KNIFE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, 3.f, "knife kill" ); 
			break;

		case MOD_LUGER:
		case MOD_COLT:
		case MOD_MP40:
		case MOD_THOMPSON:
		case MOD_STEN:
		case MOD_GARAND:
		case MOD_SILENCER:
		case MOD_FG42:
//		case MOD_FG42SCOPE:
		case MOD_CARBINE:
		case MOD_KAR98:
		case MOD_SILENCED_COLT:
		case MOD_K43:
		case MOD_AKIMBO_COLT:
		case MOD_AKIMBO_LUGER:
		case MOD_AKIMBO_SILENCEDCOLT:
		case MOD_AKIMBO_SILENCEDLUGER:
		case MOD_M97:
			switch( hr ) {
				case HR_HEAD:	G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 5.f ); G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 5.f, "headshot kill" ); break;
				case HR_ARMS:	G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f, "armshot kill" ); break;
				case HR_BODY:	G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f, "bodyshot kill" ); break;
				case HR_LEGS:	G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f, "legshot kill" );  break;
				default:		G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f, "kill" ); break;	// for weapons that don't have localized damage
			}
			break;

		// heavy weapons
		case MOD_MOBILE_MG42:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "mobile mg42 kill" ); 
			break;

		// scoped weapons
		case MOD_GARAND_SCOPE:
		case MOD_K43_SCOPE:
		case MOD_FG42SCOPE:
			switch( hr ) {
				case HR_HEAD:	G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 5.f ); G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 5.f, "headshot kill" ); break;
				case HR_ARMS:	G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 2.f, "armshot kill" ); break;
				case HR_BODY:	G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f, "bodyshot kill" ); break;
				case HR_LEGS:	G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 2.f, "legshot kill" ); break;
				default:		G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f ); G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f, "kill" ); break;	// for weapons that don't have localized damage
			}
			break;

		// misc weapons (individual handling)
		case MOD_SATCHEL:
			G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 5.f );
			G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 5.f, "satchel charge kill" );
			break;

		case MOD_MACHINEGUN:
		case MOD_BROWNING:
		case MOD_MG42:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "emplaced machinegun kill" );
			break;

		case MOD_PANZERFAUST:
			if( splash ) {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "panzerfaust splash damage kill" );
			} else {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "panzerfaust direct hit kill" );
			}
			break;
		case MOD_FLAMETHROWER:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "flamethrower kill" );
			break;
		case MOD_MORTAR:
			if( splash ) {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "mortar splash damage kill" );
			} else {
				G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f );
				G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, damagexp ? 1.f : 3.f, "mortar direct hit kill" );
			}
			break;
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
//bani - airstrike marker kills
		case MOD_SMOKEGRENADE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, damagexp ? 1.f : 3.f, "hand grenade kill" );
			break;
		case MOD_DYNAMITE:
		case MOD_LANDMINE:
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, damagexp ? 1.f : 4.f );
			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, damagexp ? 1.f : 4.f, "dynamite or landmine kill" );
			break;
		case MOD_ARTY:
			G_AddSkillPoints( attacker, SK_SIGNALS, damagexp ? 1.f : 4.f );
			G_DebugAddSkillPoints( attacker, SK_SIGNALS, damagexp ? 1.f : 4.f, "artillery kill" );
			break;
		case MOD_AIRSTRIKE:
			G_AddSkillPoints( attacker, SK_SIGNALS, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_SIGNALS, damagexp ? 1.f : 3.f, "airstrike kill" );
			break;
		case MOD_GPG40:
		case MOD_M7:
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, damagexp ? 1.f : 3.f, "rifle grenade kill" );
			break;

		case MOD_POISON_SYRINGE:
			G_AddSkillPoints( attacker, SK_BATTLE_SENSE, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_BATTLE_SENSE, damagexp ? 1.f : 3.f, "poison syringe kill" );
			break;

		case MOD_POISON_GAS:
			G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f );
			G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, damagexp ? 1.f : 3.f, "poison gas kill" );
			break;

		// no skills for anything else
		default:
			break;
	}
}

void G_AddKillSkillPointsForDestruction( gentity_t *attacker, meansOfDeath_t mod, g_constructible_stats_t *constructibleStats )
{
	switch( mod ) {
		case MOD_GRENADE_LAUNCHER:
		case MOD_GRENADE_PINEAPPLE:
			G_AddSkillPoints( attacker, SK_LIGHT_WEAPONS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_LIGHT_WEAPONS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_GPG40:
		case MOD_M7:
		case MOD_DYNAMITE:
		case MOD_LANDMINE:
			G_AddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_EXPLOSIVES_AND_CONSTRUCTION, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_PANZERFAUST:
		case MOD_MORTAR:
			G_AddSkillPoints( attacker, SK_HEAVY_WEAPONS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_HEAVY_WEAPONS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_ARTY:
		case MOD_AIRSTRIKE:
			G_AddSkillPoints( attacker, SK_SIGNALS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_SIGNALS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		case MOD_SATCHEL:
			G_AddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, constructibleStats->destructxpbonus );
			G_DebugAddSkillPoints( attacker, SK_MILITARY_INTELLIGENCE_AND_SCOPED_WEAPONS, constructibleStats->destructxpbonus, "destroying a constructible/explosive" ); 
			break;
		default:
			break;
	}
}

/////// SKILL DEBUGGING ///////
static fileHandle_t skillDebugLog = -1;

void G_DebugOpenSkillLog( void )
{
	vmCvar_t	mapname;
	qtime_t		ct;
	char		*s;

	if( g_debugSkills.integer < 2 )
		return;

	trap_Cvar_Register( &mapname, "mapname", "", CVAR_SERVERINFO | CVAR_ROM );

	trap_RealTime( &ct );

	if( trap_FS_FOpenFile( va( "skills-%d-%02d-%02d-%02d%02d%02d-%s.log",
								1900+ct.tm_year, ct.tm_mon+1,ct.tm_mday,
								ct.tm_hour, ct.tm_min, ct.tm_sec,
								mapname.string ), &skillDebugLog, FS_APPEND_SYNC ) < 0 )
		return;

	s = va( "%02d:%02d:%02d : Logfile opened.\n", ct.tm_hour, ct.tm_min, ct.tm_sec );

	trap_FS_Write( s, strlen( s ), skillDebugLog );
}

void G_DebugCloseSkillLog( void )
{
	qtime_t		ct;
	char		*s;

	if( skillDebugLog == -1 )
		return;

	trap_RealTime( &ct );

	s = va( "%02d:%02d:%02d : Logfile closed.\n", ct.tm_hour, ct.tm_min, ct.tm_sec );

	trap_FS_Write( s, strlen( s ), skillDebugLog );

	trap_FS_FCloseFile( skillDebugLog );
}

void G_DebugAddSkillLevel( gentity_t *ent, skillType_t skill )
{
	qtime_t		ct;

	if( !g_debugSkills.integer )
		return;
	// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
	trap_SendServerCommand( ent-g_entities, va( "sdbg \"^%c(SK: %2i XP: %.0f) %s: You raised your skill level to %i.\"\n",
								COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], ent->client->sess.skill[skill] ) );

	trap_RealTime( &ct );

	if( g_debugSkills.integer >= 2 && skillDebugLog != -1 ) {
		// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
		char *s = va( "%02d:%02d:%02d : ^%c(SK: %2i XP: %.0f) %s: %s raised in skill level to %i.\n",
			ct.tm_hour, ct.tm_min, ct.tm_sec,
			COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], ent->client->pers.netname, ent->client->sess.skill[skill] );
		trap_FS_Write( s, strlen( s ), skillDebugLog );
	}
}

void G_DebugAddSkillPoints( gentity_t *ent, skillType_t skill, float points, const char *reason )
{
	qtime_t		ct;

	if( !g_debugSkills.integer )
		return;

	// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
	trap_SendServerCommand( ent-g_entities, va( "sdbg \"^%c(SK: %2i XP: %.0f) %s: You gained %.0fXP, reason: %s.\"\n",
								COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], points, reason ) );

	trap_RealTime( &ct );

	if( g_debugSkills.integer >= 2 && skillDebugLog != -1 ) {
		// CHRUKER: b013 - Was printing the float with 6.2 as max. numbers
		char *s = va( "%02d:%02d:%02d : ^%c(SK: %2i XP: %.0f) %s: %s gained %.0fXP, reason: %s.\n",
			ct.tm_hour, ct.tm_min, ct.tm_sec,
			COLOR_RED + skill, ent->client->sess.skill[skill], ent->client->sess.skillpoints[skill], skillNames[skill], ent->client->pers.netname, points, reason );
		trap_FS_Write( s, strlen( s ), skillDebugLog );
	}
}

// CHRUKER: b017 - Added a check to make sure that the best result is larger than 0
#define CHECKSTAT1( XX )														\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( cl->XX <= 0 ) {														\
			continue;															\
		}																		\
		if( !best || cl->XX > best->XX ) {										\
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

#define CHECKSTATMIN( XX, YY )													\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || cl->XX > best->XX ) {										\
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best && best->XX >= YY ? best->pers.netname : "", best && best->XX >= YY ? best->sess.sessionTeam : -1 ) )

// CHRUKER: b017 - Moved the minimum skill requirement into a seperate if sentence
// Added a check to make sure only people who have increased their skills are considered
#define CHECKSTATSKILL( XX )															\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if ((cl->sess.skillpoints[XX] - cl->sess.startskillpoints[XX]) <= 0) {	\
			continue;															\
		}																		\
		if( cl->sess.skill[XX] < 1 ) {											\
			continue;															\
		}																		\
		if( !best || (cl->sess.skillpoints[XX] - cl->sess.startskillpoints[XX]) > (best->sess.skillpoints[XX] - best->sess.startskillpoints[XX]) ) { \
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

#define CHECKSTAT3( XX, YY, ZZ )												\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || cl->XX > best->XX ) {										\
			best = cl;															\
		} else if( cl->XX == best->XX && cl->YY > best->YY) {					\
			best = cl;															\
		} else if( cl->XX == best->XX && cl->YY == best->YY && cl->ZZ > best->ZZ) { \
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

#define CHECKSTATTIME( XX, YY )													\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || (cl->XX/(float)(level.time - cl->YY)) > (best->XX/(float)(level.time - best->YY)) ) { \
			best = cl;															\
		}																		\
	}																			\
	if( best ) {																\
		if( (best->sess.startxptotal - best->ps.persistant[PERS_SCORE]) >= 100 || best->medals || best->hasaward) {	\
			best = NULL;														\
		}																		\
	}																			\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

// Jaybird - save as above w/o restrictions
#define CHECKSTATTIME2( XX, YY )												\
	best = NULL;																\
	for( i = 0; i < level.numConnectedClients; i++ ) {							\
		gclient_t* cl = &level.clients[ level.sortedClients[ i ] ];				\
		if( cl->sess.sessionTeam == TEAM_SPECTATOR ) {							\
			continue;															\
		}																		\
		if( !best || (cl->XX/(float)(level.time - cl->YY)) > (best->XX/(float)(level.time - best->YY)) ) { \
			best = cl;															\
		}																		\
	}																			\
	if( best ) { best->hasaward = qtrue; }										\
	Q_strcat( buffer, 1024, va( ";%s; %i ", best ? best->pers.netname : "", best ? best->sess.sessionTeam : -1 ) )

void G_BuildEndgameStats( void ) {
	char buffer[1024];
	int i;
	gclient_t* best;

	G_CalcClientAccuracies();

	for( i = 0; i < level.numConnectedClients; i++ ) {
		level.clients[ i ].hasaward = qfalse;
	}

	*buffer = '\0';

	CHECKSTAT1( sess.kills );
	CHECKSTAT1( ps.persistant[PERS_SCORE] );
	CHECKSTAT3( sess.rank, medals, ps.persistant[PERS_SCORE] );
	CHECKSTAT1( medals );
	CHECKSTAT1( sess.revives );
	CHECKSTAT1( sess.headshots );
	CHECKSTATSKILL( SK_FIRST_AID );
	CHECKSTATSKILL( SK_SIGNALS );
	CHECKSTATSKILL( SK_LIGHT_WEAPONS );
	CHECKSTATSKILL( SK_HEAVY_WEAPONS );
	CHECKSTATTIME2( ps.persistant[PERS_SCORE], pers.enterTime );
	CHECKSTAT1( acc );
	CHECKSTATMIN( sess.team_kills, 5 );
	CHECKSTATTIME( ps.persistant[PERS_SCORE], pers.enterTime );

	trap_SetConfigstring( CS_ENDGAME_STATS, buffer );
}
