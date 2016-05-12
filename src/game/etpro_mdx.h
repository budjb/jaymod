/* 

	Copyright (C) 2003-2005 Christopher Lais (aka "Zinx Verituse")
	and is covered by the following license:

	***
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. Modified source for this software, as used in any binaries you have
	distributed, must be provided on request, free of charge and/or penalty.

	4. This notice may not be removed or altered from any source distribution.
	***

*/

#ifndef GAME_ETPRO_MDX_H
#define GAME_ETPRO_MDX_H

///////////////////////////////////////////////////////////////////////////////

/* This should be somewhere else... it's an etpro cvar, not related to etpro_mdx.* */
#define REALHEAD_HEAD		1
#define REALHEAD_BODY       2   // Probably misnamed, but oh well
#define REALHEAD_BONEHITS	128

#define REALHEAD_DEBUG_HEAD	256
#define REALHEAD_DEBUG_LEGS	512
#define REALHEAD_DEBUG_BODY	1024

/* cut down refEntity_t w/ only stuff needed for player bone calculation */
/* Used only by game code - not engine */
typedef struct {
	qhandle_t	hModel;				// opaque type outside refresh

	vec3_t		headAxis[3];

	// most recent data
	vec3_t		axis[3];		// rotation vectors
	vec3_t		torsoAxis[3];		// rotation vectors for torso section of skeletal animation
//	qboolean	nonNormalizedAxes;	// axis are not normalized, i.e. they have scale
	float		origin[3];
	int			frame;
	qhandle_t	frameModel;
	int			torsoFrame;			// skeletal torso can have frame independant of legs frame
	qhandle_t	torsoFrameModel;

	// previous data for frame interpolation
	float		oldorigin[3];
	int			oldframe;
	qhandle_t	oldframeModel;
	int			oldTorsoFrame;
	qhandle_t	oldTorsoFrameModel;
	float		backlerp;			// 0.0 = current, 1.0 = old
	float		torsoBacklerp;
} grefEntity_t;

extern void mdx_cleanup(void);

extern qhandle_t trap_R_RegisterModel(const char *filename);

extern int trap_R_LerpTagNumber( orientation_t *tag, /*const*/ grefEntity_t *refent, int tagNum );

extern int trap_R_LookupTag( /*const*/ grefEntity_t *refent, const char *tagName );
extern int trap_R_LerpTag( orientation_t *tag, /*const*/ grefEntity_t *refent, const char *tagName, int startIndex );

extern void mdx_head_position(/*const*/ gentity_t *ent, /*const*/ grefEntity_t *refent, vec3_t org);
extern void mdx_legs_position(/*const*/ gentity_t *ent, /*const*/ grefEntity_t *refent, vec3_t org);

extern void mdx_PlayerAngles( gentity_t *ent, vec3_t legsAngles, vec3_t torsoAngles, vec3_t headAngles, qboolean doswing );
extern void mdx_PlayerAnimation( gentity_t *ent );
extern void mdx_gentity_to_grefEntity(gentity_t *ent, grefEntity_t *refent, int lerpTime);

typedef enum {
    MRP_NECK,
    MRP_ELBOW_LEFT,
    MRP_ELBOW_RIGHT,
    MRP_HAND_LEFT,
    MRP_HAND_RIGHT,
    MRP_BACK,
    MRP_CHEST,
    MRP_PELVIS,
    MRP_KNEE_LEFT,
    MRP_KNEE_RIGHT,
    MRP_ANKLE_LEFT,
    MRP_ANKLE_RIGHT,
    MRP_MAX,
} mdx_advanced_position_t;

extern void mdx_advanced_positions ( gentity_t& ent, grefEntity_t& re, vec3_t*, orientation_t* );
extern void mdx_weapon_positions   ( gentity_t& ent, grefEntity_t& re, vec3_t*, orientation_t* );

#ifdef BONE_HITTESTS
enum {
	MDX_NONE,
	MDX_GUN,
	MDX_HEAD,
	MDX_TORSO,
	MDX_ARM_L,
	MDX_ARM_R,
	MDX_LEG_L,
	MDX_LEG_R,
	MDX_HIT_TYPE_MAX
};

extern qhandle_t mdx_RegisterHits(animModelInfo_t *animModelInfo, const char *filename);
extern qboolean mdx_hit_test(const vec3_t start, const vec3_t end, /*const*/ gentity_t *ent, /*const*/ grefEntity_t *refent, int *hit_type, vec_t *fraction, animScriptImpactPoint_t *impactpoint);
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_ETPRO_MDX_H
