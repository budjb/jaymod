#include <bgame/impl.h>

/*
===========================================================
The following is code that implements client smoothing.
This code is all derived from functions in bg_pmove.cpp
and bg_slidemove.cpp.  They are either complete conversions
or nearly complete subsets.  The original Unlagged source
was used as guidance.

This code has nothing to do with backwards-reconciled
weapons hit testing.  It is fully related to smoothing
client movement, and thus MUST match the code in bg_pmove
and bg_slidemove.  Any changes here or there must be
duplicated in their counterparts!
===========================================================
*/

#define	STEPSIZE 18
#define	MAX_CLIP_PLANES	5

/*
=============================
G_PredictPlayerTraceLegs
-----------------------------
Derived from PM_TraceLegs
=============================
*/
void G_PredictPlayerTraceLegs( trace_t *trace, float *legsOffset, vec3_t start, vec3_t end, trace_t *bodytrace, vec3_t viewangles, int ignoreent, int tracemask, bool prone )
{
	trace_t steptrace;
	vec3_t ofs, org, point;
	vec3_t flatforward;
	float angle;

	// zinx - don't let players block legs
	tracemask &= ~(CONTENTS_BODY | CONTENTS_CORPSE);

	if (legsOffset) {
		*legsOffset = 0;
	}

	angle = DEG2RAD(viewangles[YAW]);
	flatforward[0] = cos(angle);
	flatforward[1] = sin(angle);
	flatforward[2] = 0;

    if (prone) {
	    VectorScale(flatforward, -32, ofs);
    } else {
        VectorScale(flatforward, 32, ofs);
    }

	VectorAdd(start, ofs, org);
	VectorAdd(end, ofs, point);
	trap_TraceCapsule(trace, org, playerlegsProneMins, playerlegsProneMaxs, point, ignoreent, tracemask);
	if (!bodytrace || trace->fraction < bodytrace->fraction ||
	    trace->allsolid) {
		// legs are clipping sooner than body
		// see if our legs can step up

		// give it a try with the new height
		ofs[2] += STEPSIZE;

		VectorAdd(start, ofs, org);
		VectorAdd(end, ofs, point);
		trap_TraceCapsule(&steptrace, org, playerlegsProneMins, playerlegsProneMaxs, point, ignoreent, tracemask);
		if (!steptrace.allsolid && !steptrace.startsolid &&
		    steptrace.fraction > trace->fraction) {
			// the step trace did better -- use it instead
			*trace = steptrace;

			// get legs offset
			if (legsOffset) {
				*legsOffset = ofs[2];

				VectorCopy(steptrace.endpos, org);
				VectorCopy(steptrace.endpos, point);
				point[2] -= STEPSIZE;

				trap_TraceCapsule(&steptrace, org, playerlegsProneMins, playerlegsProneMaxs, point, ignoreent, tracemask);
				if (!steptrace.allsolid) {
					*legsOffset = ofs[2] - (org[2] - steptrace.endpos[2]);
				}
			}
		}
	}
}

/*
=============================
G_PredictPlayerTraceAllLegs
-----------------------------
Derived from PM_TraceAllLegs
=============================
*/
void G_PredictPlayerTraceAllLegs( trace_t *trace, float *legsOffset, vec3_t start, vec3_t end, gentity_t* ent )
{
	trap_TraceCapsule(trace, start, ent->r.mins, ent->r.maxs, end, ent->s.number, ent->clipmask);
	
	/* legs */
	if ( ent->client->ps.eFlags & (EF_PRONE|EF_PLAYDEAD) ) {
		trace_t legtrace;

        G_PredictPlayerTraceLegs( &legtrace, legsOffset, start, end, trace, ent->client->ps.viewangles, ent->s.number, ent->clipmask, (ent->client->ps.eFlags & EF_PRONE) ? true : false );

		if (legtrace.fraction < trace->fraction ||
		    legtrace.startsolid ||
		    legtrace.allsolid) {
			VectorSubtract( end, start, legtrace.endpos );
			VectorMA( start, legtrace.fraction, legtrace.endpos, legtrace.endpos );
			*trace = legtrace;
		}
	}
}

/*
=============================
G_PredictPlayerTraceAll
-----------------------------
Derived from PM_TraceAll
=============================
*/
void G_PredictPlayerTraceAll( trace_t *trace, vec3_t start, vec3_t end, gentity_t* ent )
{
	G_PredictPlayerTraceAllLegs( trace, NULL, start, end, ent );
}

/*
=============================
G_PredictPlayerClipVelocity
-----------------------------
Derived from PM_ClipVelocity
=============================
*/
void G_PredictPlayerClipVelocity( vec3_t in, vec3_t normal, vec3_t out ) {
	float	backoff;

	// find the magnitude of the vector "in" along "normal"
	backoff = DotProduct (in, normal);

	// tilt the plane a bit to avoid floating-point error issues
	if ( backoff < 0 ) {
		backoff *= OVERCLIP;
	} else {
		backoff /= OVERCLIP;
	}

	// slide along
	VectorMA( in, -backoff, normal, out );
}

/*
=============================
G_PredictPlayerSlideMove
-----------------------------
Derived from PM_SlideMove
=============================
*/
qboolean G_PredictPlayerSlideMove( gentity_t* ent, float frametime ) {
	int			bumpcount, numbumps, extrabumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, velocity, origin;
	vec3_t		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	float		into;
	vec3_t		endVelocity;
	vec3_t		endClipVelocity;
	
	numbumps = 4;
	extrabumps = 0;

	VectorCopy (ent->s.pos.trDelta, primal_velocity);
    VectorCopy (primal_velocity, velocity);
    VectorCopy (ent->s.pos.trBase, origin);

    VectorCopy (velocity, endVelocity);

	time_left = frametime;

	numplanes = 0;

	for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) {

		// calculate position we are trying to move to
		VectorMA( origin, time_left, velocity, end );

		// see if we can make it there
		G_PredictPlayerTraceAll( &trace, origin, end, ent );

		if (trace.allsolid) {
			VectorClear(velocity);
            VectorCopy(origin, ent->s.pos.trBase);
			return qtrue;
		}

		if (trace.fraction > 0) {
			// actually covered some distance
			VectorCopy (trace.endpos, origin);
		}

		if (trace.fraction == 1) {
			 break;		// moved the entire distance
		}

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) {
			// this shouldn't really happen
			VectorClear( velocity );
            VectorCopy(origin, ent->s.pos.trBase);
			return qtrue;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for ( i = 0 ; i < numplanes ; i++ ) {
			if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) {
				if ( extrabumps <= 0 ) {
					VectorAdd( trace.plane.normal, velocity, velocity );
					extrabumps++;
					numbumps++;
				} else {
					// zinx - if it happens again, nudge the origin instead,
					// and trace it, to make sure we don't end up in a solid

					VectorAdd( origin, trace.plane.normal, end );
					G_PredictPlayerTraceAll( &trace, origin, end, ent );
					VectorCopy( trace.endpos, origin );

				}
				break;
			}
		}
		if ( i < numplanes ) {
			continue;
		}

		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for ( i = 0 ; i < numplanes ; i++ ) {
			into = DotProduct( velocity, planes[i] );
			if ( into >= 0.1 ) {
				continue;		// move doesn't interact with the plane
			}

			// slide along the plane
			G_PredictPlayerClipVelocity (velocity, planes[i], clipVelocity);

			// slide along the plane
			G_PredictPlayerClipVelocity (endVelocity, planes[i], endClipVelocity);

			// see if there is a second plane that the new move enters
			for ( j = 0 ; j < numplanes ; j++ ) {
				if ( j == i ) {
					continue;
				}

				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				G_PredictPlayerClipVelocity( clipVelocity, planes[j], clipVelocity );
				G_PredictPlayerClipVelocity( endClipVelocity, planes[j], endClipVelocity );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, velocity );
				VectorScale( dir, d, clipVelocity );

				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				VectorScale( dir, d, endClipVelocity );

				// see if there is a third plane the the new move enters
				for ( k = 0 ; k < numplanes ; k++ ) {
					if ( k == i || k == j ) {
						continue;
					}
					if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) {
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a triple plane interaction
					VectorClear( velocity );
                    VectorCopy( origin, ent->s.pos.trBase );
					return qtrue;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, velocity );
			VectorCopy( endClipVelocity, endVelocity );
			break;
		}
	}

    VectorCopy( endVelocity, velocity );
    VectorCopy( origin, ent->s.pos.trBase );

	return ( bumpcount != 0 ) ? qtrue : qfalse;
}

/*
=============================
G_PredictPlayerStepSlideMove
-----------------------------
Derived from PM_StepSlideMove
=============================
*/
void G_PredictPlayerStepSlideMove( gentity_t* ent, float frametime ) {
	vec3_t		start_o, start_v, down_o, down_v;
	vec3_t		up, down;
	trace_t		trace;

	VectorCopy (ent->s.pos.trBase, start_o);
	VectorCopy (ent->s.pos.trDelta, start_v);

	if ( !G_PredictPlayerSlideMove( ent, frametime ) ) {
		return;		// we got exactly where we wanted to go first try
	}

	VectorCopy (ent->s.pos.trBase, down_o);
	VectorCopy (ent->s.pos.trDelta, down_v);

	VectorCopy (start_o, up);
	up[2] += STEPSIZE;

	// test the player position if they were a stepheight higher
	G_PredictPlayerTraceAll( &trace, start_o, up, ent );
	if ( trace.allsolid ) {
		return;		// can't step up
	}

	// try slidemove from this position
	VectorCopy (up, ent->s.pos.trBase);
	VectorCopy (start_v, ent->s.pos.trDelta);

	G_PredictPlayerSlideMove( ent, frametime );

	// push down the final amount
	VectorCopy (ent->s.pos.trBase, down);
	down[2] -= STEPSIZE;

	// check legs separately
	if ( ent->client->ps.eFlags & (EF_PRONE|EF_PLAYDEAD) ) {
        G_PredictPlayerTraceLegs( &trace, NULL, ent->s.pos.trBase, down, NULL, ent->client->ps.viewangles, ent->s.number, ent->clipmask, (ent->client->ps.eFlags & EF_PRONE) ? true : false );
		if ( trace.allsolid ) {
			// legs don't step, just fuzz.
			VectorCopy( down_o, ent->s.pos.trBase );
			VectorCopy( down_v, ent->s.pos.trDelta );
			return;
		}
	}

	trap_TraceCapsule( &trace, ent->s.pos.trBase, ent->r.mins, ent->r.maxs, down, ent->s.number, ent->clipmask );
	if ( !trace.allsolid ) {
		VectorCopy (trace.endpos, ent->s.pos.trBase);
	}
	if ( trace.fraction < 1.0 ) {
		G_PredictPlayerClipVelocity( ent->s.pos.trDelta, trace.plane.normal, ent->s.pos.trDelta );
	}
}

/*
=============================
G_SkipCorrectionSafe
=============================
*/
qboolean G_SkipCorrectionSafe(gentity_t *ent) 
{
	if (!ent || !ent->inuse || !ent->client || !ent->r.linked)
		return qfalse;

	if (ent->client->sess.sessionTeam != TEAM_AXIS && ent->client->sess.sessionTeam != TEAM_ALLIES)
		return qfalse;
	
	if (ent->client->ps.pm_flags & (PMF_LIMBO | PMF_TIME_LOCKPLAYER))
		return qfalse;

	if (ent->health <= 0)
		return qfalse;

	if (ent->client->ps.pm_type != PM_NORMAL)
		return qfalse;

	if (ent->client->ps.eFlags & EF_MOUNTEDTANK)
		return qfalse;

	if (ent->waterlevel > 1) 
		return qfalse;

	return qtrue;
}

/*
===================
G_PredictPlayerMove

Advance the given entity frametime seconds, stepping and sliding as appropriate

This is the entry point to the server-side-only prediction code
===================
*/
void G_PredictPlayerMove( gentity_t *ent, float frametime ) {
    if (!G_SkipCorrectionSafe(ent))
        return;

	G_PredictPlayerStepSlideMove( ent, frametime );
}
