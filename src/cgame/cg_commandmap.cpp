#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

// The base zoom for the automap is 5.159
#define BASEMAPZOOM                 5.159f
#define COMMANDMAP_PLAYER_ICON_SIZE 6
#define AUTOMAP_PLAYER_ICON_SIZE    5
#define CONST_ICON_NORMAL_SIZE      32
#define CONST_ICON_EXPANDED_SIZE    48

#define FLAGSIZE_EXPANDED           48
#define FLAGSIZE_NORMAL             32
#define FLAG_LEFTFRAC               0.1953125f // (25/128)
#define FLAG_TOPFRAC                0.7421875f // (95/128)
#define SPAWN_SIZEUPTIME            1000

#define CC_MENU_WIDTH               150

// I am keeping the code around for small icons in a define
//#define SMALL_CM_ICONS

///////////////////////////////////////////////////////////////////////////////

static mapEntList_t mapEntities;
static bool         expanded = false;
static char         cg_highlightText[256];
static rectDef_t    cg_highlightTextRect;
static bool         gridInitDone = false;
static vec2_t       gridStartCoord;
static vec2_t       gridStep;

extern playerInfo_t pi;

///////////////////////////////////////////////////////////////////////////////

void CG_CommandMap_DrawHighlightText( void ) {
	CG_Text_Paint_Ext( cg_highlightTextRect.x, cg_highlightTextRect.y, 0.25f, 0.25f, colorWhite, cg_highlightText, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2 );
	*cg_highlightText = '\0';
}

///////////////////////////////////////////////////////////////////////////////

static void CG_CommandMap_SetHighlightText( const char* text, float x, float y ) {
	Q_strncpyz( cg_highlightText, text, sizeof( cg_highlightText ) );
	cg_highlightTextRect.x = x;
	cg_highlightTextRect.y = y;
	expanded = true;
}

///////////////////////////////////////////////////////////////////////////////

// Cleans up, clears, and initializes the list of map entities
void CG_InitMapEntities()
{
    for (mapEntList_t::iterator i = mapEntities.begin(); i != mapEntities.end(); i++) {
        delete *i;
    }

    mapEntities.clear();
}

///////////////////////////////////////////////////////////////////////////////

void CG_TransformToCommandMapCoord( float *coord_x, float *coord_y ) {
	*coord_x = CC_2D_X + ((*coord_x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * CC_2D_W;
	*coord_y = CC_2D_Y + ((*coord_y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * CC_2D_H;
}

///////////////////////////////////////////////////////////////////////////////

int CG_CurLayerForZ( int z ) {
	int curlayer = 0;

	while( z > cgs.ccLayerCeils[curlayer] && curlayer < cgs.ccLayers )
		curlayer++;

	if( curlayer == cgs.ccLayers ) {
		CG_Printf( "^3Warning: no valid command map layer for z\n" );
		curlayer = 0;
	}

	return curlayer;
}

///////////////////////////////////////////////////////////////////////////////

static bool CG_ScissorEntIsCulled( mapEntityData_t* mEnt, mapScissor_t *scissor, float size ) {
	if( !scissor->circular ) {
		if( mEnt->automapTransformed[0] < scissor->tl[0] ||
			mEnt->automapTransformed[0] > scissor->br[0] ||
			mEnt->automapTransformed[1] < scissor->tl[1] ||
			mEnt->automapTransformed[1] > scissor->br[1] )
			return qtrue;
	} else {
		float distSquared;
		vec2_t distVec;

		distVec[0] = mEnt->automapTransformed[0] - ( scissor->tl[0] + ( 0.5f * ( scissor->br[0] - scissor->tl[0] ) ) );
		distVec[1] = mEnt->automapTransformed[1] - ( scissor->tl[1] + ( 0.5f * ( scissor->br[1] - scissor->tl[1] ) ) );
		distSquared = distVec[0]*distVec[0] + distVec[1]*distVec[1];

        float icon_r = size * 2.0f;
		if( distSquared > Square(0.5f * (scissor->br[0] - scissor->tl[0])) + Square(icon_r) )
			return qtrue;
	}

	return qfalse;
}

///////////////////////////////////////////////////////////////////////////////

static bool CG_ScissorPointIsCulled( vec2_t vec, mapScissor_t *scissor, float tolerance ) {
	if( !scissor->circular ) {
		if( vec[0] < scissor->tl[0]
			|| vec[0] > scissor->br[0]
			|| vec[1] < scissor->tl[1]
			|| vec[1] > scissor->br[1] )
			return qtrue;
	} else {
		float distSquared;
		vec2_t distVec;

		distVec[0] = vec[0] - ( scissor->tl[0] + ( 0.5f * ( scissor->br[0] - scissor->tl[0] ) ) );
		distVec[1] = vec[1] - ( scissor->tl[1] + ( 0.5f * ( scissor->br[1] - scissor->tl[1] ) ) );
		distSquared = distVec[0]*distVec[0] + distVec[1]*distVec[1];

		if( distSquared > Square(0.5f * (scissor->br[0] - scissor->tl[0])) + Square(tolerance * 2.0f) )
			return qtrue;
	}

	return qfalse;
}

///////////////////////////////////////////////////////////////////////////////

// calculate the scaled (zoomed) yet unshifted coordinate for each map entity within the automap
static void CG_TransformAutomapEntity( void )
{
    float width = 100.0f;
    float zoomFactor = cg_mapZoom.value;

    // Alternate HUD adjustments
    if (cg_althud.integer) {
        zoomFactor = max(zoomFactor * CM_ALT_SCALE, 1.0f);
        width *= CM_ALT_SCALE;
    }

    for (mapEntList_t::const_iterator i = mapEntities.begin(); i != mapEntities.end(); i++) {
		mapEntityData_t* mEnt = *i;

		// calculate the screen coordinate of this entity for the automap, consider the zoom value
		mEnt->automapTransformed[0] = (mEnt->x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0] * width * zoomFactor;
		mEnt->automapTransformed[1] = (mEnt->y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1] * width * zoomFactor;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CG_AdjustAutomapZoom(int zoomIn)
{
    float mapZoom = cg_mapZoom.value;
	if (zoomIn) {
		mapZoom *= 1.2;
		if (mapZoom > 7.43)  // approximately 1.2^11
			mapZoom = 7.43;
	} else {
		mapZoom /= 1.2;
		// zoom value of 1 corresponds to the most zoomed out view. The whole map is displayed 
		// in the automap
		if ( mapZoom < 1)
			mapZoom = 1;
	}
    trap_Cvar_Set("cg_mapZoom", va("%.3f", mapZoom));

	// recalculate the screen coordinates since the zoom changed
	CG_TransformAutomapEntity();
}

///////////////////////////////////////////////////////////////////////////////

static void CG_ParseMapEntity( stringstream& entList, team_t team ) {

	mapEntityData_t* mEnt = new mapEntityData_t;

    entList >> mEnt->type;

	switch( mEnt->type ) {
        case ME_PLAYER:
        case ME_PLAYER_REVIVE:
        case ME_PLAYER_DISGUISED:
            entList >> hex >> mEnt->yaw >> dec;
            mEnt->yaw = mEnt->yaw * 24 - 180;
            break;

		case ME_CONSTRUCT:
		case ME_DESTRUCT:
		case ME_DESTRUCT_2:
		case ME_COMMANDMAP_MARKER:
		case ME_TANK:
		case ME_TANK_DEAD:
        case ME_LANDMINE:
            entList >> mEnt->x;
            mEnt->x *= 128;

            entList >> mEnt->y;
            mEnt->y *= 128;

			if( cgs.ccLayers ) {
                entList >> mEnt->z;
                mEnt->z *= 128;
			}
			break;

		default:
            entList >> mEnt->x;
            mEnt->x *= 128;

            entList >> mEnt->y;
            mEnt->y *= 128;

			if( cgs.ccLayers ) {
                entList >> mEnt->z;
                mEnt->z *= 128;
			}

            entList >> hex >> mEnt->yaw >> dec;
            mEnt->yaw = mEnt->yaw * 24 - 180;
			break;
	}

    entList >> mEnt->data;

	mEnt->transformed[0] = (mEnt->x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0] * CC_2D_W;
	mEnt->transformed[1] = (mEnt->y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1] * CC_2D_H;

	mEnt->team = team;

    mapEntities.push_back(mEnt);
}

///////////////////////////////////////////////////////////////////////////////

// Entry point from server command to parse map entities
void CG_ParseMapEntityInfo( int axis_number, int allied_number, bool reset ) {
	int i;

    if (reset) {
	    CG_InitMapEntities();
    }

    stringstream entList;
    entList.str(CG_Argv(3));

	for( i = 0; i < axis_number; i++ ) {
		CG_ParseMapEntity( entList, TEAM_AXIS );
	}

	for(i = 0; i < allied_number; i++) {
		CG_ParseMapEntity( entList, TEAM_ALLIES );
	}

	CG_TransformAutomapEntity();
}

///////////////////////////////////////////////////////////////////////////////

static bool CG_CircularPointIsCulled(float x, float y, float w, float h, vec2_t point)
{
    float cx = x + w / 2;
    float cy = y + h / 2;
    float ix = point[0];
    float iy = point[1];

    float xd = cx - ix;
    float yd = cy - iy;

    if (Square(xd) + Square(yd) > Square(w/2))
        return true;

    return false;
}

///////////////////////////////////////////////////////////////////////////////

static void CG_DrawGrid( float x, float y, float w, float h, mapScissor_t *scissor )
{
	vec2_t step;
	vec2_t dim_x, dim_y;
	vec4_t line;
	float xscale, yscale;
	float grid_x, grid_y;
	vec2_t dist;
	vec4_t gridColour;

	dist[0] = cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0];
	dist[1] = cg.mapcoordsMaxs[1] - cg.mapcoordsMins[1];

	if( !gridInitDone ) {
		gridStep[0] = 1200.f;
		gridStep[1] = 1200.f;

		// ensure minimal grid density
		while( ( cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0] ) / gridStep[0] < 7 )
			gridStep[0] -= 50.f;
		while( ( cg.mapcoordsMins[1] - cg.mapcoordsMaxs[1] ) / gridStep[1] < 7 )
			gridStep[1] -= 50.f;

		gridStartCoord[0] = .5f * ( ( ( ( cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0] ) / gridStep[0] ) - ( (int)(( cg.mapcoordsMaxs[0] - cg.mapcoordsMins[0] ) / gridStep[0] ) ) ) * gridStep[0] );
		gridStartCoord[1] = .5f * ( ( ( ( cg.mapcoordsMins[1] - cg.mapcoordsMaxs[1] ) / gridStep[1] ) - ( (int)(( cg.mapcoordsMins[1] - cg.mapcoordsMaxs[1] ) / gridStep[1] ) ) ) * gridStep[1] );

		gridInitDone = true;
	}

	if( scissor ) {
		dim_x[0] = cg.mapcoordsMins[0];
		dim_x[1] = cg.mapcoordsMaxs[0];

		dim_y[0] = cg.mapcoordsMaxs[1];
		dim_y[1] = cg.mapcoordsMins[1];

		// transform
		xscale = ( w * scissor->zoomFactor ) / dist[0];
		yscale = ( h * scissor->zoomFactor ) / -dist[1];

		dim_x[0] = (dim_x[0] - cg.mapcoordsMins[0]) * xscale;
		dim_x[1] = (dim_x[1] - cg.mapcoordsMins[0]) * xscale;

		dim_y[0] = (dim_y[0] - cg.mapcoordsMaxs[1]) * yscale;
		dim_y[1] = (dim_y[1] - cg.mapcoordsMaxs[1]) * yscale;

		grid_x = ((gridStartCoord[0] / dist[0]) * w * scissor->zoomFactor) - scissor->tl[0];
		grid_y = ((-gridStartCoord[1] / dist[1]) * h * scissor->zoomFactor) - scissor->tl[1];

		step[0] = gridStep[0] * xscale;
		step[1] = gridStep[1] * yscale;

		// draw
		Vector4Set( gridColour, clrBrownLine[0], clrBrownLine[1], clrBrownLine[2], .4f );
		trap_R_SetColor( gridColour );
		for( ; grid_x < dim_x[1]; grid_x += step[0] )
		{
			if( grid_x < dim_x[0] )
				continue;

			if( grid_x > w )
				break;

			if( scissor->circular ) {
				// clip line against circle
				float xc, yc;

				line[0] = x + grid_x;
				xc = line[0] >= x + .5f * w ? line[0] - (x + .5f * w) : (x + .5f * w) - line[0];
				yc = SQRTFAST( Square(.5f * w) - Square(xc) );
				line[1] = y + (.5f * h) - yc;
				line[2] = 1.f;
				line[3] = 2 * yc;
			} else {
				Vector4Set( line, x + grid_x, y + dim_y[0], 1.f, h );
			}
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[3] *= cgs.screenYScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}

		for( ; grid_y < dim_y[1]; grid_y += step[1] )
		{
			if( grid_y < dim_y[0] )
				continue;

			if( grid_y > h )
				break;

			if( scissor->circular ) {
				// clip line against circle
				float xc, yc;

				line[1] = y + grid_y;
				yc = line[1] >= y + .5f * h ? line[1] - (y + .5f * h) : (y + .5f * h) - line[1];
				xc = SQRTFAST( Square(.5f * h) - Square(yc) );
				line[0] = x + (.5f * w) - xc;
				line[2] = 2 * xc;
				line[3] = 1.f;
			} else {
				Vector4Set( line, x + dim_x[0], y + grid_y, w, 1 );
			}			
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[2] *= cgs.screenXScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}
		trap_R_SetColor( NULL );
	} else {
		char coord_char[3], coord_int;
		float text_width, text_height;
		vec2_t textOrigin;

		dim_x[0] = cg.mapcoordsMins[0];
		dim_x[1] = cg.mapcoordsMaxs[0];

		dim_y[0] = cg.mapcoordsMaxs[1];
		dim_y[1] = cg.mapcoordsMins[1];

		// transform
		xscale = w / dist[0];
		yscale = h / -dist[1];

		dim_x[0] = (dim_x[0] - cg.mapcoordsMins[0]) * xscale;
		dim_x[1] = (dim_x[1] - cg.mapcoordsMins[0]) * xscale;

		dim_y[0] = (dim_y[0] - cg.mapcoordsMaxs[1]) * yscale;
		dim_y[1] = (dim_y[1] - cg.mapcoordsMaxs[1]) * yscale;

		grid_x = gridStartCoord[0] * xscale;
		grid_y = gridStartCoord[1] * yscale;

		step[0] = gridStep[0] * xscale;
		step[1] = gridStep[1] * yscale;

		// draw
		textOrigin[0] = grid_x;
		textOrigin[1] = grid_y;

		Vector4Set( gridColour, clrBrownLine[0], clrBrownLine[1], clrBrownLine[2], 1.f );

		coord_char[1] = '\0';
		for( coord_char[0] = ('A' - 1); grid_x < dim_x[1]; grid_x += step[0], coord_char[0]++ )
		{
			if( coord_char[0] >= 'A' ) {
				text_width = CG_Text_Width_Ext( coord_char, 0.2f, 0, &cgs.media.limboFont2 );
				text_height = CG_Text_Height_Ext( coord_char, 0.2f, 0, &cgs.media.limboFont2 );
				CG_Text_Paint_Ext( ( x + grid_x ) - (.5f * step[0]) - (.5f * text_width), y + dim_y[0] + textOrigin[1] + 1.5f * text_height, 0.2f, 0.2f, colorBlack, coord_char, 0, 0, 0, &cgs.media.limboFont2 );
			}
			trap_R_SetColor( gridColour );

			Vector4Set( line, x + grid_x, y + dim_y[0], 1, dim_x[1] - dim_x[0] );
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[3] *= cgs.screenYScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}

		for( coord_int = -1; grid_y < dim_y[1]; grid_y += step[1], coord_int++ )
		{
			if( coord_int >= 0 ) {
				Com_sprintf( coord_char, sizeof(coord_char), "%i", coord_int );
				text_width = CG_Text_Width_Ext( "0", 0.2f, 0, &cgs.media.limboFont2 );
				text_height = CG_Text_Height_Ext( coord_char, 0.2f, 0, &cgs.media.limboFont2 );
				CG_Text_Paint_Ext( x + dim_x[0] + textOrigin[0] + .5f * text_width, (y + grid_y) - (.5f * step[1]) + (.5f * text_height), 0.2f, 0.2f, colorBlack, coord_char, 0, 0, 0, &cgs.media.limboFont2 );
			}
			trap_R_SetColor( gridColour );

			Vector4Set( line, x + dim_x[0], y + grid_y, dim_y[1] - dim_y[0], 1 );
			line[0] *= cgs.screenXScale;
			line[1] *= cgs.screenYScale;
			line[2] *= cgs.screenXScale;
			trap_R_DrawStretchPic( line[0], line[1], line[2], line[3], 0, 0, 0, 1, cgs.media.whiteShader );
		}
		trap_R_SetColor( NULL );
	}
}

///////////////////////////////////////////////////////////////////////////////

static void CG_DrawPlayerEntity( mapEntityData_t* mEnt, float x, float y, float w, float h, int mEntFilter, mapScissor_t* scissor, bool interactive, snapshot_t* snap, int icon_size )
{
    vec2_t string_pos = { 0, 0 };

    // Do not draw if their clientinfo is not valid
    clientInfo_t* ci = &cgs.clientinfo[mEnt->data];
	if(!ci->infoValid) {
		return;
	}

    // Enforce filters
	if(ci->team == TEAM_AXIS) {
		if(mEntFilter & CC_FILTER_AXIS) {
			return;
		}
	} else if(ci->team == TEAM_ALLIES) {
		if(mEntFilter & CC_FILTER_ALLIES) {
			return;
		}
	} else {
		return;
	}

	centity_t* cent = &cg_entities[mEnt->data];

	if( mEnt->type == ME_PLAYER_DISGUISED && !(cent->currentState.powerups & (1 << PW_OPS_DISGUISED)) ) {
		return;
	}

	bg_playerclass_t* classInfo = CG_PlayerClassForClientinfo(ci, cent);

	// For these, if availaible, ignore the coordinate data and grab the most up to date pvs data
	if (cent-cg_entities == cg.clientNum) {
        // Use our own lerp'ed origin
		if( !scissor ) {
			mEnt->transformed[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
			mEnt->transformed[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
		} else {
			mEnt->automapTransformed[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
			mEnt->automapTransformed[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
		}
		
		mEnt->yaw = (int)cg.predictedPlayerState.viewangles[YAW];
    } else if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && snap->ps.clientNum != cg.clientNum && cent-cg_entities == snap->ps.clientNum) {
        // We're following someone, so use their info
		if( !scissor ) {
			mEnt->transformed[0] = ((snap->ps.origin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
			mEnt->transformed[1] = ((snap->ps.origin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
		} else {
			mEnt->automapTransformed[0] = ((snap->ps.origin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
			mEnt->automapTransformed[1] = ((snap->ps.origin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
		}
		
		mEnt->yaw = (int)snap->ps.viewangles[YAW];
	} else if( cent->currentValid ) {
        // Use more up-to-date info from pvs
		if( !scissor ) {
			mEnt->transformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
			mEnt->transformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
		} else {
			mEnt->automapTransformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
			mEnt->automapTransformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
		}

		mEnt->yaw = (int)cent->lerpAngles[YAW];
	} else {
        // Only drawing those revivable teammates that are near you.
        if(mEnt->type == ME_PLAYER_REVIVE) {
			return;
		}

        // Use the coordinates from clientinfo
		if( !scissor ) {
			mEnt->transformed[0] = ((ci->location[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
			mEnt->transformed[1] = ((ci->location[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
		} else {
			mEnt->automapTransformed[0] = ((ci->location[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
			mEnt->automapTransformed[1] = ((ci->location[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
		}
	}

    // Enforce layer
	if( cgs.ccLayers ) {
		if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
            return;
	}

    // Set up icon coords
    vec2_t icon_pos;
    vec2_t icon_extends;
	if( scissor ) {
		icon_pos[0] = mEnt->automapTransformed[0] - scissor->tl[0] + x - (icon_size * scissor->zoomFactor / BASEMAPZOOM);
		icon_pos[1] = mEnt->automapTransformed[1] - scissor->tl[1] + y - (icon_size * scissor->zoomFactor / BASEMAPZOOM);
	} else { 
		icon_pos[0] = x + mEnt->transformed[0] - icon_size;
		icon_pos[1] = y + mEnt->transformed[1] - icon_size;
		string_pos[0] = x + mEnt->transformed[0];
		string_pos[1] = y + mEnt->transformed[1] + icon_size;
	}
	icon_extends[0] = 2 * icon_size;
	icon_extends[1] = 2 * icon_size;

#ifdef SMALL_CM_ICONS
	if( scissor ) {
		icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
		icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
	}
#endif

	// Check for culling
	if( scissor && CG_ScissorEntIsCulled( mEnt, scissor, icon_extends[0] ) ) {
        trap_R_SetColor( NULL );
		return;
	}

    // Revive icons do not get the normal player icon
	if ( mEnt->type == ME_PLAYER_REVIVE ) {
		float msec;
		vec4_t reviveClr = { 1.f, 1.f, 1.f, 1.f };

		if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_AXIS) {
			msec = (cg_redlimbotime.integer - (cg.time%cg_redlimbotime.integer)) / (float)cg_redlimbotime.integer;
		} else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_ALLIES) {
			msec = (cg_bluelimbotime.integer - (cg.time%cg_bluelimbotime.integer)) / (float)cg_bluelimbotime.integer;
		} else {
			msec = 0;
		}

		reviveClr[3] = .5f + .5f * ((sin(sqrt(msec) * 25 * 2 * M_PI) + 1) * .5f);

		trap_R_SetColor( reviveClr );
		CG_DrawPic( icon_pos[0] + 3, icon_pos[1] + 3, icon_extends[0] - 3, icon_extends[1] - 3, cgs.media.medicIconCM);
		trap_R_SetColor( NULL );

        return;
	}

    // Certain cases will draw a voice chat icon, others will not
    bool      drawVoiceChat = false;
    qhandle_t voiceChatIcon = 0;

    // Draw player highlights and set up voice chats
	if (cg.clientNum == mEnt->data || (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && snap->ps.clientNum != cg.clientNum && cent-cg_entities == snap->ps.clientNum)) {

        // Draw a yellow highlight for ourself
		trap_R_SetColor( colorYellow );
		CG_DrawPic(icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
		trap_R_SetColor( NULL );

        // Voice chat icon
		if (cg.predictedPlayerEntity.voiceChatSpriteTime > cg.time) {
            drawVoiceChat = true;
            voiceChatIcon = cg.predictedPlayerEntity.voiceChatSprite;
        }
	} else if (CG_IsOnSameFireteam( cg.clientNum, mEnt->data)) {

        // Fireteam players are white
        CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
	
        // Draw the player's name in the expanded command map
		if (!scissor) {
			CG_Text_Paint_Ext( string_pos[0], string_pos[1], 0.2f, 0.2f, colorWhite, ci->name, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2 );
		}

        // Voice chat icon
		if(cent->voiceChatSpriteTime > cg.time) {
            drawVoiceChat = true;
            voiceChatIcon = cent->voiceChatSprite;
		}
	} else if (ci->team == snap->ps.persistant[PERS_TEAM]) {

        // Voice chat icon
		if (cent->voiceChatSpriteTime > cg.time) {
            drawVoiceChat = true;
            voiceChatIcon = cent->voiceChatSprite;
		}
	}

    // Draw the voice chat icon
    if (drawVoiceChat) {
        float offset = 12;
        bool draw = true;

        // Jaybird - check for culling
        if (scissor) {
            offset *= CM_ALT_SCALE;

            vec2_t point;
            point[0] = icon_pos[0] + offset + icon_extends[0] * 0.25f;
            point[1] = icon_pos[1] + icon_extends[1] * 0.25f;

            if (CG_CircularPointIsCulled(x, y, w, h, point)) {
                draw = false;
            }
        }

        if (draw) {
			CG_DrawPic( icon_pos[0] + offset, icon_pos[1], icon_extends[0] * 0.5f, icon_extends[1] * 0.5f, voiceChatIcon);
        }
	}

    // Draw the player class icon
	CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], classInfo->icon);

	// Draw overlays
	if(ci->team == snap->ps.persistant[PERS_TEAM] ) {
		if( cent->currentState.powerups & ( 1 << PW_REDFLAG  ) || cent->currentState.powerups & ( 1 << PW_BLUEFLAG )) {
			CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.objectiveShaderCM );
		}

		if( cent->currentState.powerups & ( 1 << PW_OPS_DISGUISED )) {
			CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.friendShader );
		}
	}

    // Draw the arrow
	CG_DrawRotatedPic( icon_pos[0]-1, icon_pos[1]-1, icon_extends[0]+2, icon_extends[1]+2, classInfo->arrow, (0.5 - ( mEnt->yaw - 180.f ) / 360.f));	
}

static void CG_DrawMapEntity( mapEntityData_t *mEnt, float x, float y, float w, float h, int mEntFilter, mapScissor_t *scissor, qboolean interactive, snapshot_t* snap, int icon_size ) {
	int j = 1;
	qhandle_t pic;
	centity_t *cent;
	const char* name;
	vec4_t c_clr = {1.f, 1.f, 1.f, 1.f};
	vec2_t icon_extends, icon_pos;
	int customimage = 0;
	oidInfo_t* oidInfo = NULL;

	switch( mEnt->type ) {
	case ME_PLAYER_DISGUISED:
	case ME_PLAYER_REVIVE:
	case ME_PLAYER:
        CG_DrawPlayerEntity(mEnt, x, y, w, h, mEntFilter, scissor, interactive, snap, icon_size);
		return;
	case ME_CONSTRUCT:
	case ME_DESTRUCT:
	case ME_DESTRUCT_2:
	case ME_TANK:
	case ME_TANK_DEAD:
	case ME_COMMANDMAP_MARKER:
		cent = NULL;
		if( mEnt->type == ME_TANK || mEnt->type == ME_TANK_DEAD ) {
			oidInfo = &cgs.oidInfo[ mEnt->data ];

			for( j = 0; j < cg.snap->numEntities; j++ ) {
				if( cg.snap->entities[j].eType == ET_OID_TRIGGER && cg.snap->entities[j].teamNum == mEnt->data ) {
					cent = &cg_entities[cg.snap->entities[j].number];
					if( !scissor ) {
						mEnt->transformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
						mEnt->transformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
					} else {
						mEnt->automapTransformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
						mEnt->automapTransformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
					}
					break;
				}
			}
		} else if( mEnt->type == ME_CONSTRUCT || mEnt->type == ME_DESTRUCT || mEnt->type == ME_DESTRUCT_2 ) {
			cent = &cg_entities[mEnt->data];

			oidInfo = &cgs.oidInfo[ cent->currentState.modelindex2 ];

			if( !scissor ) {
				mEnt->transformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
				mEnt->transformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
			} else {
				mEnt->automapTransformed[0] = ((cent->lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
				mEnt->automapTransformed[1] = ((cent->lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
			}
		} else if( mEnt->type == ME_COMMANDMAP_MARKER ) {
			oidInfo = &cgs.oidInfo[ mEnt->data ];

			if( !scissor ) {
				mEnt->transformed[0] = ((oidInfo->origin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
				mEnt->transformed[1] = ((oidInfo->origin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
			} else {
				mEnt->automapTransformed[0] = ((oidInfo->origin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
				mEnt->automapTransformed[1] = ((oidInfo->origin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
			}
		}

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
                return;
		}

		if( oidInfo ) {
			customimage = mEnt->team == TEAM_AXIS ? oidInfo->customimageaxis : oidInfo->customimageallies;
		}

/*		if((mEnt->yaw & 0xFF) & (1 << (atoi(CG_ConfigString(mEnt->team == TEAM_AXIS ? CS_MAIN_AXIS_OBJECTIVE : CS_MAIN_ALLIES_OBJECTIVE)) - 1))) {
			trap_R_SetColor( colorYellow );
		}*/

		if(mEnt->type == ME_CONSTRUCT) {
			if(mEntFilter & CC_FILTER_CONSTRUCTIONS) {
				return;
			}
			pic = mEnt->team == TEAM_AXIS ? cgs.media.ccConstructIcon[0] : cgs.media.ccConstructIcon[1];
		} else if(mEnt->type == ME_TANK) {
			if(mEntFilter & CC_FILTER_OBJECTIVES) {
				return;
			}
			pic = cgs.media.ccTankIcon;
		} else if(mEnt->type == ME_TANK_DEAD) {
			if(mEntFilter & CC_FILTER_OBJECTIVES) {
				return;
			}
			pic = cgs.media.ccTankIcon;
			trap_R_SetColor( colorRed );
		} else if( mEnt->type == ME_COMMANDMAP_MARKER ) {
			pic = 0;
		} else if( mEnt->type == ME_DESTRUCT_2 ) {
			pic = 0;
		} else {
			if( mEntFilter & CC_FILTER_DESTRUCTIONS ) {
				return;
			}
			pic = mEnt->team == TEAM_AXIS ? cgs.media.ccDestructIcon[cent->currentState.effect1Time][0] : cgs.media.ccDestructIcon[cent->currentState.effect1Time][1];
		}

		{
			int info = 0;

			if( oidInfo ) {
				info = oidInfo->spawnflags;
			}

			if( info & (1 << 4) ) {
				if( mEntFilter & CC_FILTER_OBJECTIVES ) {
					return;
				}
			}
			if( info & (1 << 5) ) {
				if( mEntFilter & CC_FILTER_HACABINETS ) {
					return;
				}
			}
			if( info & (1 << 6) ) {
				if( mEnt->type == ME_DESTRUCT_2 ) {
					pic = mEnt->team == TEAM_AXIS ? cgs.media.ccCmdPost[0] : cgs.media.ccCmdPost[1];
				}
				if( mEntFilter & CC_FILTER_CMDPOST ) {
					return;
				}
			}
		}

		if( customimage ) {
			pic = customimage;
		}

		if( scissor ) {
			icon_pos[0] = mEnt->automapTransformed[0] - scissor->tl[0] + x;
			icon_pos[1] = mEnt->automapTransformed[1] - scissor->tl[1] + y;
		} else {
			icon_pos[0] = x + mEnt->transformed[0];
			icon_pos[1] = y + mEnt->transformed[1];
		}

		if( interactive && !expanded && BG_RectContainsPoint( x + mEnt->transformed[0] - (CONST_ICON_NORMAL_SIZE*0.5f), y + mEnt->transformed[1] - (CONST_ICON_NORMAL_SIZE*0.5f), CONST_ICON_NORMAL_SIZE, CONST_ICON_NORMAL_SIZE, cgDC.cursorx, cgDC.cursory )) {
			float w;

			icon_extends[0] = CONST_ICON_EXPANDED_SIZE;
			icon_extends[1] = CONST_ICON_EXPANDED_SIZE;
			if( mEnt->type == ME_TANK_DEAD || mEnt->type == ME_TANK ) {
				icon_extends[1] *= 0.5f;
			}

#ifdef SMALL_CM_ICONS
			if( scissor ) {
				icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
				icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
			} else {
#endif
				icon_extends[0] *= cgs.ccZoomFactor;
				icon_extends[1] *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
			}
#endif

		    // now check to see if the entity is within our clip region
		    if( scissor && CG_ScissorEntIsCulled( mEnt, scissor, icon_extends[0] ) ) {
                trap_R_SetColor( NULL );
			    return;
		    }

			CG_DrawPic( icon_pos[0] - (icon_extends[0]*0.5f), icon_pos[1] - (icon_extends[1]*0.5f), icon_extends[0], icon_extends[1], pic );

			if( oidInfo ) {
				name = oidInfo->name;
			} else {
				name = va( "%i", j );
			}

			w = CG_Text_Width_Ext( name, 0.2f, 0, &cgs.media.limboFont2 );
			CG_CommandMap_SetHighlightText( name, icon_pos[0] - ( w * 0.5f ), icon_pos[1] - 8 );
		} else if ( interactive && (mEnt->yaw & 0xFF) & (1 << cgs.ccSelectedObjective) ) {
			float scalesize;
			int time = cg.time % 1400;

			if(time <= 700) {
				scalesize = 12 * (time) / 700.f;
			} else {
				scalesize = 12 * (1 - ((time - 700) / 700.f));
			}

			icon_extends[0] = CONST_ICON_NORMAL_SIZE + scalesize;
			icon_extends[1] = CONST_ICON_NORMAL_SIZE + scalesize;

#ifdef SMALL_CM_ICONS
			if( scissor ) {
				icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
				icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
			} else {
#endif
				icon_extends[0] *= cgs.ccZoomFactor;
				icon_extends[1] *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
			}
#endif

			if( mEnt->type == ME_TANK_DEAD || mEnt->type == ME_TANK ) {
				icon_extends[1] *= 0.5f;
			}

		    // now check to see if the entity is within our clip region
		    if( scissor && CG_ScissorEntIsCulled( mEnt, scissor, icon_extends[0] ) ) {
                trap_R_SetColor( NULL );
			    return;
		    }

			CG_DrawPic( icon_pos[0] - (icon_extends[0]*0.5f), icon_pos[1] - (icon_extends[1]*0.5f), icon_extends[0], icon_extends[1], pic );
		} else {
			icon_extends[0] = CONST_ICON_NORMAL_SIZE;
			icon_extends[1] = CONST_ICON_NORMAL_SIZE;
			if( mEnt->type == ME_TANK_DEAD || mEnt->type == ME_TANK ) {
				icon_extends[1] *= 0.5f;
			}
			
#ifdef SMALL_CM_ICONS
            if( scissor ) {
				icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
				icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
			} else {
#endif
				icon_extends[0] *= cgs.ccZoomFactor;
				icon_extends[1] *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
			}
#endif

		    // now check to see if the entity is within our clip region
		    if( scissor && CG_ScissorEntIsCulled( mEnt, scissor, icon_extends[0] ) ) {
                trap_R_SetColor( NULL );
			    return;
		    }

			CG_DrawPic( icon_pos[0] - (icon_extends[0]*0.5f), icon_pos[1] - (icon_extends[1]*0.5f), icon_extends[0], icon_extends[1], pic );
		}
		trap_R_SetColor( NULL );
		return;
	case ME_LANDMINE:
/*			if(mEntFilter & CC_FILTER_LANDMINES) {
			continue;
		}*/

        if( scissor) {
            mEnt->automapTransformed[0] = (mEnt->x - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0] * w * scissor->zoomFactor;
            mEnt->automapTransformed[1] = (mEnt->y - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1] * h * scissor->zoomFactor;
        }
        
		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( mEnt->z ) != cgs.ccSelectedLayer )
                return;
		}

		if( mEnt->data == TEAM_AXIS ) {
			pic = cgs.media.commandCentreAxisMineShader;
		} else { // TEAM_ALLIES
			pic = cgs.media.commandCentreAlliedMineShader;
		}			

		c_clr[3] = 1.0f;

		if( scissor ) {
			icon_pos[0] = mEnt->automapTransformed[0] - scissor->tl[0] + x;
			icon_pos[1] = mEnt->automapTransformed[1] - scissor->tl[1] + y;
		} else {
			icon_pos[0] = x + mEnt->transformed[0];
			icon_pos[1] = y + mEnt->transformed[1];
		}

		icon_extends[0] = 12;
		icon_extends[1] = 12;

#ifdef SMALL_CM_ICONS
		if( scissor ) {
			icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
			icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
		} else {
#endif
			icon_extends[0] *= cgs.ccZoomFactor;
			icon_extends[1] *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
		}
#endif

        // now check to see if the entity is within our clip region
		if( scissor && CG_ScissorEntIsCulled( mEnt, scissor, icon_extends[0] ) ) {
            trap_R_SetColor( NULL );
			return;
		}

		trap_R_SetColor(c_clr);
		CG_DrawPic( icon_pos[0] - icon_extends[0]*0.5f, icon_pos[1] - icon_extends[1]*0.5f, icon_extends[0], icon_extends[1], pic);
		trap_R_SetColor(NULL);

		return;
	default:
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////

void CG_DrawMortarMarker( int px, int py, int pw, int ph, qboolean draw, mapScissor_t *scissor, int expand ) {
	
	float size = 16.f;
	if (scissor) {
		size *= CM_ALT_SCALE;
	}

	if( cg.lastFiredWeapon == WP_MORTAR_SET && cg.mortarImpactTime >= 0 ) {
		if( cg.snap->ps.weapon != WP_MORTAR_SET ) {
			cg.mortarImpactTime = 0;
		} else {

			//vec4_t colour = { .77f, .1f, .1f, 1.f };
			vec4_t colour = { 1.f, 1.f, 1.f, 1.f };
			vec3_t point;
//			int fadeTime = 0; // cg.time - (cg.predictedPlayerEntity.muzzleFlashTime + 5000);

			if( scissor ) {
				point[0] = ((cg.mortarImpactPos[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw * scissor->zoomFactor;
				point[1] = ((cg.mortarImpactPos[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph * scissor->zoomFactor;
			} else {
				point[0] = px + (((cg.mortarImpactPos[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw );
				point[1] = py + (((cg.mortarImpactPos[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph );
			}

			// rain - don't return if the marker is culled, just don't
			// draw it.
			if (!(scissor && CG_ScissorPointIsCulled( point, scissor, size ))) {
				if( scissor ) {
					point[0] += px - scissor->tl[0];
					point[1] += py - scissor->tl[1];
				}

				if( cg.mortarImpactOutOfMap ) {
					if( !scissor ) {
						// near the edge of the map, fit into it
						if( point[0] + (size/2) > (px + pw) )
							point[0] -= (size/2);
						else if( point[0] - (size/2) < px )
							point[0] += (size/2);

						if( point[1] + (size/2) > (py + ph) )
							point[1] -= (size/2);
						else if( point[1] - (size/2) < py )
							point[1] += (size/2);
					}
					colour[3] = .5f;
				}

				trap_R_SetColor( colour );
				CG_DrawRotatedPic( point[0] - (size/2), point[1] - (size/2), size, size, cgs.media.ccMortarHit, .5f - ( cg.mortarFireAngles[YAW] /*- 180.f */+ 45.f ) / 360.f );
				trap_R_SetColor( NULL );
			}
		}
	}

	if( COM_BitCheck( cg.snap->ps.weapons, WP_MORTAR_SET ) ) {
		int i;

		for( i = 0; i < MAX_CLIENTS; i++ ) {
			//vec4_t colour = { .23f, 1.f, .23f, 1.f };
			vec4_t colour = { 1.f, 1.f, 1.f, 1.f };
			vec3_t point;
			int fadeTime = cg.time - (cg.artilleryRequestTime[i] + 25000);

			if( fadeTime < 5000 ) {
				if( fadeTime > 0 ) {
					colour[3] = 1.f - (fadeTime/5000.f);
				}

				if( scissor ) {
					point[0] = ((cg.artilleryRequestPos[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw * scissor->zoomFactor;
					point[1] = ((cg.artilleryRequestPos[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph * scissor->zoomFactor;
				} else {
					point[0] = px + (((cg.artilleryRequestPos[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw );
					point[1] = py + (((cg.artilleryRequestPos[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph );
				}

				// rain - don't return if the marker is culled, just skip
				// it (so we draw the rest, if any)
				if( scissor && CG_ScissorPointIsCulled( point, scissor, size ) ) {
					continue;
				}

				if( scissor ) {
					point[0] += px - scissor->tl[0];
					point[1] += py - scissor->tl[1];
				}

				trap_R_SetColor( colour );
				CG_DrawPic( point[0] - (size/2), point[1] - (size/2), size, size, cgs.media.ccMortarTargetCM );
				trap_R_SetColor( NULL );

				//CG_FillRect( point[0] - 8.f, point[1] - 8.f, 16, 16, colour );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

int CG_DrawSpawnPointInfo( int px, int py, int pw, int ph, qboolean draw, mapScissor_t *scissor, int expand ) {
	int i;
	char buffer[64];
	vec2_t point;
	int e = -1;
	vec2_t icon_extends;

	team_t team = CG_LimboPanel_GetRealTeam();

	if( cgs.ccFilter & CC_FILTER_SPAWNS ) {
		return -1;
	}

	for ( i = 1; i < cg.spawnCount; i++ ) {
		float changetime = 0;
		if(cg.spawnTeams_changeTime[i]) {
			changetime = (cg.time - cg.spawnTeams_changeTime[i]);
			if(changetime > SPAWN_SIZEUPTIME || changetime < 0) {
				changetime = cg.spawnTeams_changeTime[i] = 0;
			}
		}

		// rain - added parens around ambiguity
		if (((cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR) &&
			(cg.spawnTeams[i] != team)) ||
			((cg.spawnTeams[i] & 256) && !changetime)) {

			continue;
		}

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( (int)cg.spawnCoords[i][2] ) != cgs.ccSelectedLayer )
				break;
		}

		if( scissor ) {
			point[0] = ((cg.spawnCoordsUntransformed[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw * scissor->zoomFactor;
			point[1] = ((cg.spawnCoordsUntransformed[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph * scissor->zoomFactor;
		} else {
			point[0] = px + (((cg.spawnCoordsUntransformed[i][0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * pw );
			point[1] = py + (((cg.spawnCoordsUntransformed[i][1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * ph );
		}

		icon_extends[0] = FLAGSIZE_NORMAL;
		icon_extends[1] = FLAGSIZE_NORMAL;

#ifdef SMALL_CM_ICONS
		if( scissor ) {
			icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
			icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
		} else {
#endif
			icon_extends[0] *= cgs.ccZoomFactor;
			icon_extends[1] *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
		}
#endif

        // This is moved here because I need to know the extents of the icon in order to cull it properly
		if( scissor && CG_ScissorPointIsCulled( point, scissor, icon_extends[0] ) ) {
			continue;
		}

		if( scissor ) {
			point[0] += px - scissor->tl[0];
			point[1] += py - scissor->tl[1];
		}

		point[0] -= (icon_extends[0] * (39 / 128.f));
		point[1] += (icon_extends[1] * (31 / 128.f));

		if( changetime ) {
			if( draw ) {
				float size;

				if(cg.spawnTeams[i] == team) {
					size = 20 * (changetime / SPAWN_SIZEUPTIME);
				} else {
					size = 20 * (1 - (changetime / SPAWN_SIZEUPTIME));
				}

#ifdef SMALL_CM_ICONS
				if( scissor ) {
					size *= (scissor->zoomFactor / BASEMAPZOOM);
				} else {
#endif
					size *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
				}
#endif

				CG_DrawPic( point[0] - FLAG_LEFTFRAC*size, point[1] - FLAG_TOPFRAC*size, size, size, cgs.media.commandCentreSpawnShader[  cg.spawnTeams[i] == TEAM_AXIS ? 0 : 1 ] );
			}
		} else if( (draw && i == expand) || (!expanded && BG_RectContainsPoint( point[0] - FLAGSIZE_NORMAL*0.5f, point[1] - FLAGSIZE_NORMAL*0.5f, FLAGSIZE_NORMAL, FLAGSIZE_NORMAL, cgDC.cursorx, cgDC.cursory )) ) {
			if( draw ) {
				float size = FLAGSIZE_EXPANDED;

#ifdef SMALL_CM_ICONS
				if( scissor ) {
					size *= (scissor->zoomFactor / BASEMAPZOOM);
				} else {
#endif
					size *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
				}
#endif

				CG_DrawPic( point[0] - FLAG_LEFTFRAC*size, point[1] - FLAG_TOPFRAC*size, size, size, cgs.media.commandCentreSpawnShader[  cg.spawnTeams[i] == TEAM_AXIS ? 0 : 1 ] );
			} else {			
				if( !scissor ) {
					float w;

					Com_sprintf( buffer, sizeof( buffer ), "%s (Troops: %i)", cg.spawnPoints[i], cg.spawnPlayerCounts[i] );
					w = CG_Text_Width_Ext( buffer, 0.2f, 0, &cgs.media.limboFont2 );
					CG_CommandMap_SetHighlightText( buffer, point[0] - ( w * 0.5f ), point[1] - 8 );
				}

				e = i;
			}
		} else {
			if( draw ) {
				float size = FLAGSIZE_NORMAL;
				
#ifdef SMALL_CM_ICONS
                if( scissor ) {
					size *= (scissor->zoomFactor / BASEMAPZOOM);
				} else {
#endif
					size *= cgs.ccZoomFactor;
#ifdef SMALL_CM_ICONS
				}
#endif

				CG_DrawPic( point[0] - FLAG_LEFTFRAC*size, point[1] - FLAG_TOPFRAC*size, size, size, cgs.media.commandCentreSpawnShader[  cg.spawnTeams[i] == TEAM_AXIS ? 0 : 1 ] );
				
				if( !scissor ) {
					Com_sprintf( buffer, sizeof( buffer ), "(Troops: %i)", cg.spawnPlayerCounts[i] );
					CG_Text_Paint_Ext( point[0] + FLAGSIZE_NORMAL*0.25f, point[1], 0.2f, 0.2f, colorWhite, buffer, 0, 0, ITEM_TEXTSTYLE_SHADOWED, &cgs.media.limboFont2 );
				}
			}
		}
	}

	return e;
}

///////////////////////////////////////////////////////////////////////////////

static void CG_DrawSpectatorIcon( float x, float y, float w, float h, mapScissor_t* scissor, float icon_size )
{
    vec2_t pos;
    float yaw;
    vec4_t clr = {1.0f, 1.0f, 1.0f, 0.65f};

    // Use our own lerp'ed origin
	if( !scissor ) {
		pos[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w;
		pos[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h;
	} else {
		pos[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * scissor->zoomFactor;
		pos[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * scissor->zoomFactor;
	}
	
	yaw = (int)cg.predictedPlayerState.viewangles[YAW];

    // Set up icon coords
    vec2_t icon_pos;
    vec2_t icon_extends;
    if( scissor ) {
	    icon_pos[0] = pos[0] - scissor->tl[0] + x - (icon_size * scissor->zoomFactor / BASEMAPZOOM);
	    icon_pos[1] = pos[1] - scissor->tl[1] + y - (icon_size * scissor->zoomFactor / BASEMAPZOOM);
    } else { 
	    icon_pos[0] = x + pos[0] - icon_size;
	    icon_pos[1] = y + pos[1] - icon_size;
    }
    icon_extends[0] = 2 * icon_size;
    icon_extends[1] = 2 * icon_size;

#ifdef SMALL_CM_ICONS
    if( scissor ) {
        icon_extends[0] *= (scissor->zoomFactor / BASEMAPZOOM);
	    icon_extends[1] *= (scissor->zoomFactor / BASEMAPZOOM);
    }
#endif

    // Highlight
    trap_R_SetColor(clr);
	//CG_DrawPic(icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.ccPlayerHighlight);
    CG_DrawPic( icon_pos[0], icon_pos[1], icon_extends[0], icon_extends[1], cgs.media.cm_spec_icon);
    CG_DrawRotatedPic( icon_pos[0]-1, icon_pos[1]-1, icon_extends[0]+2, icon_extends[1]+2, cgs.media.cm_arrow_spec, (0.5 - ( yaw - 180.f ) / 360.f));	
    trap_R_SetColor(NULL);
}

///////////////////////////////////////////////////////////////////////////////

void CG_DrawMap( float x, float y, float w, float h, int mEntFilter, mapScissor_t *scissor, qboolean interactive, float alpha, qboolean borderblend ) {
	snapshot_t* snap;
	int icon_size;
	int exspawn;

	expanded = false;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	if ( scissor ) {
		float s0, s1, t0, t1;

		icon_size = AUTOMAP_PLAYER_ICON_SIZE;

		if ( scissor->br[0] >= scissor->tl[0] ) {
			float sc_x, sc_y, sc_w, sc_h;

			sc_x = x;
			sc_y = y;
			sc_w = w;
			sc_h = h;

			CG_DrawPic( sc_x, sc_y, sc_w, sc_h, cgs.media.commandCentreAutomapMaskShader );

			s0 = (scissor->tl[0])/(w*scissor->zoomFactor);
			s1 = (scissor->br[0])/(w*scissor->zoomFactor);
			t0 = (scissor->tl[1])/(h*scissor->zoomFactor);
			t1 = (scissor->br[1])/(h*scissor->zoomFactor);

			CG_AdjustFrom640( &sc_x, &sc_y, &sc_w, &sc_h );
			if( cgs.ccLayers ) {
				trap_R_DrawStretchPic( sc_x, sc_y, sc_w, sc_h, s0, t0, s1, t1, cgs.media.commandCentreAutomapShader[cgs.ccSelectedLayer] );
			} else {
				trap_R_DrawStretchPic( sc_x, sc_y, sc_w, sc_h, s0, t0, s1, t1, cgs.media.commandCentreAutomapShader[0] );
			}
			trap_R_DrawStretchPic( 0, 0, 0, 0, 0, 0, 0, 0, cgs.media.whiteShader ); // HACK : the code above seems to do weird things to 
																					// the next trap_R_DrawStretchPic issued. This works
																					// around this.
		}

		// Draw the grid
		CG_DrawGrid( x, y, w, h, scissor );
	} else {
		icon_size = COMMANDMAP_PLAYER_ICON_SIZE;

		{
			vec4_t color;
			Vector4Set( color, 1.f, 1.f, 1.f, alpha );
			trap_R_SetColor( color );
            // Jaybird - since the icons now need an alpha background, include a fake one
            CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapMask);
			if( cgs.ccLayers ) {
				CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapShaderTrans[cgs.ccSelectedLayer] );
			} else {
				CG_DrawPic(x, y, w, h, cgs.media.commandCentreMapShaderTrans[0] );
			}
			trap_R_SetColor( NULL );
		}

		// Draw the grid
		CG_DrawGrid( x, y, w, h, NULL );
	}

	if( borderblend ) {
		vec4_t clr = { 0.f, 0.f, 0.f, 0.75f };
		trap_R_SetColor( clr );
		CG_DrawPic( x, y, w, h, cgs.media.limboBlendThingy );
		trap_R_SetColor( NULL );
	}

	exspawn = CG_DrawSpawnPointInfo( (int)x, (int)y, (int)w, (int)h, qfalse, scissor, -1 );

    // Draw everything but players first
    for(mapEntList_t::const_iterator i = mapEntities.begin(); i != mapEntities.end(); i++ ) {
        mapEntityData_t* mEnt = *i;

        // There's a lot of conditions for spectators :)
        if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {

            // Interactive should draw the selected team's objectives
            if (interactive) {
                if (CG_LimboPanel_GetRealTeam() != mEnt->team) {
                    continue;
                }

            // If we are following someone, we should draw their team's objectives
            } else if (snap->ps.clientNum == cg.clientNum || mEnt->team != cgs.clientinfo[snap->ps.clientNum].team) {
                continue;
            }

        // Only draw our own team's objectives
        } else if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
			continue;
		}

		if( mEnt->type == ME_PLAYER ||
			mEnt->type == ME_PLAYER_DISGUISED ||
			mEnt->type == ME_PLAYER_REVIVE )
			continue;

        CG_DrawMapEntity( mEnt, x, y, w, h, mEntFilter, scissor, interactive, snap, icon_size );
	}

    // Spawn point info
	CG_DrawSpawnPointInfo( (int)x, (int)y, (int)w, (int)h, qtrue, scissor, exspawn );

    // Mortar impact markers
	CG_DrawMortarMarker( (int)x, (int)y, (int)w, (int)h, qtrue, scissor, exspawn );

    // Now draw players
    for(mapEntList_t::const_iterator i = mapEntities.begin(); i != mapEntities.end(); i++ ) {
        mapEntityData_t* mEnt = *i;

        if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR) {
            if (snap->ps.clientNum != cg.clientNum && mEnt->team != cgs.clientinfo[snap->ps.clientNum].team) {
                continue;
            }
        } else if( mEnt->team != CG_LimboPanel_GetRealTeam() ) {
			continue;
		}

		if( mEnt->type != ME_PLAYER &&
			mEnt->type != ME_PLAYER_DISGUISED &&
			mEnt->type != ME_PLAYER_REVIVE )
			continue;

        CG_DrawMapEntity( mEnt, x, y, w, h, mEntFilter, scissor, interactive, snap, icon_size );
	}

    // Since we're now allowing spectators to use command map, an icon for themselves might be handy
    if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR && snap->ps.clientNum == cg.clientNum) {
        CG_DrawSpectatorIcon( x, y, w, h, scissor, icon_size );
    }
}

///////////////////////////////////////////////////////////////////////////////

void CG_DrawExpandedAutoMap( void ) {
	float x, y, w, h;
	float b_x, b_y, b_w, b_h;
	float s1, t1, s2, t2;

	x = SCREEN_WIDTH + 10.f;
	y = 20.f;

	w = CC_2D_W;
	h = CC_2D_H;


	if( cgs.autoMapExpanded ) {	
		if( cg.time - cgs.autoMapExpandTime < 250.f ) {
			x -= ( ( cg.time - cgs.autoMapExpandTime ) / 250.f ) * ( w + 30.f );
		} else {
			x = SCREEN_WIDTH - w - 20.f;
		}
	} else {
		if( cg.time - cgs.autoMapExpandTime < 250.f ) {
			x = ( SCREEN_WIDTH - w - 20.f ) + ( ( cg.time - cgs.autoMapExpandTime ) / 250.f ) * ( w + 30.f );
		} else {
			return;
		}
	}

	CG_DrawMap( x, y, w, h, cgs.ccFilter, NULL, qfalse, .7f, qfalse );

	// Draw the border

	// top left
	s1 = 0;
	t1 = 0;
	s2 = 1;
	t2 = 1;
	b_x = x - 8;
	b_y = y - 8;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// top
	s2 = w / 256.f;
	b_x = x;
	b_y = y - 8;
	b_w = w;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorderShader );

	// top right
	s1 = 1;
	t1 = 0;
	s2 = 0;
	t2 = 1;
	b_x = x + w;
	b_y = y - 8;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// right
	s1 = 1;
	t1 = h / 256.f;
	s2 = 0;
	t2 = 0;
	b_x = x + w;
	b_y = y;
	b_w = 8;
	b_h = h;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorder2Shader );

	// bottom right
	s1 = 1;
	t1 = 1;
	s2 = 0;
	t2 = 0;
	b_x = x + w;
	b_y = y + h;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// bottom
	s1 = w / 256.f;
	b_x = x;
	b_y = y + h;
	b_w = w;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorderShader );

	// bottom left
	s1 = 0;
	t1 = 1;
	s2 = 1;
	t2 = 0;
	b_x = x - 8;
	b_y = y + h;
	b_w = 8;
	b_h = 8;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapCornerShader );

	// left
	s1 = 0;
	t1 = 0;
	s2 = 1;
	t2 = h / 256.f;
	b_x = x - 8;
	b_y = y;
	b_w = 8;
	b_h = h;
	CG_AdjustFrom640( &b_x, &b_y, &b_w, &b_h );
	trap_R_DrawStretchPic( b_x, b_y, b_w, b_h, s1, t1, s2, t2, cgs.media.commandCentreAutomapBorder2Shader );
}

///////////////////////////////////////////////////////////////////////////////

void CG_DrawAutoMap( void ) {
	float x, y, w, h;
	mapScissor_t mapScissor;
	vec2_t automapTransformed;

	memset( &mapScissor, 0, sizeof(mapScissor) );

	if( cgs.ccLayers ) {
		cgs.ccSelectedLayer = CG_CurLayerForZ( (int)cg.predictedPlayerEntity.lerpOrigin[2] );
	}

	x = CM_DEFAULT_X;
	y = CM_DEFAULT_Y;
	w = CM_DEFAULT_W;
	h = CM_DEFAULT_H;

    // Jaybird - Alternate Hud
	if (cg_althud.integer) {
		x = CM_ALT_X;
		y = CM_ALT_Y;
		w *= CM_ALT_SCALE;
		h *= CM_ALT_SCALE;
	}

    if( cgs.autoMapExpanded ) {	
		if( cg.time - cgs.autoMapExpandTime < 100.f ) {
			// Jaybird - Alternate Hud
			if (!cg_althud.integer)
				y -= ((cg.time - cgs.autoMapExpandTime) / 100.f) * 128.f;
			else
				y += ((cg.time - cgs.autoMapExpandTime) / 100.f) * 128.f;
			CG_DrawExpandedAutoMap();
		} else {
			//y -= 128.f;
			CG_DrawExpandedAutoMap();
			return;
		}
	} else {
		if( cg.time - cgs.autoMapExpandTime <= 150.f ) {
			//y -= 128.f;
			CG_DrawExpandedAutoMap();
			return;
		} else if( ( cg.time - cgs.autoMapExpandTime > 150.f ) && ( cg.time - cgs.autoMapExpandTime < 250.f ) ) {
			// Jaybird - Alternate Hud
			if (!cg_althud.integer)
				y = (y - 128.f) + ((cg.time - cgs.autoMapExpandTime - 150.f) / 100.f) * 128.f;
			else
				y = (y + 128.f) - ((cg.time - cgs.autoMapExpandTime - 150.f) / 100.f) * 128.f;
			CG_DrawExpandedAutoMap();
		}
	}

	mapScissor.circular = qtrue;


	mapScissor.zoomFactor = cg_mapZoom.value;

    if (cg_althud.integer)
        mapScissor.zoomFactor = max(mapScissor.zoomFactor * CM_ALT_SCALE, 1.0f);

	mapScissor.tl[0] = mapScissor.tl[1] = 0;
	mapScissor.br[0] = mapScissor.br[1] = -1;

	automapTransformed[0] = ((cg.predictedPlayerEntity.lerpOrigin[0] - cg.mapcoordsMins[0]) * cg.mapcoordsScale[0]) * w * mapScissor.zoomFactor;
	automapTransformed[1] = ((cg.predictedPlayerEntity.lerpOrigin[1] - cg.mapcoordsMins[1]) * cg.mapcoordsScale[1]) * h * mapScissor.zoomFactor;

	// update clip region (for next drawing). clip region has a size kAutomap_width x kAutomap_height
	// and it is after zooming is accounted for.
	//
	// first try to center the clip region around the player. then make sure the region
	// stays within the world map.
	mapScissor.tl[0] = automapTransformed[0] - w/2;
	if( mapScissor.tl[0] < 0 )
		mapScissor.tl[0] = 0;
	mapScissor.br[0] = mapScissor.tl[0] + w;
	if( mapScissor.br[0] > (w * mapScissor.zoomFactor) ) {
		mapScissor.br[0] = w * mapScissor.zoomFactor;
		mapScissor.tl[0] = mapScissor.br[0] - w;
	}

	mapScissor.tl[1] = automapTransformed[1] - h/2;
	if( mapScissor.tl[1] < 0 )
		mapScissor.tl[1] = 0;
	mapScissor.br[1] = mapScissor.tl[1] + h;
	if ( mapScissor.br[1] > (h * mapScissor.zoomFactor)) {
		mapScissor.br[1] = h * mapScissor.zoomFactor;
		mapScissor.tl[1] = mapScissor.br[1] - h;
	}

	CG_DrawMap( x, y, w, h, cgs.ccFilter, &mapScissor, qfalse, 1.f, qfalse );
}

///////////////////////////////////////////////////////////////////////////////

qboolean CG_CommandCentreLayersClick( void ) {
	int x, y;
	int i;

	if( !cgs.ccLayers )
		return qfalse;

	x = CC_2D_X + CC_2D_W - 32; 
	y = CC_2D_Y + CC_2D_H - 32;

	for( i = 0; i < cgs.ccLayers; i++ ) {
		if( BG_RectContainsPoint( x, y, 32, 32, cgDC.cursorx, cgDC.cursory ) ) {
			cgs.ccSelectedLayer = i;
			return qtrue;
		}
		y -= 34;
	}

	return qfalse;
}

///////////////////////////////////////////////////////////////////////////////

qboolean CG_CommandCentreSpawnPointClick( void ) {
	int i;
	vec2_t point;

	if( cgs.ccFilter & CC_FILTER_SPAWNS ) {
		return qfalse;
	}

	for ( i = 1; i < cg.spawnCount; i++ ) {
		if( (cgs.clientinfo[cg.clientNum].team != TEAM_SPECTATOR) && cg.spawnTeams[i] && cg.spawnTeams[i] != CG_LimboPanel_GetRealTeam())
			continue;

		if( cg.spawnTeams[i] & 256 )
			continue;

		if( cgs.ccLayers ) {
			if( CG_CurLayerForZ( (int)cg.spawnCoords[i][2] ) != cgs.ccSelectedLayer )
                continue;
		}

		point[0] = cg.spawnCoords[i][0];
		point[1] = cg.spawnCoords[i][1];

		if( BG_RectContainsPoint( point[0] - FLAGSIZE_NORMAL*0.5f, point[1] - FLAGSIZE_NORMAL*0.5f, FLAGSIZE_NORMAL, FLAGSIZE_NORMAL, cgDC.cursorx, cgDC.cursory ) ) {
			trap_SendConsoleCommand( va( "setspawnpt %i\n", i ) );
			cg.selectedSpawnPoint = i;
			cgs.ccRequestedObjective = -1;
			return qtrue;
		}
	}

	return qfalse;
}
