// Copyright (C) 1999-2000 Id Software, Inc.
//

#include <bgame/impl.h>

// this file is only included when building a dll
// g_syscalls.asm is included instead when building a qvm

extern "C" LF_PUBLIC void
dllEntry( Engine::Ptr ptr ) {
	Engine::ptr = ptr;
}

int PASSFLOAT( float x ) {
	float	floatTemp;
	floatTemp = x;
	return *(int *)&floatTemp;
}

void	trap_Printf( const char *fmt ) {
	Engine::ptr( G_PRINT, fmt );
}

void
trap_Print( const string& text )
{
    string line;
    const string::size_type max = text.length();
    for (string::size_type i = 0; i < max; i++) {
        const char c = text[i];
        line += c;
        if (c == '\n') {
            Engine::ptr( G_PRINT, line.c_str() );
            line.resize( 0 );
        }
    }

    if (line.length())
        Engine::ptr( G_PRINT, line.c_str() );
}

void	trap_Error( const char *fmt ) {
	Engine::ptr( G_ERROR, fmt );
}

int		trap_Milliseconds( void ) {
	return Engine::ptr( G_MILLISECONDS ); 
}
int		trap_Argc( void ) {
	return Engine::ptr( G_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	Engine::ptr( G_ARGV, n, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return Engine::ptr( G_FS_FOPEN_FILE, qpath, f, mode );
}

void	trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	Engine::ptr( G_FS_READ, buffer, len, f );
}

int		trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return Engine::ptr( G_FS_WRITE, buffer, len, f );
}

int		trap_FS_Rename( const char *from, const char *to ) {
	return Engine::ptr( G_FS_RENAME, from, to );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	Engine::ptr( G_FS_FCLOSE_FILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return Engine::ptr( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

void	trap_SendConsoleCommand( int exec_when, const char *text ) {
	Engine::ptr( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags ) {
	Engine::ptr( G_CVAR_REGISTER, cvar, var_name, value, flags );
}

void	trap_Cvar_Update( vmCvar_t *cvar ) {
	Engine::ptr( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	Engine::ptr( G_CVAR_SET, var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return Engine::ptr( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Engine::ptr( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	Engine::ptr( G_CVAR_LATCHEDVARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) {
	Engine::ptr( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

void trap_DropClient( int clientNum, const char *reason, int length ) {
	Engine::ptr( G_DROP_CLIENT, clientNum, reason, length );
}

void trap_SendServerCommand( int clientNum, const char *text ) {
	// rain - #433 - commands over 1022 chars will crash the
	// client engine upon receipt, so ignore them
	if( strlen( text ) > 1022 ) {
		G_LogPrintf( "%s: trap_SendServerCommand( %d ... ) length exceeds 1022.\n", GAMEVERSION, clientNum );
		G_LogPrintf( "%s: text [%.950s]... truncated\n", GAMEVERSION, text );
		return;
	}

	// Make sure no extended ASCII gets sent over the wire
	for (int i = 0, end = strlen(text); i < end; i++) {
		char c = text[i];
		if (c < 0) {
			G_LogPrintf("%s: trap_SendServerCommand(%d ...) dropped due to extended ASCII.\n", GAMEVERSION, clientNum);
			return;
		}
	}

	Engine::ptr( G_SEND_SERVER_COMMAND, clientNum, text );
}

void
trap_SendServerCommand( const string& cmd, const text::Buffer& buf, int slot )
{
    if (!buf.length)
        return;

    list<string> commands;
    buf.toCommands( cmd, commands );

    const list<string>::const_iterator end = commands.end();
    for ( list<string>::const_iterator it = commands.begin(); it != end; it++ )
	    Engine::ptr( G_SEND_SERVER_COMMAND, slot, it->c_str() );
}

void trap_SetConfigstring( int num, const char *string ) {
    if (cvars::g_test.ivalue & G_TEST_LOG_SETCS)
        G_LogPrintf( "SETCS[%3d]: %d bytes\n", num, string ? strlen( string ) : 0);

	Engine::ptr( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	Engine::ptr( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	Engine::ptr( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
    if (cvars::g_test.ivalue & G_TEST_LOG_SETUI)
        G_LogPrintf( "SETUI[%2d]: %d bytes\n", num, buffer ? strlen( buffer ) : 0);

	Engine::ptr( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	Engine::ptr( G_GET_SERVERINFO, buffer, bufferSize );
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	// rain - if the model is NULL but we have mins/maxs
	// make a last-ditch effort to fake the bmodel
	if ( ( !name || !*name ) && (!VectorCompare( ent->r.mins, vec3_origin ) || !VectorCompare( ent->r.maxs, vec3_origin )) ) {
		ent->s.eFlags |= EF_FAKEBMODEL;
	}
	// bani - we also fake it if explicitly requested
	if ( ent->s.eFlags & EF_FAKEBMODEL ) {
		VectorCopy( ent->r.mins, ent->s.angles );
		VectorCopy( ent->r.maxs, ent->s.angles2 );
	} else {
		Engine::ptr( G_SET_BRUSH_MODEL, ent, name );
	}
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	Engine::ptr( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	Engine::ptr( G_TRACE, results, start, mins, maxs, end, -2, contentmask );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	Engine::ptr( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceCapsuleNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	Engine::ptr( G_TRACECAPSULE, results, start, mins, maxs, end, -2, contentmask );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return Engine::ptr( G_POINT_CONTENTS, point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return (qboolean)Engine::ptr( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return (qboolean)Engine::ptr( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	Engine::ptr( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return (qboolean)Engine::ptr( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntity( gentity_t *ent ) {
	Engine::ptr( G_LINKENTITY, ent );
}

void trap_UnlinkEntity( gentity_t *ent ) {
	Engine::ptr( G_UNLINKENTITY, ent );
}


int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return Engine::ptr( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return (qboolean)Engine::ptr( G_ENTITY_CONTACT, mins, maxs, ent );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return (qboolean)Engine::ptr( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

int trap_BotAllocateClient( int clientNum ) {
	return Engine::ptr( G_BOT_ALLOCATE_CLIENT, clientNum );
}

void trap_BotFreeClient( int clientNum ) {
	Engine::ptr( G_BOT_FREE_CLIENT, clientNum );
}

int trap_GetSoundLength(sfxHandle_t sfxHandle) {
	return Engine::ptr( G_GET_SOUND_LENGTH, sfxHandle );
}

sfxHandle_t	trap_RegisterSound( const char *sample, qboolean compressed ) {
	return Engine::ptr( G_REGISTERSOUND, sample, compressed );
}

#ifdef DEBUG
//#define FAKELAG
#ifdef FAKELAG
#define	MAX_USERCMD_BACKUP	256
#define	MAX_USERCMD_MASK	(MAX_USERCMD_BACKUP - 1)

static usercmd_t cmds[MAX_CLIENTS][MAX_USERCMD_BACKUP];
static int cmdNumber[MAX_CLIENTS];
#endif // FAKELAG
#endif // DEBUG

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	Engine::ptr( G_GET_USERCMD, clientNum, cmd );

#ifdef FAKELAG
	{
		char s[MAX_STRING_CHARS];
		int fakeLag;

		trap_Cvar_VariableStringBuffer( "g_fakelag", s, sizeof(s) );
		fakeLag = atoi(s);
		if( fakeLag < 0 )
			fakeLag = 0;

		if( fakeLag ) {
			int i;
			int realcmdtime, thiscmdtime;

			// store our newest usercmd
			cmdNumber[clientNum]++;
			memcpy( &cmds[clientNum][cmdNumber[clientNum] & MAX_USERCMD_MASK], cmd, sizeof(usercmd_t) );

			// find a usercmd that is fakeLag msec behind
			i = cmdNumber[clientNum] & MAX_USERCMD_MASK;
			realcmdtime = cmds[clientNum][i].serverTime;
			i--;
			do {
				thiscmdtime = cmds[clientNum][i & MAX_USERCMD_MASK].serverTime;

				if( realcmdtime - thiscmdtime > fakeLag ) {
					// found the right one
                    cmd = &cmds[clientNum][i & MAX_USERCMD_MASK];
					return;
				}

				i--;
			} while ( (i & MAX_USERCMD_MASK) != (cmdNumber[clientNum] & MAX_USERCMD_MASK) );

			// didn't find a proper one, just use the oldest one we have
			cmd = &cmds[clientNum][(cmdNumber[clientNum] - 1) & MAX_USERCMD_MASK];
			return;
		}
	}
#endif // FAKELAG
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return (qboolean)Engine::ptr( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return Engine::ptr( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete(int id) {
	Engine::ptr( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime ) {
	return Engine::ptr( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	Engine::ptr( G_SNAPVECTOR, v );
	return;
}

qboolean trap_GetTag( int clientNum, int tagFileNumber, char *tagName, orientation_t *orient ) {
	return (qboolean)Engine::ptr( G_GETTAG, clientNum, tagFileNumber, tagName, orient );
}

qboolean trap_LoadTag( const char* filename ) {
	return (qboolean)Engine::ptr( G_REGISTERTAG, filename );
}

// BotLib traps start here
int trap_BotLibSetup( void ) {
	return Engine::ptr( BOTLIB_SETUP );
}

int trap_BotLibShutdown( void ) {
	return Engine::ptr( BOTLIB_SHUTDOWN );
}

int trap_BotLibVarSet(char *var_name, char *value) {
	return Engine::ptr( BOTLIB_LIBVAR_SET, var_name, value );
}

int trap_BotLibVarGet(char *var_name, char *value, int size) {
	return Engine::ptr( BOTLIB_LIBVAR_GET, var_name, value, size );
}

int trap_BotLibDefine(char *string) {
	return Engine::ptr( BOTLIB_PC_ADD_GLOBAL_DEFINE, string );
}

int trap_PC_AddGlobalDefine( char *define ) {
	return Engine::ptr( BOTLIB_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return Engine::ptr( BOTLIB_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return Engine::ptr( BOTLIB_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return Engine::ptr( BOTLIB_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return Engine::ptr( BOTLIB_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

int trap_PC_UnReadToken( int handle ) {
	return Engine::ptr( BOTLIB_PC_UNREAD_TOKEN, handle );
}

int trap_BotLibStartFrame(float time) {
	return Engine::ptr( BOTLIB_START_FRAME, PASSFLOAT( time ) );
}

int trap_BotLibLoadMap(const char *mapname) {
	return Engine::ptr( BOTLIB_LOAD_MAP, mapname );
}

int trap_BotLibUpdateEntity(int ent, void /* struct bot_updateentity_s */ *bue) {
	return Engine::ptr( BOTLIB_UPDATENTITY, ent, bue );
}

int trap_BotLibTest(int parm0, char *parm1, vec3_t parm2, vec3_t parm3) {
	return Engine::ptr( BOTLIB_TEST, parm0, parm1, parm2, parm3 );
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return Engine::ptr( BOTLIB_GET_SNAPSHOT_ENTITY, clientNum, sequence );
}

int trap_BotGetServerCommand(int clientNum, char *message, int size) {
	return Engine::ptr( BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size );
}

void trap_BotUserCommand(int clientNum, usercmd_t *ucmd) {
	Engine::ptr( BOTLIB_USER_COMMAND, clientNum, ucmd );
}

void trap_AAS_EntityInfo(int entnum, void /* struct aas_entityinfo_s */ *info) {
	Engine::ptr( BOTLIB_AAS_ENTITY_INFO, entnum, info );
}

int trap_AAS_Initialized(void) {
	return Engine::ptr( BOTLIB_AAS_INITIALIZED );
}

void trap_AAS_PresenceTypeBoundingBox(int presencetype, vec3_t mins, vec3_t maxs) {
	Engine::ptr( BOTLIB_AAS_PRESENCE_TYPE_BOUNDING_BOX, presencetype, mins, maxs );
}

float trap_AAS_Time(void) {
	int temp;
	temp = Engine::ptr( BOTLIB_AAS_TIME );
	return (*(float*)&temp);
}

// Ridah, multiple AAS files
void trap_AAS_SetCurrentWorld(int index) {
	// Gordon: stubbed out: we only use one aas
//	Engine::ptr( BOTLIB_AAS_SETCURRENTWORLD, index );
}
// done.

int trap_AAS_PointAreaNum(vec3_t point) {
	return Engine::ptr( BOTLIB_AAS_POINT_AREA_NUM, point );
}

int trap_AAS_TraceAreas(vec3_t start, vec3_t end, int *areas, vec3_t *points, int maxareas) {
	return Engine::ptr( BOTLIB_AAS_TRACE_AREAS, start, end, areas, points, maxareas );
}

int trap_AAS_BBoxAreas(vec3_t absmins, vec3_t absmaxs, int *areas, int maxareas) {
	return Engine::ptr( BOTLIB_AAS_BBOX_AREAS, absmins, absmaxs, areas, maxareas);
}

void trap_AAS_AreaCenter(int areanum, vec3_t center) {
	Engine::ptr( BOTLIB_AAS_AREA_CENTER, areanum, center);
}

qboolean trap_AAS_AreaWaypoint(int areanum, vec3_t center) {
	return (qboolean)Engine::ptr( BOTLIB_AAS_AREA_WAYPOINT, areanum, center);
}

int trap_AAS_PointContents(vec3_t point) {
	return Engine::ptr( BOTLIB_AAS_POINT_CONTENTS, point );
}

int trap_AAS_NextBSPEntity(int ent) {
	return Engine::ptr( BOTLIB_AAS_NEXT_BSP_ENTITY, ent );
}

int trap_AAS_ValueForBSPEpairKey(int ent, char *key, char *value, int size) {
	return Engine::ptr( BOTLIB_AAS_VALUE_FOR_BSP_EPAIR_KEY, ent, key, value, size );
}

int trap_AAS_VectorForBSPEpairKey(int ent, char *key, vec3_t v) {
	return Engine::ptr( BOTLIB_AAS_VECTOR_FOR_BSP_EPAIR_KEY, ent, key, v );
}

int trap_AAS_FloatForBSPEpairKey(int ent, char *key, float *value) {
	return Engine::ptr( BOTLIB_AAS_FLOAT_FOR_BSP_EPAIR_KEY, ent, key, value );
}

int trap_AAS_IntForBSPEpairKey(int ent, char *key, int *value) {
	return Engine::ptr( BOTLIB_AAS_INT_FOR_BSP_EPAIR_KEY, ent, key, value );
}

int trap_AAS_AreaReachability(int areanum) {
	return Engine::ptr( BOTLIB_AAS_AREA_REACHABILITY, areanum );
}

int trap_AAS_AreaLadder(int areanum) {
	return Engine::ptr( BOTLIB_AAS_AREA_LADDER, areanum );
}

int trap_AAS_AreaTravelTimeToGoalArea(int areanum, vec3_t origin, int goalareanum, int travelflags) {
	return Engine::ptr( BOTLIB_AAS_AREA_TRAVEL_TIME_TO_GOAL_AREA, areanum, origin, goalareanum, travelflags );
}

int trap_AAS_Swimming(vec3_t origin) {
	return Engine::ptr( BOTLIB_AAS_SWIMMING, origin );
}

int trap_AAS_PredictClientMovement(void /* struct aas_clientmove_s */ *move, int entnum, vec3_t origin, int presencetype, int onground, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize) {
	return Engine::ptr( BOTLIB_AAS_PREDICT_CLIENT_MOVEMENT, move, entnum, origin, presencetype, onground, velocity, cmdmove, cmdframes, maxframes, PASSFLOAT(frametime), stopevent, stopareanum, visualize );
}

void trap_EA_Say(int client, char *str) {
	Engine::ptr( BOTLIB_EA_SAY, client, str );
}

void trap_EA_SayTeam(int client, char *str) {
	Engine::ptr( BOTLIB_EA_SAY_TEAM, client, str );
}

void trap_EA_UseItem(int client, char *it) {
	Engine::ptr( BOTLIB_EA_USE_ITEM, client, it );
}

void trap_EA_DropItem(int client, char *it) {
	Engine::ptr( BOTLIB_EA_DROP_ITEM, client, it );
}

void trap_EA_UseInv(int client, char *inv) {
	Engine::ptr( BOTLIB_EA_USE_INV, client, inv );
}

void trap_EA_DropInv(int client, char *inv) {
	Engine::ptr( BOTLIB_EA_DROP_INV, client, inv );
}

void trap_EA_Gesture(int client) {
	Engine::ptr( BOTLIB_EA_GESTURE, client );
}

void trap_EA_Command(int client, char *command) {
	Engine::ptr( BOTLIB_EA_COMMAND, client, command );
}

void trap_EA_SelectWeapon(int client, int weapon) {
	Engine::ptr( BOTLIB_EA_SELECT_WEAPON, client, weapon );
}

void trap_EA_Talk(int client) {
	Engine::ptr( BOTLIB_EA_TALK, client );
}

void trap_EA_Attack(int client) {
	Engine::ptr( BOTLIB_EA_ATTACK, client );
}

void trap_EA_Reload(int client) {
	Engine::ptr( BOTLIB_EA_RELOAD, client );
}

void trap_EA_Activate(int client) {
	Engine::ptr( BOTLIB_EA_USE, client );
}

void trap_EA_Respawn(int client) {
	Engine::ptr( BOTLIB_EA_RESPAWN, client );
}

void trap_EA_Jump(int client) {
	Engine::ptr( BOTLIB_EA_JUMP, client );
}

void trap_EA_DelayedJump(int client) {
	Engine::ptr( BOTLIB_EA_DELAYED_JUMP, client );
}

void trap_EA_Crouch(int client) {
	Engine::ptr( BOTLIB_EA_CROUCH, client );
}

void trap_EA_Walk(int client) {
	Engine::ptr( BOTLIB_EA_WALK, client );
}

void trap_EA_MoveUp(int client) {
	Engine::ptr( BOTLIB_EA_MOVE_UP, client );
}

void trap_EA_MoveDown(int client) {
	Engine::ptr( BOTLIB_EA_MOVE_DOWN, client );
}

void trap_EA_MoveForward(int client) {
	Engine::ptr( BOTLIB_EA_MOVE_FORWARD, client );
}

void trap_EA_MoveBack(int client) {
	Engine::ptr( BOTLIB_EA_MOVE_BACK, client );
}

void trap_EA_MoveLeft(int client) {
	Engine::ptr( BOTLIB_EA_MOVE_LEFT, client );
}

void trap_EA_MoveRight(int client) {
	Engine::ptr( BOTLIB_EA_MOVE_RIGHT, client );
}

void trap_EA_Move(int client, vec3_t dir, float speed) {
	Engine::ptr( BOTLIB_EA_MOVE, client, dir, PASSFLOAT(speed) );
}

void trap_EA_View(int client, vec3_t viewangles) {
	Engine::ptr( BOTLIB_EA_VIEW, client, viewangles );
}

void trap_EA_EndRegular(int client, float thinktime) {
	Engine::ptr( BOTLIB_EA_END_REGULAR, client, PASSFLOAT(thinktime) );
}

void trap_EA_GetInput(int client, float thinktime, void /* struct bot_input_s */ *input) {
	Engine::ptr( BOTLIB_EA_GET_INPUT, client, PASSFLOAT(thinktime), input );
}

void trap_EA_ResetInput(int client, void *init) {
	Engine::ptr( BOTLIB_EA_RESET_INPUT, client, init );
}

void trap_EA_Prone(int client) {
	Engine::ptr( BOTLIB_EA_PRONE, client );
}

int trap_BotLoadCharacter(char *charfile, int skill) {
	return Engine::ptr( BOTLIB_AI_LOAD_CHARACTER, charfile, skill);
}

void trap_BotFreeCharacter(int character) {
	Engine::ptr( BOTLIB_AI_FREE_CHARACTER, character );
}

float trap_Characteristic_Float(int character, int index) {
	int temp;
	temp = Engine::ptr( BOTLIB_AI_CHARACTERISTIC_FLOAT, character, index );
	return (*(float*)&temp);
}

float trap_Characteristic_BFloat(int character, int index, float min, float max) {
	int temp;
	temp = Engine::ptr( BOTLIB_AI_CHARACTERISTIC_BFLOAT, character, index, PASSFLOAT(min), PASSFLOAT(max) );
	return (*(float*)&temp);
}

int trap_Characteristic_Integer(int character, int index) {
	return Engine::ptr( BOTLIB_AI_CHARACTERISTIC_INTEGER, character, index );
}

int trap_Characteristic_BInteger(int character, int index, int min, int max) {
	return Engine::ptr( BOTLIB_AI_CHARACTERISTIC_BINTEGER, character, index, min, max );
}

void trap_Characteristic_String(int character, int index, char *buf, int size) {
	Engine::ptr( BOTLIB_AI_CHARACTERISTIC_STRING, character, index, buf, size );
}

int trap_BotAllocChatState(void) {
	return Engine::ptr( BOTLIB_AI_ALLOC_CHAT_STATE );
}

void trap_BotFreeChatState(int handle) {
	Engine::ptr( BOTLIB_AI_FREE_CHAT_STATE, handle );
}

void trap_BotQueueConsoleMessage(int chatstate, int type, char *message) {
	Engine::ptr( BOTLIB_AI_QUEUE_CONSOLE_MESSAGE, chatstate, type, message );
}

void trap_BotRemoveConsoleMessage(int chatstate, int handle) {
	Engine::ptr( BOTLIB_AI_REMOVE_CONSOLE_MESSAGE, chatstate, handle );
}

int trap_BotNextConsoleMessage(int chatstate, void /* struct bot_consolemessage_s */ *cm) {
	return Engine::ptr( BOTLIB_AI_NEXT_CONSOLE_MESSAGE, chatstate, cm );
}

int trap_BotNumConsoleMessages(int chatstate) {
	return Engine::ptr( BOTLIB_AI_NUM_CONSOLE_MESSAGE, chatstate );
}

void trap_BotInitialChat(int chatstate, char *type, int mcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	Engine::ptr( BOTLIB_AI_INITIAL_CHAT, chatstate, type, mcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int	trap_BotNumInitialChats(int chatstate, char *type) {
	return Engine::ptr( BOTLIB_AI_NUM_INITIAL_CHATS, chatstate, type );
}

int trap_BotReplyChat(int chatstate, char *message, int mcontext, int vcontext, char *var0, char *var1, char *var2, char *var3, char *var4, char *var5, char *var6, char *var7 ) {
	return Engine::ptr( BOTLIB_AI_REPLY_CHAT, chatstate, message, mcontext, vcontext, var0, var1, var2, var3, var4, var5, var6, var7 );
}

int trap_BotChatLength(int chatstate) {
	return Engine::ptr( BOTLIB_AI_CHAT_LENGTH, chatstate );
}

void trap_BotEnterChat(int chatstate, int client, int sendto) {
	// RF, disabled
	return;
	Engine::ptr( BOTLIB_AI_ENTER_CHAT, chatstate, client, sendto );
}

void trap_BotGetChatMessage(int chatstate, char *buf, int size) {
	Engine::ptr( BOTLIB_AI_GET_CHAT_MESSAGE, chatstate, buf, size);
}

int trap_StringContains(char *str1, char *str2, int casesensitive) {
	return Engine::ptr( BOTLIB_AI_STRING_CONTAINS, str1, str2, casesensitive );
}

int trap_BotFindMatch(char *str, void /* struct bot_match_s */ *match, unsigned long int context) {
	return Engine::ptr( BOTLIB_AI_FIND_MATCH, str, match, context );
}

void trap_BotMatchVariable(void /* struct bot_match_s */ *match, int variable, char *buf, int size) {
	Engine::ptr( BOTLIB_AI_MATCH_VARIABLE, match, variable, buf, size );
}

void trap_UnifyWhiteSpaces(char *string) {
	Engine::ptr( BOTLIB_AI_UNIFY_WHITE_SPACES, string );
}

void trap_BotReplaceSynonyms(char *string, unsigned long int context) {
	Engine::ptr( BOTLIB_AI_REPLACE_SYNONYMS, string, context );
}

int trap_BotLoadChatFile(int chatstate, char *chatfile, char *chatname) {
	return Engine::ptr( BOTLIB_AI_LOAD_CHAT_FILE, chatstate, chatfile, chatname );
}

void trap_BotSetChatGender(int chatstate, int gender) {
	Engine::ptr( BOTLIB_AI_SET_CHAT_GENDER, chatstate, gender );
}

void trap_BotSetChatName(int chatstate, char *name) {
	Engine::ptr( BOTLIB_AI_SET_CHAT_NAME, chatstate, name );
}

void trap_BotResetGoalState(int goalstate) {
	Engine::ptr( BOTLIB_AI_RESET_GOAL_STATE, goalstate );
}

void trap_BotResetAvoidGoals(int goalstate) {
	Engine::ptr( BOTLIB_AI_RESET_AVOID_GOALS, goalstate );
}

void trap_BotRemoveFromAvoidGoals(int goalstate, int number) {
	Engine::ptr( BOTLIB_AI_REMOVE_FROM_AVOID_GOALS, goalstate, number);
}

void trap_BotPushGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	Engine::ptr( BOTLIB_AI_PUSH_GOAL, goalstate, goal );
}

void trap_BotPopGoal(int goalstate) {
	Engine::ptr( BOTLIB_AI_POP_GOAL, goalstate );
}

void trap_BotEmptyGoalStack(int goalstate) {
	Engine::ptr( BOTLIB_AI_EMPTY_GOAL_STACK, goalstate );
}

void trap_BotDumpAvoidGoals(int goalstate) {
	Engine::ptr( BOTLIB_AI_DUMP_AVOID_GOALS, goalstate );
}

void trap_BotDumpGoalStack(int goalstate) {
	Engine::ptr( BOTLIB_AI_DUMP_GOAL_STACK, goalstate );
}

void trap_BotGoalName(int number, char *name, int size) {
	Engine::ptr( BOTLIB_AI_GOAL_NAME, number, name, size );
}

int trap_BotGetTopGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_GET_TOP_GOAL, goalstate, goal );
}

int trap_BotGetSecondGoal(int goalstate, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_GET_SECOND_GOAL, goalstate, goal );
}

int trap_BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags) {
	return Engine::ptr( BOTLIB_AI_CHOOSE_LTG_ITEM, goalstate, origin, inventory, travelflags );
}

int trap_BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags, void /* struct bot_goal_s */ *ltg, float maxtime) {
	return Engine::ptr( BOTLIB_AI_CHOOSE_NBG_ITEM, goalstate, origin, inventory, travelflags, ltg, PASSFLOAT(maxtime) );
}

int trap_BotTouchingGoal(vec3_t origin, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_TOUCHING_GOAL, origin, goal );
}

int trap_BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_ITEM_GOAL_IN_VIS_BUT_NOT_VISIBLE, viewer, eye, viewangles, goal );
}

int trap_BotGetLevelItemGoal(int index, char *classname, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_GET_LEVEL_ITEM_GOAL, index, classname, goal );
}

int trap_BotGetNextCampSpotGoal(int num, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_GET_NEXT_CAMP_SPOT_GOAL, num, goal );
}

int trap_BotGetMapLocationGoal(char *name, void /* struct bot_goal_s */ *goal) {
	return Engine::ptr( BOTLIB_AI_GET_MAP_LOCATION_GOAL, name, goal );
}

float trap_BotAvoidGoalTime(int goalstate, int number) {
	int temp;
	temp = Engine::ptr( BOTLIB_AI_AVOID_GOAL_TIME, goalstate, number );
	return (*(float*)&temp);
}

void trap_BotInitLevelItems(void) {
	Engine::ptr( BOTLIB_AI_INIT_LEVEL_ITEMS );
}

void trap_BotUpdateEntityItems(void) {
	Engine::ptr( BOTLIB_AI_UPDATE_ENTITY_ITEMS );
}

int trap_BotLoadItemWeights(int goalstate, char *filename) {
	return Engine::ptr( BOTLIB_AI_LOAD_ITEM_WEIGHTS, goalstate, filename );
}

void trap_BotFreeItemWeights(int goalstate) {
	Engine::ptr( BOTLIB_AI_FREE_ITEM_WEIGHTS, goalstate );
}

void trap_BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child) {
	Engine::ptr( BOTLIB_AI_INTERBREED_GOAL_FUZZY_LOGIC, parent1, parent2, child );
}

void trap_BotSaveGoalFuzzyLogic(int goalstate, char *filename) {
	Engine::ptr( BOTLIB_AI_SAVE_GOAL_FUZZY_LOGIC, goalstate, filename );
}

void trap_BotMutateGoalFuzzyLogic(int goalstate, float range) {
	Engine::ptr( BOTLIB_AI_MUTATE_GOAL_FUZZY_LOGIC, goalstate, range );
}

int trap_BotAllocGoalState(int state) {
	return Engine::ptr( BOTLIB_AI_ALLOC_GOAL_STATE, state );
}

void trap_BotFreeGoalState(int handle) {
	Engine::ptr( BOTLIB_AI_FREE_GOAL_STATE, handle );
}

void trap_BotResetMoveState(int movestate) {
	Engine::ptr( BOTLIB_AI_RESET_MOVE_STATE, movestate );
}

void trap_BotMoveToGoal(void /* struct bot_moveresult_s */ *result, int movestate, void /* struct bot_goal_s */ *goal, int travelflags) {
	Engine::ptr( BOTLIB_AI_MOVE_TO_GOAL, result, movestate, goal, travelflags );
}

int trap_BotMoveInDirection(int movestate, vec3_t dir, float speed, int type) {
	return Engine::ptr( BOTLIB_AI_MOVE_IN_DIRECTION, movestate, dir, PASSFLOAT(speed), type );
}

void trap_BotResetAvoidReach(int movestate) {
	Engine::ptr( BOTLIB_AI_RESET_AVOID_REACH, movestate );
}

void trap_BotResetLastAvoidReach(int movestate) {
	Engine::ptr( BOTLIB_AI_RESET_LAST_AVOID_REACH,movestate  );
}

int trap_BotReachabilityArea(vec3_t origin, int testground) {
	return Engine::ptr( BOTLIB_AI_REACHABILITY_AREA, origin, testground );
}

int trap_BotMovementViewTarget(int movestate, void /* struct bot_goal_s */ *goal, int travelflags, float lookahead, vec3_t target) {
	return Engine::ptr( BOTLIB_AI_MOVEMENT_VIEW_TARGET, movestate, goal, travelflags, PASSFLOAT(lookahead), target );
}

int trap_BotPredictVisiblePosition(vec3_t origin, int areanum, void /* struct bot_goal_s */ *goal, int travelflags, vec3_t target) {
	return Engine::ptr( BOTLIB_AI_PREDICT_VISIBLE_POSITION, origin, areanum, goal, travelflags, target );
}

int trap_BotAllocMoveState(void) {
	return Engine::ptr( BOTLIB_AI_ALLOC_MOVE_STATE );
}

void trap_BotFreeMoveState(int handle) {
	Engine::ptr( BOTLIB_AI_FREE_MOVE_STATE, handle );
}

void trap_BotInitMoveState(int handle, void /* struct bot_initmove_s */ *initmove) {
	Engine::ptr( BOTLIB_AI_INIT_MOVE_STATE, handle, initmove );
}

// Ridah
void trap_BotInitAvoidReach(int handle) {
	Engine::ptr( BOTLIB_AI_INIT_AVOID_REACH, handle );
}
// Done.

int trap_BotChooseBestFightWeapon(int weaponstate, int *inventory) {
	return Engine::ptr( BOTLIB_AI_CHOOSE_BEST_FIGHT_WEAPON, weaponstate, inventory );
}

void trap_BotGetWeaponInfo(int weaponstate, int weapon, void /* struct weaponinfo_s */ *weaponinfo) {
	Engine::ptr( BOTLIB_AI_GET_WEAPON_INFO, weaponstate, weapon, weaponinfo );
}

int trap_BotLoadWeaponWeights(int weaponstate, char *filename) {
	return Engine::ptr( BOTLIB_AI_LOAD_WEAPON_WEIGHTS, weaponstate, filename );
}

int trap_BotAllocWeaponState(void) {
	return Engine::ptr( BOTLIB_AI_ALLOC_WEAPON_STATE );
}

void trap_BotFreeWeaponState(int weaponstate) {
	Engine::ptr( BOTLIB_AI_FREE_WEAPON_STATE, weaponstate );
}

void trap_BotResetWeaponState(int weaponstate) {
	Engine::ptr( BOTLIB_AI_RESET_WEAPON_STATE, weaponstate );
}

int trap_GeneticParentsAndChildSelection(int numranks, float *ranks, int *parent1, int *parent2, int *child) {
	return Engine::ptr( BOTLIB_AI_GENETIC_PARENTS_AND_CHILD_SELECTION, numranks, ranks, parent1, parent2, child );
}

void trap_PbStat ( int clientNum , char *category , char *values ) {
	Engine::ptr( PB_STAT_REPORT , clientNum , category , values ) ;
}
