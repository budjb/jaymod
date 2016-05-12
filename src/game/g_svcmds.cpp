


// this file holds commands that can be executed by the server console, but not remote clients

#include <bgame/impl.h>
#include <omnibot/et/g_etbot_interface.h>

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and you can use '*' to match any value
so you can specify an entire class C network with "addip 192.246.40.*"

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.

TTimo NOTE: GUID functions are copied over from the model of IP banning,
used to enforce max lives independently from server reconnect and team changes (Xian)

TTimo NOTE: for persistence, bans are stored in g_banIPs cvar MAX_CVAR_VALUE_STRING
The size of the cvar string buffer is limiting the banning to around 20 masks
this could be improved by putting some g_banIPs2 g_banIps3 etc. maybe
still, you should rely on PB for banning instead

==============================================================================
*/

typedef struct ipGUID_s
{
	char		compare[33];
} ipGUID_t;

#define	MAX_IPFILTERS	1024

typedef struct ipFilterList_s {
	ipFilter_t	ipFilters[MAX_IPFILTERS];
	int			numIPFilters;
	char		cvarIPList[32];
} ipFilterList_t;

static ipFilterList_t		ipFilters;
static ipFilterList_t		ipMaxLivesFilters;

static ipGUID_t		guidMaxLivesFilters[MAX_IPFILTERS];
static int			numMaxLivesFilters = 0;

/*
=================
StringToFilter
=================
*/
qboolean StringToFilter( const char *s, ipFilter_t *f )
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			if (*s == '*') // 'match any'
			{
				// b[i] and m[i] to 0
				s++;
				if (!*s)
					break;
				s++;
				continue;
			}
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans( ipFilterList_t *ipFilterList )
{
	byte	b[4];
	byte	m[4];
	int		i,j;
	char	iplist_final[MAX_CVAR_VALUE_STRING];
	char	ip[64];

	*iplist_final = 0;
	for (i = 0 ; i < ipFilterList->numIPFilters ; i++)
	{
		if( ipFilterList->ipFilters[i].compare == 0xffffffff )
			continue;

		*(unsigned *)b = ipFilterList->ipFilters[i].compare;
		*(unsigned *)m = ipFilterList->ipFilters[i].mask;
		*ip = 0;
		for( j = 0; j < 4 ; j++ ) {
			if( m[j] != 255 )
				Q_strcat(ip, sizeof(ip), "*");
			else
				Q_strcat(ip, sizeof(ip), va("%i", b[j]));
			Q_strcat(ip, sizeof(ip), (j<3) ? "." : " ");
		}		
		if( strlen(iplist_final)+strlen(ip) < MAX_CVAR_VALUE_STRING )
		{
			Q_strcat( iplist_final, sizeof(iplist_final), ip);
		}
		else
		{
			Com_Printf( "%s overflowed at MAX_CVAR_VALUE_STRING\n", ipFilterList->cvarIPList );
			break;
		}
	}

	trap_Cvar_Set( ipFilterList->cvarIPList, iplist_final );
}

void PrintMaxLivesGUID ()
{
	int		i;

	for (i = 0 ; i < numMaxLivesFilters ; i++)
	{
		G_LogPrintf( "%i. %s\n", i, guidMaxLivesFilters[i].compare );
	}
	G_LogPrintf ( "--- End of list\n");
}

/*
=================
G_FilterPacket
=================
*/
bool G_FilterPacket( ipFilterList_t* ipFilterList, const char* from )
{
	unsigned in;
	byte m[4];

	int i = 0;
	const char* p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for( i = 0; i < ipFilterList->numIPFilters; i++ )
		if( (in & ipFilterList->ipFilters[i].mask) == ipFilterList->ipFilters[i].compare)
 			return (g_filterBan.integer != 0);

	return (g_filterBan.integer == 0);
}

bool G_FilterIPBanPacket( const char* from )
{
	return G_FilterPacket( &ipFilters, from );
}

bool G_FilterMaxLivesIPPacket( const char *from )
{
	return G_FilterPacket( &ipMaxLivesFilters, from );
}

/*
 Check to see if the user is trying to sneak back in with g_enforcemaxlives enabled
*/
bool G_FilterMaxLivesPacket( const char* from )
{
	for ( int i = 0; i < numMaxLivesFilters; i++ ) {
		if ( !Q_stricmp( guidMaxLivesFilters[i].compare, from ))
			return true;
	}

	return false;
}

/*
=================
AddIP
=================
*/
void AddIP( ipFilterList_t *ipFilterList, const char *str )
{
	int		i;

	for( i = 0; i < ipFilterList->numIPFilters; i++ ) {
		if(  ipFilterList->ipFilters[i].compare == 0xffffffff )
			break;		// free spot
	}

	if( i == ipFilterList->numIPFilters ) {
		if( ipFilterList->numIPFilters == MAX_IPFILTERS ) {
			G_Printf( "IP filter list is full\n" );
			return;
		}
		ipFilterList->numIPFilters++;
	}
	
	if( !StringToFilter( str, &ipFilterList->ipFilters[i] ) )
		ipFilterList->ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans( ipFilterList );
}

void AddIPBan( const char *str )
{
	AddIP( &ipFilters, str );
}

void AddMaxLivesBan( const char *str )
{
	AddIP( &ipMaxLivesFilters, str );
}

/*
=================
AddMaxLivesGUID
Xian - with g_enforcemaxlives enabled, this adds a client GUID to a list
that prevents them from quitting and reconnecting
=================
*/
void AddMaxLivesGUID( char *str )
{
	if( numMaxLivesFilters == MAX_IPFILTERS ) {
		G_Printf( "MaxLives GUID filter list is full\n" );
		return;
	}
	Q_strncpyz( guidMaxLivesFilters[numMaxLivesFilters].compare, str, 33 );
	numMaxLivesFilters++;
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char str[MAX_CVAR_VALUE_STRING];

	ipFilters.numIPFilters = 0;
	Q_strncpyz( ipFilters.cvarIPList, "g_banIPs", sizeof(ipFilters.cvarIPList) );

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for( t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if( !s )
			break;
		while( *s == ' ' )
			*s++ = 0;
		if( *t )
			AddIP( &ipFilters, t );
		t = s;
	}
}

/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage:  addip <ip-mask>\n" );
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	AddIP( &ipFilters, str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage:  removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	if( !StringToFilter( str, &f ) )
		return;

	for (i=0 ; i< ipFilters.numIPFilters ; i++) {
		if( ipFilters.ipFilters[i].mask == f.mask	&&
			ipFilters.ipFilters[i].compare == f.compare ) {
			ipFilters.ipFilters[i].compare = 0xffffffffu;
			G_Printf( "Removed.\n" );

			UpdateIPBans( &ipFilters );
			return;
		}
	}

	G_Printf( "Didn't find %s.\n", str );
}

/*
 Xian - Clears out the entire list maxlives enforcement banlist
*/
void ClearMaxLivesBans ()
{
	int i;
	
	for( i = 0; i < numMaxLivesFilters; i++ ) {
		guidMaxLivesFilters[i].compare[0] = '\0';
	}
	numMaxLivesFilters = 0;

	ipMaxLivesFilters.numIPFilters = 0;
	Q_strncpyz( ipMaxLivesFilters.cvarIPList, "g_maxlivesbanIPs", sizeof(ipMaxLivesFilters.cvarIPList) );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_CONCUSSIVE_TRIGGER:
			G_Printf("ET_CONCUSSIVE_TRIGGR");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
		case ET_EXPLOSIVE:
			G_Printf("ET_EXPLOSIVE        ");
			break;
		case ET_EF_SPOTLIGHT:
			G_Printf("ET_EF_SPOTLIGHT     ");
			break;
		case ET_ALARMBOX:
			G_Printf("ET_ALARMBOX          ");
			break;
		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}

		G_Printf("\n");
	}
}

// fretn, note: if a player is called '3' and there are only 2 players
// on the server (clientnum 0 and 1)
// this function will say 'client 3 is not connected'
// solution: first check for usernames, if none is found, check for slotnumbers
gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

// fretn

static qboolean G_Is_SV_Running( void ) {

	char		cvar[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "sv_running", cvar, sizeof( cvar ) );
	return (qboolean)atoi( cvar );
}

/*
==================
G_GetPlayerByNum
==================
*/
gclient_t	*G_GetPlayerByNum( int clientNum ) {
	gclient_t	*cl;

	
	// make sure server is running
	if ( !G_Is_SV_Running() ) {
		return NULL;
	}

	if ( trap_Argc() < 2 ) {
		G_Printf( "No player specified.\n" );
		return NULL;
	}

	if ( clientNum < 0 || clientNum >= level.maxclients ) {
		Com_Printf( "Bad client slot: %i\n", clientNum );
		return NULL;
	}

	cl = &level.clients[clientNum];
	if ( cl->pers.connected == CON_DISCONNECTED ) {
		G_Printf( "Client %i is not connected\n", clientNum );
		return NULL;
	}

	if (cl)
		return cl;

	
	G_Printf( "User %d is not on the server\n", clientNum );

	return NULL;
}

/*
==================
G_GetPlayerByName
==================
*/
gclient_t *G_GetPlayerByName( char *name ) {

	int			i;
	gclient_t	*cl;
	char		cleanName[64];

	// make sure server is running
	if ( !G_Is_SV_Running() ) {
		return NULL;
	}

	if ( trap_Argc() < 2 ) {
		G_Printf( "No player specified.\n" );
		return NULL;
	}

	for (i = 0; i < level.numConnectedClients; i++) {
		
		cl = &level.clients[i];
		
		if (!Q_stricmp(cl->pers.netname, name)) {
			return cl;
		}

		Q_strncpyz( cleanName, cl->pers.netname, sizeof(cleanName) );
		Q_CleanStr( cleanName );
		if ( !Q_stricmp( cleanName, name ) ) {
			return cl;
		}
	}

	G_Printf( "Player %s is not on the server\n", name );

	return NULL;
}

// -fretn

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/

void Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str, qfalse, (weapon_t)cl->sess.playerWeapon, (weapon_t)cl->sess.playerWeapon2, qtrue );
}

/*
============
Svcmd_StartMatch_f

NERVE - SMF - starts match if in tournament mode
============
*/
void Svcmd_StartMatch_f(void)
{
/*	if ( !g_noTeamSwitching.integer ) {
		trap_SendServerCommand( -1, va("print \"g_noTeamSwitching not activated.\n\""));
		return;
	}
*/

	G_refAllReady_cmd(NULL);

/*
	if ( level.numPlayingClients <= 1 ) {
		trap_SendServerCommand( -1, va("print \"Not enough playing clients to start match.\n\""));
		return;
	}

	if ( cvars::gameState.ivalue == GS_PLAYING ) {
		trap_SendServerCommand( -1, va("print \"Match is already in progress.\n\""));
		return;
	}

	if ( cvars::gameState.ivalue == GS_WARMUP ) {
		trap_SendConsoleCommand( EXEC_APPEND, va( "map_restart 0 %i\n", GS_PLAYING ) );
	}
*/
}

/*
==================
Svcmd_ResetMatch_f

OSP - multiuse now for both map restarts and total match resets
==================
*/
void Svcmd_ResetMatch_f(qboolean fDoReset, qboolean fDoRestart)
{
	int i;

	for(i=0; i<level.numConnectedClients; i++) {
		g_entities[level.sortedClients[i]].client->pers.ready = qfalse;
	}

	if(fDoReset) {
		G_resetRoundState();
		G_resetModeState();
	}

	if(fDoRestart) {
		trap_SendConsoleCommand( EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP ));
	}
}

/*
============
Svcmd_SwapTeams_f

NERVE - SMF - swaps all clients to opposite team
============
*/
void Svcmd_SwapTeams_f(void)
{
    G_resetRoundState();

    switch (cvars::gameState.ivalue) {
        case GS_INITIALIZE:
        case GS_WARMUP:
            G_swapTeams();
            return;

        default:
            break;
    }

    G_resetModeState();
    trap_Cvar_Set( "g_swapteams", "1" );
    Svcmd_ResetMatch_f( qfalse, qtrue );
}


/*
====================
Svcmd_ShuffleTeams_f

OSP - randomly places players on teams
====================
*/
void Svcmd_ShuffleTeams_f(void)
{
	G_resetRoundState();
	G_shuffleTeams();

    switch (cvars::gameState.ivalue) {
        case GS_INITIALIZE:
        case GS_WARMUP:
            return;

        default:
            break;
    }

	G_resetModeState();
	Svcmd_ResetMatch_f( qfalse, qtrue );
}

void Svcmd_Campaign_f(void) {
	char	str[MAX_TOKEN_CHARS];
	int		i;
	g_campaignInfo_t *campaign = NULL;

	// find the campaign
	trap_Argv( 1, str, sizeof( str ) );
	
	for( i = 0; i < level.campaignCount; i++ ) {
		campaign = &g_campaigns[i];

		if( !Q_stricmp( campaign->shortname, str ) ) {
			break;
		}
	}

	if( i == level.campaignCount || !(campaign->typeBits & (1 << GT_WOLF) ) ) {
		G_Printf( "Can't find campaign '%s'\n", str );
		return;
	}

	trap_Cvar_Set( "g_oldCampaign", g_currentCampaign.string );
	trap_Cvar_Set( "g_currentCampaign", campaign->shortname );
	trap_Cvar_Set( "g_currentCampaignMap", "0" );

	level.newCampaign = qtrue;

	// we got a campaign, start it
	trap_Cvar_Set( "g_gametype", va( "%i", GT_WOLF_CAMPAIGN ) );
#if 0
	if( g_developer.integer )
		trap_SendConsoleCommand( EXEC_APPEND, va( "devmap %s\n", campaign->mapnames[0] ) );
	else
#endif
		trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", campaign->mapnames[0] ) );
}

void Svcmd_ListCampaigns_f(void) {
	int i, mpCampaigns;

	mpCampaigns = 0;

	for( i = 0; i < level.campaignCount; i++ ) {
		if( g_campaigns[i].typeBits & (1 << GT_WOLF) )
			mpCampaigns++;
	}

	if( mpCampaigns ) {
		G_Printf( "%i campaigns found:\n", mpCampaigns );
	} else {
		G_Printf( "No campaigns found.\n" );
		return;
	}

	for( i = 0; i < level.campaignCount; i++ ) {
		if( g_campaigns[i].typeBits & (1 << GT_WOLF) )
			G_Printf( " %s\n", g_campaigns[i].shortname );
	}
}

// ydnar: modified from maddoc sp func
extern int FindClientByName( char *name );
void Svcmd_RevivePlayer( char *name )
{
	int			clientNum;
	gentity_t	*player;
	
	
	if( !g_cheats.integer ) 
	{
		trap_SendServerCommand( -1, va( "print \"Cheats are not enabled on this server.\n\"" ) );
		return;
	}
	
	clientNum = FindClientByName( name );
	if( clientNum < 0 )
		return;
	player = &g_entities[ clientNum ];
	
	ReviveEntity( player, player );

}


// fretn - kicking

/*
==================
Svcmd_Kick_f

Kick a user off of the server
==================
*/

// change into qfalse if you want to use the qagame banning system
// which makes it possible to unban IP addresses
#define USE_ENGINE_BANLIST qtrue

static void Svcmd_Kick_f( void ) {
	gclient_t	*cl;
	int			i;
	int			timeout = -1;
	char		sTimeout[MAX_TOKEN_CHARS];
	char		name[MAX_TOKEN_CHARS];

	// make sure server is running
	if ( !G_Is_SV_Running() ) {
		G_Printf( "Server is not running.\n" );
		return;
	}

	if ( trap_Argc() < 2 || trap_Argc() > 3 ) {
		G_Printf ("Usage: kick <player name> [timeout]\n");
		return;
	}

	if( trap_Argc() == 3 ) {
		trap_Argv( 2, sTimeout, sizeof( sTimeout ) );
		timeout = atoi( sTimeout );
	} else {
		timeout = 300;
	}

	trap_Argv(1, name, sizeof(name));
	cl = G_GetPlayerByName( name );//ClientForString( name );

	if ( !cl ) {
		if ( !Q_stricmp(name, "all") ) {
			for (i = 0, cl = level.clients; i < level.numConnectedClients; i++, cl++) {
		
				// dont kick localclients ...
				if ( cl->pers.localClient ) {
					continue;
				}

				if ( timeout != -1 ) {
					char *ip;
					char userinfo[MAX_INFO_STRING];
					
					trap_GetUserinfo( cl->ps.clientNum, userinfo, sizeof( userinfo ) );
					ip = Info_ValueForKey (userinfo, "ip");
					
					// use engine banning system, mods may choose to use their own banlist
					if (USE_ENGINE_BANLIST) { 
						
						// kick but dont ban bots, they arent that lame
						if ( (g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
							timeout = 0;
						}

						trap_DropClient(cl->ps.clientNum, "player kicked", timeout);
					} else {
						trap_DropClient(cl->ps.clientNum, "player kicked", 0);
						
						// kick but dont ban bots, they arent that lame
						if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) )
							AddIPBan( ip );
					}

				} else {
					trap_DropClient(cl->ps.clientNum, "player kicked", 0);				
				}
			}
		}
		return;
	} else {
		// dont kick localclients ...
		if ( cl->pers.localClient ) {
			G_Printf("Cannot kick host player\n");
			return;
		}

		if ( timeout != -1 ) {
			char *ip;
			char userinfo[MAX_INFO_STRING];
			
			trap_GetUserinfo( cl->ps.clientNum, userinfo, sizeof( userinfo ) );
			ip = Info_ValueForKey (userinfo, "ip");
			
			// use engine banning system, mods may choose to use their own banlist
			if (USE_ENGINE_BANLIST) { 
				
				// kick but dont ban bots, they arent that lame
				if ( (g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
					timeout = 0;
				}
				trap_DropClient(cl->ps.clientNum, "player kicked", timeout);
			} else {
				trap_DropClient(cl->ps.clientNum, "player kicked", 0);
				
				// kick but dont ban bots, they arent that lame
				if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) )
					AddIPBan( ip );
			}

		} else {
			trap_DropClient(cl->ps.clientNum, "player kicked", 0);				
		}
	}
}

/*
==================
Svcmd_KickNum_f

Kick a user off of the server
==================
*/
static void Svcmd_KickNum_f( void ) {
	gclient_t	*cl;
	int timeout = -1;
	char	*ip;
	char	userinfo[MAX_INFO_STRING];
	char	sTimeout[MAX_TOKEN_CHARS];
	char	name[MAX_TOKEN_CHARS];
	int		clientNum;

	// make sure server is running
	if ( !G_Is_SV_Running() ) {
		G_Printf( "Server is not running.\n" );
		return;
	}

	if ( trap_Argc() < 2 || trap_Argc() > 3 ) {
		G_Printf ("Usage: kick <client number> [timeout]\n");
		return;
	}

	if( trap_Argc() == 3 ) {
		trap_Argv( 2, sTimeout, sizeof( sTimeout ) );
		timeout = atoi( sTimeout );
	} else {
		timeout = 300;
	}

	trap_Argv(1, name, sizeof(name));
	clientNum = atoi(name);

	cl = G_GetPlayerByNum( clientNum );
	if ( !cl ) {
		return;
	}
	if ( cl->pers.localClient ) {
		G_Printf("Cannot kick host player\n");
		return;
	}
		
	trap_GetUserinfo( cl->ps.clientNum, userinfo, sizeof( userinfo ) );
	ip = Info_ValueForKey (userinfo, "ip");
	// use engine banning system, mods may choose to use their own banlist
	if (USE_ENGINE_BANLIST) { 

		// kick but dont ban bots, they arent that lame
		if ( (g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) ) {
			timeout = 0;
		}
		trap_DropClient(cl->ps.clientNum, "player kicked", timeout);
	} else {
		trap_DropClient(cl->ps.clientNum, "player kicked", 0);

		// kick but dont ban bots, they arent that lame
		if ( !(g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT) )
			AddIPBan( ip );
	}
}

// -fretn



char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	/*if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}
	if (Q_stricmp (cmd, "removebot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}*/
	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_INSERT, "g_banIPs\n" );
		return qtrue;
	}

	if (Q_stricmp (cmd, "listmaxlivesip") == 0) {
		PrintMaxLivesGUID();
		return qtrue;
	}

	// NERVE - SMF
	if (Q_stricmp (cmd, "start_match") == 0) {
		Svcmd_StartMatch_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "reset_match") == 0) {
		Svcmd_ResetMatch_f(qtrue, qtrue);
		return qtrue;
	}

	if (Q_stricmp (cmd, "swap_teams") == 0) {
		Svcmd_SwapTeams_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "shuffle_teams") == 0) {
		Svcmd_ShuffleTeams_f();
		return qtrue;
	}

	// -NERVE - SMF

	if (Q_stricmp (cmd, "makeReferee") == 0) {
		G_MakeReferee();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeReferee") == 0) {
		G_RemoveReferee();
		return qtrue;
	}

	/*if (Q_stricmp (cmd, "mute") == 0) {
		G_MuteClient();
		return qtrue;
	}

	if (Q_stricmp (cmd, "unmute") == 0) {
		G_UnMuteClient();
		return qtrue;
	}*/

	if (Q_stricmp (cmd, "ban") == 0) {
		G_PlayerBan();
		return qtrue;
	}

	if( Q_stricmp( cmd, "campaign" ) == 0 ) {
		Svcmd_Campaign_f();
		return qtrue;
	}

	if( Q_stricmp( cmd, "listcampaigns" ) == 0 ) {
		Svcmd_ListCampaigns_f();
		return qtrue;
	}

// START - Mad Doc - TDF
	if (Q_stricmp (cmd, "revive") == 0)
	{
		trap_Argv( 1, cmd, sizeof( cmd ) );
		Svcmd_RevivePlayer( cmd );
		return qtrue;
	}
// END - Mad Doc - TDF

	if (Q_stricmp (cmd, "bot") == 0) {
		Bot_Interface_ConsoleCommand();
		return qtrue;
	}

	// fretn - moved from engine
	if (!Q_stricmp(cmd, "kick")) {
		Svcmd_Kick_f();
		return qtrue;
	}
	
	if (!Q_stricmp(cmd, "clientkick")) {
		Svcmd_KickNum_f();
		return qtrue;
	}
	// -fretn

	if( g_dedicated.integer ) {
		if( !Q_stricmp (cmd, "say")) {
			trap_SendServerCommand( -1, va("cpm \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}

		if (!Q_stricmp(cmd, "cp")) {
			trap_SendServerCommand( -1, va("cp \"%s\n\"", Q_AddCR(ConcatArgs(1))));
			return qtrue;
		}

		if (!Q_stricmp(cmd, "bp")) {
			trap_SendServerCommand( -1, va("bp \"%s\n\"", Q_AddCR(ConcatArgs(1))));
			return qtrue;
		}

		if( !Q_stricmp( cmd, "ksmsg" )) {
			// Jaybird - Format:
			// ksmsg "player_name" "kills" "custom spree message"
			// kills > 0, killing spree
			// kills < 0, losing spree

			char player[MAX_NAME_LENGTH];
			char kills[4];
			char message[150];
			trap_Argv( 1, player, sizeof( player ));
			trap_Argv( 2, kills, sizeof( kills ));
			trap_Argv( 3, message, sizeof( message ));
			trap_SendServerCommand( -1, va( "ksmsg \"%s\" \"%s\" \"%s\"\n", player, kills, message ));
			return qtrue;
		}

		if (!Q_stricmp(cmd, "cpm")) {
			trap_SendServerCommand( -1, va("cpm \"%s\n\"", Q_AddCR(ConcatArgs(1))));
			return qtrue;
		}

		if(!Q_stricmp(cmd, "chat") ||
			!Q_stricmp(cmd, "qsay")) {

			ConsoleChat(qfalse);
			return qtrue;
		}

    	if(!Q_stricmp(cmd, "chatclient")) {
			ConsoleChat(qtrue);
			return qtrue;
		}

		if ( !Q_stricmp(cmd, "clearxp")) {
            userDB.xpResetAll();

            for (int i = 0; i < level.numConnectedClients; i++)
                g_clientObjects[ level.sortedClients[i] ].xpReset();

			return qtrue;
		}

		if( !Q_stricmp( cmd, "playsound_env" )) {
			G_PlaySoundEnv_Cmd();
			return qtrue;
		}

		if( !Q_stricmp( cmd, "playsound" )) {
			G_PlaySound_Cmd();
			return qtrue;
		}

		if( !Q_stricmp( cmd, "makeshoutcaster" ) || !Q_stricmp( cmd, "makeshoutcast" ) || !Q_stricmp( cmd, "makesc" )) {
			G_MakeShoutcaster();
			return qtrue;
		}

		if( !Q_stricmp( cmd, "removeshoutcaster" ) || !Q_stricmp( cmd, "removeshoutcast" ) || !Q_stricmp( cmd, "removesc" )) {
			G_RemoveShoutcaster();
			return qtrue;
		}

		// OSP - console also gets ref commands
		if(!level.fLocalHost && Q_stricmp(cmd, "ref") == 0) {
			// CHRUKER: b005 - G_refCommandCheck expects the next argument (warn, pause, lock etc).
			trap_Argv(1, cmd, sizeof(cmd));
			if(!G_refCommandCheck(NULL, cmd)) {
				G_refHelp_cmd(NULL);
			}
			return(qtrue);
		}

		// see if this is a admin system command
		if (cmd::process( NULL ))
			return qtrue;

		// prints to the console instead now
		return qfalse;
	}

	return qfalse;
}

qboolean ConsoleChat(qboolean chatClient) {

	int clientNum = -1;
	char line[MAX_SAY_TEXT];
	int i;
	char *s;
	int pids[MAX_CLIENTS];
	char name[MAX_NAME_LENGTH];
	int pcount = 0;

	if(chatClient) {
		if(trap_Argc() < 3) {
			G_Printf("usage: chatclient [slot#|name] message\n");
			return qfalse;
		}
		trap_Argv(1, name, sizeof(name));
		pcount = ClientNumbersFromString(name, pids);
		if(pcount > 1) {
			G_Printf("chatclient: more than one player matches."
				"nothing sent.\n");
			return qfalse;
		}
		if(pcount < 1) {
			G_Printf("chatclient: no player found\n");
			return qfalse;
		}
		clientNum = pids[0];
		s = Q_AddCR(ConcatArgs(2));
	}
	else {
		s = Q_AddCR(ConcatArgs(1));
	}

	i = 0;
	line[0] = '\0';
	while(*s) {
		if(*s == '\n' || i == (MAX_SAY_TEXT - 1)) {
			line[i] = '\0';
			CPx(clientNum, va(
				"chat \"%s\" -1 0", 
				line));
			line[0] = '\0';
			i = 0;
			s++;
			continue;
		}
		line[i] = *s;
		s++;
		i++;
	}
	if(line[0]) {
		line[i] = '\0';
		CPx(clientNum, va(
				"chat \"%s\" -1 0", 
				line));
	}
	if(chatClient) {
		G_LogPrintf(va("chat(client): %d: %s\n",
			clientNum,
			ConcatArgs(1)));
	}
	else {
		G_LogPrintf(va("chat(client): %s\n", 
			ConcatArgs(1)));
	}
	return qtrue;
}
