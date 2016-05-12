

// cg_predict.c -- this file generates cg.predictedPlayerState by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement.
// It also handles local physics interaction, like fragments bouncing off walls

#include <bgame/impl.h> 

/*static*/	pmove_t		cg_pmove;

static	int			cg_numSolidEntities;
static	centity_t	*cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static	int			cg_numSolidFTEntities;
static	centity_t	*cg_solidFTEntities[MAX_ENTITIES_IN_SNAPSHOT];
static	int			cg_numTriggerEntities;
static	centity_t	*cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

/*
====================
CG_BuildSolidList

When a new cg.snap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
void CG_BuildSolidList( void ) {
	int			i;
	centity_t	*cent;
	snapshot_t	*snap;
	entityState_t	*ent;

	cg_numSolidEntities = 0;
	cg_numSolidFTEntities = 0;
	cg_numTriggerEntities = 0;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

		// rain - don't clip against temporarily non-solid SOLID_BMODELS
		// (e.g. constructibles); use current state so prediction isn't fubar
		if( cent->currentState.solid == SOLID_BMODEL &&
			( cent->currentState.eFlags & EF_NONSOLID_BMODEL ) ) {
			continue;
		}

		if( ent->eType == ET_ITEM || 
			ent->eType == ET_PUSH_TRIGGER || 
			ent->eType == ET_TELEPORT_TRIGGER || 
			ent->eType == ET_CONCUSSIVE_TRIGGER || 
			ent->eType == ET_OID_TRIGGER 
#ifdef VISIBLE_TRIGGERS
			|| ent->eType == ET_TRIGGER_MULTIPLE
			|| ent->eType == ET_TRIGGER_FLAGONLY
			|| ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE
#endif // VISIBLE_TRIGGERS
			) {

			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if(	ent->eType == ET_CONSTRUCTIBLE ) {
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
		}

		if ( cent->nextState.solid) {
/*			if(cg_fastSolids.integer) { // Gordon: "optimization" (disabling until i fix it)
				vec3_t vec;
				float len;

				cg_solidFTEntities[cg_numSolidFTEntities] = cent;
				cg_numSolidFTEntities++;

				// FIXME: use range to bbox, not to origin
				if ( cent->nextState.solid == SOLID_BMODEL ) {
					VectorAdd( cgs.inlineModelMidpoints[ cent->currentState.modelindex ], cent->lerpOrigin, vec );
					VectorSubtract( vec, cg.predictedPlayerEntity.lerpOrigin, vec );
				} else {
					VectorSubtract( cent->lerpOrigin, cg.predictedPlayerEntity.lerpOrigin, vec );
				}
				if((len = DotProduct( vec, vec )) < (512 * 512)) {
					cg_solidEntities[cg_numSolidEntities] = cent;
					cg_numSolidEntities++;
					continue;
				}
			} else*/ {
				cg_solidEntities[cg_numSolidEntities] = cent;
				cg_numSolidEntities++;

				cg_solidFTEntities[cg_numSolidFTEntities] = cent;
				cg_numSolidFTEntities++;
				continue;
			}
		}
	}
}

/*
====================
CG_ClipMoveToEntities

====================
*/
static void CG_ClipMoveToEntities ( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
							int skipNumber, int mask, int capsule, trace_t *tr ) {
	int			i, x, zd, zu;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	vec3_t		bmins, bmaxs;
	vec3_t		origin, angles;
	centity_t	*cent;

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = trap_CM_InlineModel( ent->modelindex );
//			VectorCopy( cent->lerpAngles, angles );
//			VectorCopy( cent->lerpOrigin, origin );
			BG_EvaluateTrajectory( &cent->currentState.apos, cg.physicsTime, angles, qtrue, cent->currentState.effect2Time );
			BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin, qfalse, cent->currentState.effect2Time );
		} else {
			// Jaybird - EF_FAKEBMODEL
			// rain - #397 - fetch the real bbox for a fake bmodel
			if (ent->eType != ET_PLAYER && ent->eFlags & EF_FAKEBMODEL) {
				VectorCopy(ent->angles, bmins);
				VectorCopy(ent->angles2, bmaxs);
			} else {
				// encoded bbox
				x = (ent->solid & 255);
				zd = ((ent->solid>>8) & 255);
				zu = ((ent->solid>>16) & 255) - 32;

				bmins[0] = bmins[1] = -x;
				bmaxs[0] = bmaxs[1] = x;
				bmins[2] = -zd;
				bmaxs[2] = zu;
			}

			//cmodel = trap_CM_TempCapsuleModel( bmins, bmaxs );
			cmodel = trap_CM_TempBoxModel( bmins, bmaxs );

			VectorCopy( vec3_origin, angles );
			VectorCopy( cent->lerpOrigin, origin );
		}
		// MrE: use bbox of capsule
		if (capsule) {
			trap_CM_TransformedCapsuleTrace ( &trace, start, end,
				mins, maxs, cmodel,  mask, origin, angles);
		}
		else {
			trap_CM_TransformedBoxTrace ( &trace, start, end,
				mins, maxs, cmodel,  mask, origin, angles);
		}

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}

static void CG_ClipMoveToEntities_FT ( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask, int capsule, trace_t *tr ) {
	int			i, x, zd, zu;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	vec3_t		bmins, bmaxs;
	vec3_t		origin, angles;
	centity_t	*cent;

	for ( i = 0 ; i < cg_numSolidFTEntities ; i++ ) {
		cent = cg_solidFTEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = trap_CM_InlineModel( ent->modelindex );
//			VectorCopy( cent->lerpAngles, angles );
//			VectorCopy( cent->lerpOrigin, origin );
			BG_EvaluateTrajectory( &cent->currentState.apos, cg.physicsTime, angles, qtrue, cent->currentState.effect2Time );
			BG_EvaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin, qfalse, cent->currentState.effect2Time );
		} else {
			// Jaybird - EF_FAKEBMODEL
			// rain - #397 - fetch the real bbox for a fake bmodel
			if (ent->eType != ET_PLAYER && ent->eFlags & EF_FAKEBMODEL) {
				VectorCopy(ent->angles, bmins);
				VectorCopy(ent->angles2, bmaxs);
			} else {
				// encoded bbox
				x = (ent->solid & 255);
				zd = ((ent->solid>>8) & 255);
				zu = ((ent->solid>>16) & 255) - 32;

				bmins[0] = bmins[1] = -x;
				bmaxs[0] = bmaxs[1] = x;
				bmins[2] = -zd;
				bmaxs[2] = zu;
			}

			cmodel = trap_CM_TempCapsuleModel( bmins, bmaxs );

			VectorCopy( vec3_origin, angles );
			VectorCopy( cent->lerpOrigin, origin );
		}
		// MrE: use bbox of capsule
		if (capsule) {
			trap_CM_TransformedCapsuleTrace ( &trace, start, end,
				mins, maxs, cmodel,  mask, origin, angles);
		}
		else {
			trap_CM_TransformedBoxTrace ( &trace, start, end,
				mins, maxs, cmodel,  mask, origin, angles);
		}

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = qtrue;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}

/*
================
CG_Trace
================
*/
void	CG_Trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask ) {
	trace_t	t;

	trap_CM_BoxTrace ( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities (start, mins, maxs, end, skipNumber, mask, qfalse, &t);

	*result = t;
}

void	CG_Trace_World( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask ) {
	trace_t	t;

	trap_CM_BoxTrace ( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;

	*result = t;
}

void	CG_FTTrace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask ) {
	trace_t	t;


	trap_CM_BoxTrace ( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models

	CG_ClipMoveToEntities_FT (start, mins, maxs, end, skipNumber, mask, qfalse, &t);

	*result = t;
}


/*
================
CG_TraceCapsule
================
*/
void CG_TraceCapsule( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask ) {
	trace_t	t;

	trap_CM_CapsuleTrace ( &t, start, end, mins, maxs, 0, mask );
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities (start, mins, maxs, end, skipNumber, mask, qtrue, &t);

	*result = t;
}

void CG_TraceCapsule_World( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask ) {
	trace_t	t;
	trap_CM_CapsuleTrace ( &t, start, end, mins, maxs, 0, mask );

	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	*result = t;
}
/*
================
CG_PointContents
================
*/
int		CG_PointContents( const vec3_t point, int passEntityNum ) {
	int			i;
	entityState_t	*ent;
	centity_t	*cent;
	clipHandle_t cmodel;
	int			contents;

	contents = trap_CM_PointContents (point, 0);

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];

		ent = &cent->currentState;

		if ( ent->number == passEntityNum ) {
			continue;
		}

		if (ent->solid != SOLID_BMODEL) { // special value for bmodel
			continue;
		}

		cmodel = trap_CM_InlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		contents |= trap_CM_TransformedPointContents( point, cmodel, cent->lerpOrigin, cent->lerpAngles );
		// Gordon: again, need to use the projected water position to allow for moving entity based water.
//		contents |= trap_CM_TransformedPointContents( point, cmodel, ent->origin, ent->angles );
	}

	return contents;
}


/*
========================
CG_InterpolatePlayerState

Generates cg.predictedPlayerState by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
static void CG_InterpolatePlayerState( qboolean grabAngles ) {
	float			f;
	int				i;
	playerState_t	*out;
	snapshot_t		*prev, *next;

	out = &cg.predictedPlayerState;
	prev = cg.snap;
	next = cg.nextSnap;

	*out = cg.snap->ps;

	if( cg.showGameView ) {
		return;
	}

	// if we are still allowing local input, short circuit the view angles
	if ( grabAngles ) {
		usercmd_t	cmd;
		int			cmdNum;

		cmdNum = trap_GetCurrentCmdNumber();
		trap_GetUserCmd( cmdNum, &cmd );

		// rain - added tracemask
		PM_UpdateViewAngles( out, &cg.pmext, &cmd, CG_Trace, MASK_PLAYERSOLID );
	}

	// if the next frame is a teleport, we can't lerp to it
	if ( cg.nextFrameTeleport ) {
		return;
	}

	if ( !next || next->serverTime <= prev->serverTime ) {
		return;
	}

	f = (float)( cg.time - prev->serverTime ) / ( next->serverTime - prev->serverTime );

	i = next->ps.bobCycle;
	if ( i < prev->ps.bobCycle ) {
		i += 256;		// handle wraparound
	}
	out->bobCycle = int( prev->ps.bobCycle + f * ( i - prev->ps.bobCycle ) );

	for ( i = 0 ; i < 3 ; i++ ) {
		out->origin[i] = prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i] );
		if ( !grabAngles ) {
			out->viewangles[i] = LerpAngle( 
				prev->ps.viewangles[i], next->ps.viewangles[i], f );
		}
		out->velocity[i] = prev->ps.velocity[i] + 
			f * (next->ps.velocity[i] - prev->ps.velocity[i] );
	}

}

/*
===================
CG_TouchItem
===================
*/
static void CG_TouchItem( centity_t *cent ) {
	gitem_t		*item;

	return;

	if ( !cg_predictItems.integer) {
		return;
	}

	if ( !cg_autoactivate.integer) {
		return;
	}

	if ( !BG_PlayerTouchesItem( &cg.predictedPlayerState, &cent->currentState, cg.time ) ) {
		return;
	}

	// never pick an item up twice in a prediction
	if ( cent->miscTime == cg.time ) {
		return;
	}

	if ( !BG_CanItemBeGrabbed( &cent->currentState, &cg.predictedPlayerState, cgs.clientinfo[cg.snap->ps.clientNum].skill, cgs.clientinfo[cg.snap->ps.clientNum].team ) ) {
		return;		// can't hold it
	}

	item = &bg_itemlist[ cent->currentState.modelindex ];

	// force activate only for weapons you don't have
	if( item->giType == IT_WEAPON ) {
		if( item->giTag != WP_AMMO ) {
			if( !COM_BitCheck( cg.predictedPlayerState.weapons, item->giTag ) && item->giTag != WP_BINOCULARS) {
				return;	// force activate only
			}
		}
	}

	// OSP - Do it here rather than forcing gamestate into BG_CanItemBeGrabbed
	if(cgs.gamestate != GS_PLAYING &&
	   item->giType != IT_WEAPON &&
	   item->giType != IT_AMMO &&
	   item->giType != IT_HEALTH)
		return;

	// OSP - special case for panzers, as server may not allow us to pick them up
	//       let the server tell us for sure that we got it
	if(item->giType == IT_WEAPON && item->giTag == WP_PANZERFAUST) return;

	// (SA) treasure needs to be activeated, no touch
	if(item->giType == IT_TREASURE)
		return;

	// Special case for flags.  
	// We don't predict touching our own flag
	if (cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_AXIS &&
		item->giTag == PW_REDFLAG)
		return;
	if (cg.predictedPlayerState.persistant[PERS_TEAM] == TEAM_ALLIES &&
		item->giTag == PW_BLUEFLAG)
		return;


	// grab it
	BG_AddPredictableEventToPlayerstate( EV_ITEM_PICKUP, cent->currentState.modelindex , &cg.predictedPlayerState);

	// remove it from the frame so it won't be drawn
	cent->currentState.eFlags |= EF_NODRAW;

	// don't touch it again this prediction
	cent->miscTime = cg.time;

	// if its a weapon, give them some predicted ammo so the autoswitch will work
	if ( item->giType == IT_WEAPON ) {
		COM_BitSet( cg.predictedPlayerState.weapons, item->giTag );

		if ( !cg.predictedPlayerState.ammo[ BG_FindAmmoForWeapon( (weapon_t)item->giTag )] ) {
			cg.predictedPlayerState.ammo[ BG_FindAmmoForWeapon( (weapon_t)item->giTag )] = 1;
		}
	}
}

void CG_AddDirtBulletParticles( vec3_t origin, vec3_t dir, int speed, int duration, int count, float randScale, float width, float height, float alpha, qhandle_t shader);


/*
=========================
CG_TouchTriggerPrediction

Predict push triggers and items
=========================
*/
static void CG_TouchTriggerPrediction( void ) {
	int				i;
//	trace_t			trace;
	entityState_t	*ent;
	clipHandle_t	cmodel;
	centity_t		*cent;
	qboolean		spectator;
//	vec3_t			mins, maxs; // TTimo: unused
	const char		*cs;

	// dead clients don't activate triggers
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	spectator = (( cg.predictedPlayerState.pm_type == PM_SPECTATOR ) || (cg.predictedPlayerState.pm_flags & PMF_LIMBO)) ? qtrue : qfalse; // JPW NERVE

	if ( cg.predictedPlayerState.pm_type != PM_NORMAL && !spectator ) {
		return;
	}

	for ( i = 0 ; i < cg_numTriggerEntities ; i++ ) {
		cent = cg_triggerEntities[ i ];
		ent = &cent->currentState;

		if ( ent->eType == ET_ITEM && !spectator && (cg.predictedPlayerState.groundEntityNum == ENTITYNUM_WORLD) ) {
			CG_TouchItem( cent );
			continue;
		}

		// rain - #397 - added EF_FAKEBMODEL as well
		if (ent->solid != SOLID_BMODEL && !(ent->eFlags & EF_FAKEBMODEL)) {
			continue;
		}

		// Gordon: er, this lookup was wrong...
		cmodel = cgs.inlineDrawModel[ ent->modelindex ];
//		cmodel = trap_CM_InlineModel( ent->modelindex );

		// rain - #397 - don't try to fetch the model for a fake
		if (!(ent->eFlags & EF_FAKEBMODEL)) {
			cmodel = cgs.inlineDrawModel[ ent->modelindex ];
			if ( !cmodel ) {
				continue;
			}
		}

		if( ent->eType == ET_CONSTRUCTIBLE || 
			ent->eType == ET_OID_TRIGGER 
#ifdef VISIBLE_TRIGGERS
			|| ent->eType == ET_TRIGGER_MULTIPLE
			|| ent->eType == ET_TRIGGER_FLAGONLY
			|| ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE
#endif // VISIBLE_TRIGGERS
		) {
			vec3_t mins, maxs, pmins, pmaxs;

			if( ent->eType == ET_CONSTRUCTIBLE && ent->aiState ) {
				continue;
			}

			// rain - #397 - no real brush model, fake it
			if (ent->eType != ET_PLAYER && ent->eFlags & EF_FAKEBMODEL) {
				VectorCopy(ent->angles, mins);
				VectorCopy(ent->angles2, maxs);
			} else {
				trap_R_ModelBounds( cmodel, mins, maxs );
			}

			VectorAdd( cent->lerpOrigin, mins, mins );
			VectorAdd( cent->lerpOrigin, maxs, maxs );

#ifdef VISIBLE_TRIGGERS
			if( ent->eType == ET_TRIGGER_MULTIPLE || ent->eType == ET_TRIGGER_FLAGONLY || ent->eType == ET_TRIGGER_FLAGONLY_MULTIPLE ) {
			} else
#endif // VISIBLE_TRIGGERS
			{
				// expand the bbox a bit
				VectorSet( mins, mins[0] - 48, mins[1] - 48, mins[2] - 48 );
				VectorSet( maxs, maxs[0] + 48, maxs[1] + 48, maxs[2] + 48 );
			}

			VectorAdd( cg.predictedPlayerState.origin, cg_pmove.mins, pmins );
			VectorAdd( cg.predictedPlayerState.origin, cg_pmove.maxs, pmaxs );

#ifdef VISIBLE_TRIGGERS
			CG_RailTrail( NULL, mins, maxs, 1 );
#endif // VISIBLE_TRIGGERS

			if(!BG_BBoxCollision( pmins, pmaxs, mins, maxs )) {
				continue;
			}

			cs = NULL;
			if( ent->eType == ET_OID_TRIGGER ) {
				cs = CG_ConfigString( CS_OID_TRIGGERS + ent->teamNum );
			} else if( ent->eType == ET_CONSTRUCTIBLE ) {
				cs = CG_ConfigString( CS_OID_TRIGGERS + ent->otherEntityNum2 );
			}

			if(cs) {
				CG_ObjectivePrint( va( "You are near %s\n", cs ), SMALLCHAR_WIDTH );
			}

			continue;
		}
	}
}


#define MAX_PREDICT_ORIGIN_DELTA		0.1f
#define MAX_PREDICT_VELOCITY_DELTA		0.1f
#define MAX_PREDICT_VIEWANGLES_DELTA	1.0f

qboolean CG_PredictionOk( playerState_t *ps1, playerState_t *ps2 ) {
	vec3_t vec;
	int i;

	if(ps2->pm_type != ps1->pm_type || ps2->pm_flags != ps1->pm_flags || ps2->pm_time != ps1->pm_time ) {
		return qfalse;
	}

	VectorSubtract( ps2->origin, ps1->origin, vec );
	if( DotProduct( vec, vec ) > Square(MAX_PREDICT_ORIGIN_DELTA)) {
		return qfalse;
	}

	VectorSubtract( ps2->velocity, ps1->velocity, vec );
	if( DotProduct( vec, vec ) > Square(MAX_PREDICT_VELOCITY_DELTA) ) {
		return qfalse;
	}

	if( ps2->eFlags != ps1->eFlags ) {
		return qfalse;
	}

	if( ps2->weaponTime != ps1->weaponTime ) {
		return qfalse;
	}	

	if( ps2->groundEntityNum != ps1->groundEntityNum ) {
		return qfalse;
	}

	if( ps1->groundEntityNum != ENTITYNUM_WORLD || ps1->groundEntityNum != ENTITYNUM_NONE || ps2->groundEntityNum != ENTITYNUM_WORLD || ps2->groundEntityNum != ENTITYNUM_NONE ) {
		return qfalse;
	}

	if(	ps2->speed != ps1->speed || ps2->delta_angles[0] != ps1->delta_angles[0] || ps2->delta_angles[1] != ps1->delta_angles[1] || ps2->delta_angles[2] != ps1->delta_angles[2] ) {
		return qfalse;
	}

	if(	ps2->legsTimer != ps1->legsTimer || ps2->legsAnim != ps1->legsAnim ||
		ps2->torsoTimer != ps1->torsoTimer || ps2->torsoAnim != ps1->torsoAnim ) {
		return qfalse;
	}

/*	if( ps2->movementDir != ps1->movementDir ) {
		return qfalse;
	}*/

	if ( ps2->eventSequence != ps1->eventSequence ) {
		return qfalse;
	}

	for( i = 0; i < MAX_EVENTS; i++ ) {
		if( ps2->events[i] != ps1->events[i] || ps2->eventParms[i] != ps1->eventParms[i] ) {
			return qfalse;
		}
	}

	if( ps2->externalEvent != ps1->externalEvent || ps2->externalEventParm != ps1->externalEventParm || ps2->externalEventTime != ps1->externalEventTime ) {
		return qfalse;
	}

	if( ps2->clientNum != ps1->clientNum ) {
		return qfalse;
	} 

	if( ps2->weapon != ps1->weapon || ps2->weaponstate != ps1->weaponstate ) {
		return qfalse;
	}

	for( i = 0; i < 3; i++ ) {
		if (abs( int(ps2->viewangles[i] - ps1->viewangles[i]) ) > MAX_PREDICT_VIEWANGLES_DELTA) {
			return qfalse;
		}
	}

	if ( ps2->viewheight != ps1->viewheight ) {
		return qfalse;
	}

	if ( ps2->damageEvent != ps1->damageEvent || ps2->damageYaw != ps1->damageYaw || ps2->damagePitch != ps1->damagePitch || ps2->damageCount != ps1->damageCount ) {
		return qfalse;
	}

	for ( i = 0; i < MAX_STATS; i++ ) {
		if ( ps2->stats[i] != ps1->stats[i] ) {
			return qfalse;
		}
	}

	for ( i = 0; i < MAX_PERSISTANT; i++ ) {
		if ( ps2->persistant[i] != ps1->persistant[i] ) {
			return qfalse;
		}
	}

	for ( i = 0; i < MAX_POWERUPS; i++ ) {
		if ( ps2->powerups[i] != ps1->powerups[i] ) {
			return qfalse;
		}
	}

	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		if ( ps2->ammo[i] != ps1->ammo[i] || ps2->ammoclip[i] != ps1->ammoclip[i] ) {
			return qfalse;
		}
	}

	if( ps1->viewlocked != ps2->viewlocked || ps1->viewlocked_entNum !=  ps2->viewlocked_entNum ) {
		return qfalse;
	}

	if( ps1->onFireStart != ps2->onFireStart ) {
		return qfalse;
	}

	return qtrue;
}

#define RESET_PREDICTION						\
	cg.lastPredictedCommand = 0;				\
	cg.backupStateTail = cg.backupStateTop;		\
	useCommand = current - CMD_BACKUP + 1;


//unlagged - optimized prediction
#define ABS(x) ((x) < 0 ? (-(x)) : (x))

static int IsUnacceptableError( playerState_t *ps, playerState_t *pps ) {
	vec3_t delta;

	if ( pps->pm_type != ps->pm_type ||
			pps->pm_flags != ps->pm_flags ||
			pps->pm_time != ps->pm_time ) {
		return 1;
	}

	VectorSubtract( pps->origin, ps->origin, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		if ( cg_showmiss.integer ) {
			CG_Printf("delta: %.2f  ", VectorLength(delta) );
		}
		return 2;
	}

	VectorSubtract( pps->velocity, ps->velocity, delta );
	if ( VectorLengthSquared( delta ) > 0.1f * 0.1f ) {
		if ( cg_showmiss.integer ) {
			CG_Printf("delta: %.2f  ", VectorLength(delta) );
		}
		return 3;
	}

	if ( pps->weaponTime != ps->weaponTime ||
			pps->gravity != ps->gravity ||
			pps->speed != ps->speed ||
			pps->delta_angles[0] != ps->delta_angles[0] ||
			pps->delta_angles[1] != ps->delta_angles[1] ||
			pps->delta_angles[2] != ps->delta_angles[2] || 
			pps->groundEntityNum != ps->groundEntityNum ) {
		return 4;
	}

	if ( pps->legsTimer != ps->legsTimer ||
			pps->legsAnim != ps->legsAnim ||
			pps->torsoTimer != ps->torsoTimer ||
			pps->torsoAnim != ps->torsoAnim ||
			pps->movementDir != ps->movementDir ) {
		return 5;
	}

	if( ps->viewlocked != pps->viewlocked || ps->viewlocked_entNum != pps->viewlocked_entNum ) {
		return 6;
	}

	if ( pps->eFlags != ps->eFlags ) {
		return 7;
	}

	if ( pps->eventSequence != ps->eventSequence ) {
		return 8;
	}

	if ( pps->externalEvent != ps->externalEvent ||
			pps->externalEventParm != ps->externalEventParm ||
			pps->externalEventTime != ps->externalEventTime ) {
		return 9;
	}

	if ( pps->clientNum != ps->clientNum ||
			pps->weapon != ps->weapon ||
			pps->weaponstate != ps->weaponstate ) {
		return 10;
	}

	if ( ABS(pps->viewangles[0] - ps->viewangles[0]) > 1.0f ||
			ABS(pps->viewangles[1] - ps->viewangles[1]) > 1.0f ||
			ABS(pps->viewangles[2] - ps->viewangles[2]) > 1.0f ) {
		return 11;
	}

	if ( pps->viewheight != ps->viewheight ) {
		return 12;
	}

	if ( pps->damageEvent != ps->damageEvent ||
			pps->damageYaw != ps->damageYaw ||
			pps->damagePitch != ps->damagePitch ||
			pps->damageCount != ps->damageCount ) {
		return 13;
	}

	if( ps->onFireStart != pps->onFireStart ) {
		return 14;
	}

	if ((ps->groundEntityNum != ENTITYNUM_WORLD && ps->groundEntityNum != ENTITYNUM_NONE) || (pps->groundEntityNum != ENTITYNUM_WORLD && pps->groundEntityNum != ENTITYNUM_NONE)) {
		return 15;
	}

    // Additional ET-specific weapons times
    if (ps->grenadeTimeLeft != pps->grenadeTimeLeft || ps->weaponDelay != pps->weaponDelay) {
        return 16;
    }

    // ET-specific class time
    if (ps->classWeaponTime != pps->classWeaponTime) {
        return 17;
    }

    if (memcmp( pps->events, ps->events, sizeof(pps->events) ))
        return 18;

    if (memcmp( pps->eventParms, ps->eventParms, sizeof(pps->eventParms) ))
        return 19;

    if (memcmp( pps->stats, ps->stats, sizeof(pps->stats) ))
        return 20;

    if (memcmp( pps->persistant, ps->persistant, sizeof(pps->persistant) ))
        return 21;

    if (memcmp( pps->powerups, ps->powerups, sizeof(pps->powerups) ))
        return 22;

    if (memcmp( pps->ammo, ps->ammo, sizeof(pps->ammo) ))
        return 23;

    if (memcmp( pps->ammoclip, ps->ammoclip, sizeof(pps->ammoclip) ))
        return 24;

    if (memcmp( pps->holdable, ps->holdable, sizeof(pps->holdable) ))
        return 25;

	return 0;
}
//unlagged - optimized prediction

/*
=================
CG_PredictPlayerState

Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.

OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction. (this is "dead reckoning" and would definately
be nice to have in there (SA))

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/

// rain - we need to keep pmext around for old frames, because Pmove()
// fills in some values when it does prediction.  This in itself is fine,
// but the prediction loop starts in the past and predicts from the
// snapshot time up to the current time, and having things like jumpTime
// appear to be set for prediction runs where they previously weren't
// is a Bad Thing.  This is my bugfix for #166.

pmoveExt_t oldpmext[CMD_BACKUP];

void CG_PredictPlayerState( void ) {
	int			cmdNum, current;
	playerState_t	oldPlayerState;
	qboolean	moved;
	usercmd_t	oldestCmd;
	usercmd_t	latestCmd;
	vec3_t		deltaAngles;
	pmoveExt_t	pmext;
//	int useCommand = 0;
//unlagged - optimized prediction
	int stateIndex = 0, predictCmd = 0;
	int numPredicted = 0, numPlayedBack = 0; // debug code
//unlagged - optimized prediction

	cg.hyperspace = qfalse;	// will be set if touching a trigger_teleport

	// if this is the first frame we must guarantee
	// predictedPlayerState is valid even if there is some
	// other error condition
	if ( !cg.validPPS ) {
		cg.validPPS = qtrue;
		cg.predictedPlayerState = cg.snap->ps;
	}

	// demo playback just copies the moves
	if(cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW)) {
		CG_InterpolatePlayerState( qfalse );
		return;
	}

	// non-predicting local movement will grab the latest angles
	if ( cg_nopredict.integer ) {
		cg_pmove.ps = &cg.predictedPlayerState;
		cg_pmove.pmext = &cg.pmext;

		cg.pmext.airleft = (cg.waterundertime - cg.time);

		// Arnout: are we using an mg42?
		if( cg_pmove.ps->eFlags & EF_MG42_ACTIVE || cg_pmove.ps->eFlags & EF_AAGUN_ACTIVE ) {
			cg.pmext.harc = cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[0];
			cg.pmext.varc = cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[1];
			
			VectorCopy( cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.angles2, cg.pmext.centerangles );

			cg.pmext.centerangles[PITCH] =	AngleNormalize180( cg.pmext.centerangles[PITCH] );
			cg.pmext.centerangles[YAW] =	AngleNormalize180( cg.pmext.centerangles[YAW]	);
			cg.pmext.centerangles[ROLL] =	AngleNormalize180( cg.pmext.centerangles[ROLL]	);
		} 

		CG_InterpolatePlayerState( qtrue );
		return;
	}

	if(cg_pmove.ps && cg_pmove.ps->eFlags & EF_MOUNTEDTANK) {
		centity_t* tank = &cg_entities[cg_entities[cg.snap->ps.clientNum].tagParent];

		cg.pmext.centerangles[YAW] = tank->lerpAngles[ YAW ];
		cg.pmext.centerangles[PITCH] = tank->lerpAngles[ PITCH ];
	}

	// prepare for pmove
	cg_pmove.ps = &cg.predictedPlayerState;
	cg_pmove.pmext = &pmext; //&cg.pmext;
	cg_pmove.character = CG_CharacterForClientinfo( &cgs.clientinfo[cg.snap->ps.clientNum], &cg_entities[cg.snap->ps.clientNum] );
	cg.pmext.airleft = (cg.waterundertime - cg.time);

	// Arnout: are we using an mg42?
	if( cg_pmove.ps->eFlags & EF_MG42_ACTIVE || cg_pmove.ps->eFlags & EF_AAGUN_ACTIVE ) {
		cg.pmext.harc = cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[0];
		cg.pmext.varc = cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.origin2[1];
		
		VectorCopy( cg_entities[cg_pmove.ps->viewlocked_entNum].currentState.angles2, cg.pmext.centerangles );

		cg.pmext.centerangles[PITCH] =	AngleNormalize180( cg.pmext.centerangles[PITCH] );
		cg.pmext.centerangles[YAW] =	AngleNormalize180( cg.pmext.centerangles[YAW]	);
		cg.pmext.centerangles[ROLL] =	AngleNormalize180( cg.pmext.centerangles[ROLL]	);
	} else if(cg_pmove.ps->eFlags & EF_MOUNTEDTANK) {
		centity_t* tank = &cg_entities[cg_entities[cg.snap->ps.clientNum].tagParent];

		cg.pmext.centerangles[PITCH] = tank->lerpAngles[PITCH];
	}

	cg_pmove.skill = cgs.clientinfo[cg.snap->ps.clientNum].skill;

	cg_pmove.trace = CG_TraceCapsule;
	//cg_pmove.trace = CG_Trace;
	cg_pmove.pointcontents = CG_PointContents;
	if ( cg_pmove.ps->pm_type == PM_DEAD ) {
		cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;		
		cg_pmove.ps->eFlags |= EF_DEAD; // DHM-Nerve added:: EF_DEAD is checked for in Pmove functions, but wasn't being set until after Pmove
	} else if( cg_pmove.ps->pm_type == PM_SPECTATOR ) {
		// rain - fix the spectator can-move-partway-through-world weirdness
		// bug by actually setting tracemask when spectating :x
		cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
		cg_pmove.trace = CG_TraceCapsule_World;
	} else {
		cg_pmove.tracemask = MASK_PLAYERSOLID;
	}

	if (( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) || (cg.snap->ps.pm_flags & PMF_LIMBO)) { // JPW NERVE limbo
		cg_pmove.tracemask &= ~CONTENTS_BODY;	// spectators can fly through bodies
	}

	cg_pmove.noFootsteps = qfalse;
	cg_pmove.noWeapClips = qfalse;

	// save the state before the pmove so we can detect transitions
	oldPlayerState = cg.predictedPlayerState;

	current = trap_GetCurrentCmdNumber();

	// rain - fill in the current cmd with the latest prediction from
	// cg.pmext (#166)
	memcpy(&oldpmext[current & CMD_MASK], &cg.pmext, sizeof(pmoveExt_t));

	// if we don't have the commands right after the snapshot, we
	// can't accurately predict a current position, so just freeze at
	// the last good position we had
	cmdNum = current - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &oldestCmd );
	if ( oldestCmd.serverTime > cg.snap->ps.commandTime 
		&& oldestCmd.serverTime < cg.time ) {	// special check for map_restart
		if ( cg_showmiss.integer ) {
			CG_Printf ("exceeded PACKET_BACKUP on commands\n");
		}
	}

	// get the latest command so we can know which commands are from previous map_restarts
	trap_GetUserCmd( current, &latestCmd );

	// get the most recent information we have, even if
	// the server time is beyond our current cg.time,
	// because predicted player positions are going to 
	// be ahead of everything else anyway
	// rain - NEIN - this'll cause us to execute events from the next frame
	// early, resulting in doubled events and the like.  it seems to be
	// worse as far as prediction, too, so BLAH at id. (#405)
#if 0
	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		cg.predictedPlayerState = cg.nextSnap->ps;
		cg.physicsTime = cg.nextSnap->serverTime;
	} else {
#endif
		cg.predictedPlayerState = cg.snap->ps;
		cg.physicsTime = cg.snap->serverTime;
//	}

	if ( pmove_msec.integer < 8 ) {
		trap_Cvar_Set("pmove_msec", "8");
	}
	else if (pmove_msec.integer > 33) {
		trap_Cvar_Set("pmove_msec", "33");
	}

	cg_pmove.pmove_fixed = pmove_fixed.integer;// | cg_pmove_fixed.integer;
	cg_pmove.pmove_msec = pmove_msec.integer;

//unlagged - optimized prediction
	// Like the comments described above, a player's state is entirely
	// re-predicted from the last valid snapshot every client frame, which
	// can be really, really, really slow.  Every old command has to be
	// run again.  For every client frame that is *not* directly after a
	// snapshot, this is unnecessary, since we have no new information.
	// For those, we'll play back the predictions from the last frame and
	// predict only the newest commands.  Essentially, we'll be doing
	// an incremental predict instead of a full predict.
	//
	// If we have a new snapshot, we can compare its player state's command
	// time to the command times in the queue to find a match.  If we find
	// a matching state, and the predicted version has not deviated, we can
	// use the predicted state as a base - and also do an incremental predict.
	//
	// With this method, we get incremental predicts on every client frame
	// except a frame following a new snapshot in which there was a prediction
	// error.  This yeilds anywhere from a 15% to 40% performance increase,
	// depending on how much of a bottleneck the CPU is.

	// we check for cg_latentCmds because it'll mess up the optimization
	// FIXME: make cg_latentCmds work with cg_optimizePrediction?
	if ( cg_optimizePrediction.integer ) {
		if ( cg.nextFrameTeleport || cg.thisFrameTeleport ) {
			// do a full predict
			cg.lastPredictedCommand = 0;
			cg.backupStateTail = cg.backupStateTop;
			predictCmd = current - CMD_BACKUP + 1;
		}
		// cg.physicsTime is the current snapshot's serverTime
		// if it's the same as the last one
		else if ( cg.physicsTime == cg.lastPhysicsTime ) {
			// we have no new information, so do an incremental predict
			predictCmd = cg.lastPredictedCommand + 1;
		}
		else {
			// we have a new snapshot

			int i;
			qboolean error = qtrue;

			// loop through the saved states queue
			for ( i = cg.backupStateTop; i != cg.backupStateTail; i = (i + 1) % NUM_SAVED_STATES ) {
				// if we find a predicted state whose commandTime matches the snapshot player state's commandTime
				if ( cg.backupStates[i].commandTime == cg.predictedPlayerState.commandTime ) {
					// make sure the state differences are acceptable
					int errorcode = IsUnacceptableError( &cg.predictedPlayerState, &cg.backupStates[i] );

					// too much change?
					if ( errorcode ) {
						if ( cg_showmiss.integer ) {
							CG_Printf("errorcode %d at %d\n", errorcode, cg.time);
						}
						// yeah, so do a full predict
						break;
					}

					// this one is almost exact, so we'll copy it in as the starting point
					*cg_pmove.ps = cg.backupStates[i];
					// advance the head
					cg.backupStateTop = (i + 1) % NUM_SAVED_STATES;

					// set the next command to predict
					predictCmd = cg.lastPredictedCommand + 1;

					// a saved state matched, so flag it
					error = qfalse;
					break;
				}
			}

			// if no saved states matched
			if ( error ) {
				// do a full predict
				cg.lastPredictedCommand = 0;
				cg.backupStateTail = cg.backupStateTop;
				predictCmd = current - CMD_BACKUP + 1;
			}
		}

		// keep track of the server time of the last snapshot so we
		// know when we're starting from a new one in future calls
		cg.lastPhysicsTime = cg.physicsTime;
		stateIndex = cg.backupStateTop;
	}
//unlagged - optimized prediction

	// run cmds
	moved = qfalse;
	for ( cmdNum = current - CMD_BACKUP + 1 ; cmdNum <= current ; cmdNum++ ) {
		// get the command
		trap_GetUserCmd( cmdNum, &cg_pmove.cmd );
		// get the previous command
		trap_GetUserCmd( cmdNum-1, &cg_pmove.oldcmd );

		if ( cg_pmove.pmove_fixed ) {
			// rain - added tracemask
			PM_UpdateViewAngles( cg_pmove.ps, cg_pmove.pmext, &cg_pmove.cmd, CG_Trace, cg_pmove.tracemask );
		}
		
		// don't do anything if the time is before the snapshot player time
		if ( cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime ) {
			continue;
		}

		// don't do anything if the command was from a previous map_restart
		if ( cg_pmove.cmd.serverTime > latestCmd.serverTime ) {
			continue;
		}

		// check for a prediction error from last frame
		// on a lan, this will often be the exact value
		// from the snapshot, but on a wan we will have
		// to predict several commands to get to the point
		// we want to compare
		if ( cg.predictedPlayerState.commandTime == oldPlayerState.commandTime ) {
			vec3_t	delta;
			float	len;

			if( BG_PlayerMounted( cg_pmove.ps->eFlags ) ) {
				// no prediction errors here, we're locked in place
				VectorClear( cg.predictedError );
			} else if ( cg.thisFrameTeleport ) {
				// a teleport will not cause an error decay
				VectorClear( cg.predictedError );
				if ( cg_showmiss.integer ) {
					CG_Printf( "PredictionTeleport\n" );
				}
				cg.thisFrameTeleport = qfalse;
			} else if( !cg.showGameView ) {
				vec3_t	adjusted;
				CG_AdjustPositionForMover( cg.predictedPlayerState.origin, cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.oldTime, adjusted, deltaAngles );
				// RF, add the deltaAngles (fixes jittery view while riding trains)
				// ydnar: only do this if player is prone or using set mortar
				if( (cg.predictedPlayerState.eFlags & EF_PRONE) || cg.weaponSelect == WP_MORTAR_SET )
					cg.predictedPlayerState.delta_angles[YAW] += ANGLE2SHORT(deltaAngles[YAW]);

				if ( cg_showmiss.integer ) {
					if (!VectorCompare( oldPlayerState.origin, adjusted )) {
						CG_Printf("prediction error\n");
					}
				}
				VectorSubtract( oldPlayerState.origin, adjusted, delta );
				len = VectorLength( delta );
				if ( len > 0.1 ) {
					if ( cg_showmiss.integer ) {
						CG_Printf("Prediction miss: %f\n", len);
					}
					if ( cg_errorDecay.integer ) {
						int		t;
						float	f;

						t = cg.time - cg.predictedErrorTime;
						f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
						if ( f < 0 ) {
							f = 0;
						}
						if ( f > 0 && cg_showmiss.integer ) {
							CG_Printf("Double prediction decay: %f\n", f);
						}
						VectorScale( cg.predictedError, f, cg.predictedError );
					} else {
						VectorClear( cg.predictedError );
					}
					VectorAdd( delta, cg.predictedError, cg.predictedError );
					cg.predictedErrorTime = cg.oldTime;
				}
			}
		}

		// don't predict gauntlet firing, which is only supposed to happen
		// when it actually inflicts damage
		cg_pmove.gauntletHit = qfalse;

		if ( cg_pmove.pmove_fixed ) {
			cg_pmove.cmd.serverTime = ((cg_pmove.cmd.serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		}

		// ydnar: if server respawning, freeze the player
		if( cg.serverRespawning )
			cg_pmove.ps->pm_type = PM_FREEZE;
		
		cg_pmove.gametype = cgs.gametype;
		
		// rain - only fill in the charge times if we're on a playing team
		if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_AXIS || cg.snap->ps.persistant[PERS_TEAM] == TEAM_ALLIES ) {
			cg_pmove.ltChargeTime =			cg.ltChargeTime[cg.snap->ps.persistant[PERS_TEAM]-1];
			cg_pmove.soldierChargeTime =	cg.soldierChargeTime[cg.snap->ps.persistant[PERS_TEAM]-1];
			cg_pmove.engineerChargeTime =	cg.engineerChargeTime[cg.snap->ps.persistant[PERS_TEAM]-1];
			cg_pmove.medicChargeTime =		cg.medicChargeTime[cg.snap->ps.persistant[PERS_TEAM]-1];
			cg_pmove.covertopsChargeTime =	cg.covertopsChargeTime[cg.snap->ps.persistant[PERS_TEAM]-1];
		}

		// rain - copy the pmext as it was just before we
		// previously ran this cmd (or, this will be the
		// current predicted data if this is the current cmd)  (#166)
		memcpy(&pmext, &oldpmext[cmdNum & CMD_MASK], sizeof(pmoveExt_t));

		fflush(stdout);

//unlagged - optimized prediction
		// we check for cg_latentCmds because it'll mess up the optimization
		if ( cg_optimizePrediction.integer ) {
			// if we need to predict this command, or we've run out of space in the saved states queue
			if ( cmdNum >= predictCmd || (stateIndex + 1) % NUM_SAVED_STATES == cg.backupStateTop ) {
				// run the Pmove
				Pmove (&cg_pmove);

				numPredicted++; // debug code

				// record the last predicted command
				cg.lastPredictedCommand = cmdNum;

				// if we haven't run out of space in the saved states queue
				if ( (stateIndex + 1) % NUM_SAVED_STATES != cg.backupStateTop ) {
					// save the state for the false case (of cmdNum >= predictCmd)
					// in later calls to this function
					cg.backupStates[stateIndex] = *cg_pmove.ps;
					stateIndex = (stateIndex + 1) % NUM_SAVED_STATES;
					cg.backupStateTail = stateIndex;
				}
			}
			else {
				numPlayedBack++; // debug code

				if ( cg_showmiss.integer && cg.backupStates[stateIndex].commandTime != cg_pmove.cmd.serverTime) {
					// this should ONLY happen just after changing the value of pmove_fixed
					CG_Printf( "saved state miss\n" );
				}

				// play back the command from the saved states
				*cg_pmove.ps = cg.backupStates[stateIndex];

				// go to the next element in the saved states array
				stateIndex = (stateIndex + 1) % NUM_SAVED_STATES;
			}
		}
		else {
			// run the Pmove
			Pmove (&cg_pmove);

			numPredicted++; // debug code
		}
//unlagged - optimized prediction

		moved = qtrue;

		// add push trigger movement effects
		CG_TouchTriggerPrediction();
	}

//unlagged - optimized prediction
    // Jaybird - add to totals
    cg.numPlayedBack += numPlayedBack;
    cg.numPredicted += numPredicted;
    if (numPredicted == 1) {
        cg.numSinglePredicted ++;
    }

    if( cg_debugDelag.integer & DEBUGDELAG_PREDICTION ) {
    	// do a /condump after a few seconds of this
        CG_Printf("^3Optimized Prediction: ^7cg.time: %d, numPredicted: %d, numPlayedBack: %d, avgPredicted: %.2f, avgPlayedBack: %.2f, avgSinglePredicted: %.2f\n",
            cg.time,
            numPredicted,
            numPlayedBack,
            (float)cg.numPredicted / cg.clientFrame,
            (float)cg.numPlayedBack / cg.clientFrame,
            (float)cg.numSinglePredicted / cg.clientFrame);
	    // if everything is working right, numPredicted should be 1 more than 98%
	    // of the time, meaning only ONE predicted move was done in the frame
	    // you should see other values for numPredicted after IsUnacceptableError
	    // returns nonzero, and that's it

        // Jaybird note - 98% is probably a bit wishful thinking with ET.  There
        // are many new things in ET and thus more conditions for failure in
        // the deltas between commands.  When playing locally, I had an average
        // of between 90% and 95%.
    }
//unlagged - optimized prediction

	if ( cg_showmiss.integer > 1 ) {
		CG_Printf( "[%i : %i] ", cg_pmove.cmd.serverTime, cg.time );
	}

	if ( !moved ) {
		if ( cg_showmiss.integer ) {
			CG_Printf( "not moved\n" );
		}
		return;
	}

	// restore pmext
	memcpy( &cg.pmext, &pmext, sizeof(pmoveExt_t) );

	if( !cg.showGameView ) {
		// adjust for the movement of the groundentity
		CG_AdjustPositionForMover( cg.predictedPlayerState.origin, cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.time, cg.predictedPlayerState.origin, deltaAngles );
	}

	// fire events and other transition triggered things
	CG_TransitionPlayerState( &cg.predictedPlayerState, &oldPlayerState );
	
	
	// ydnar: shake player view here, rather than fiddle with view angles
	if( cg.time > cg.cameraShakeTime )
		cg.cameraShakeScale = 0;
	else
	{
		float x;
		
		
		// starts at 1, approaches 0 over time
		x = (cg.cameraShakeTime - cg.time) / cg.cameraShakeLength;
		
		// move
		cg.predictedPlayerState.origin[ 2 ] +=
		sin( M_PI * 8 * 13 + cg.cameraShakePhase ) * x * 6.0f * cg.cameraShakeScale;
		
		cg.predictedPlayerState.origin[ 1 ] +=
			sin( M_PI * 17 * x + cg.cameraShakePhase ) * x * 6.0f * cg.cameraShakeScale;
		
		cg.predictedPlayerState.origin[ 0 ] +=
			cos( M_PI * 7 * x + cg.cameraShakePhase ) * x * 6.0f * cg.cameraShakeScale;
	}
}
