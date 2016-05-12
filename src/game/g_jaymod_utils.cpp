#include <bgame/impl.h>

/*
==================
ClientNumbersFromString

Sets plist to an array of integers that represent client numbers that have 
names that are a partial match for s. List is terminated by a -1.

Returns number of matching clientids.
==================
*/
int ClientNumbersFromString( char *s, int *plist) {
	gclient_t *p;
	int i, found = 0;
	char s2[MAX_STRING_CHARS];
	char n2[MAX_STRING_CHARS];
	char *m;
	qboolean is_slot = qtrue;

	*plist = -1;

	// if a number is provided, it might be a slot # 
	for( i = 0; i < int(strlen( s )); i++ ) { 
		if( s[i] < '0' || s[i] > '9' ) {
			is_slot = qfalse;
			break;
		}
	}
	if( is_slot ) {
		i = atoi( s );
		if( i >= 0 && i < level.maxclients ) {
			p = &level.clients[i];
			if( p->pers.connected == CON_CONNECTED ) {
				*plist++ = i;
				*plist = -1;
				return 1;
			}
		}
	}

	// Now look for name matches
	SanitizeString( s, s2, qtrue );

	if( strlen(s2) < 1 )
		return 0;

	for( i = 0; i < level.maxclients; i++ ) {
		p = &level.clients[i];
		if( p->pers.connected != CON_CONNECTED )
			continue;
		SanitizeString( p->pers.netname, n2, qtrue );
		m = strstr( n2, s2 );
		if( m != NULL ) {
			*plist++ = i;
			found++;
		}
	}
	*plist = -1;
	return found;
}

/*
====================
Q_SayConcatArgs
====================
*/
char *Q_SayConcatArgs(int start) {
	char *s;
	int c = 0;

	s = ConcatArgs(0);
	while(*s) {
		if(c == start) return s;
		if(*s == ' ') {
			s++;
			if(*s != ' ') {
				c++;
				continue;
			}
			while(*s && *s == ' ') s++;  
			c++;
		}
		s++;
	}
	return s;
}


/*
====================
Q_AddCR
Replaces all occurances of "\n" with '\n'
====================
*/
char *Q_AddCR(char *s)
{
	char *copy, *place, *start;

	if(!*s) return s;
	start = s;
	while(*s) {
		if(*s == '\\') {
			copy = s;
			place = s;
			s++;
			if(*s && *s == 'n') {
				*copy = '\n';
				while(*++s) {
					*++copy = *s;
				}
				*++copy = '\0';
				s = place;
				continue;
			}
		}
		s++;
	}
	return start;
}

/*
====================
A replacement for trap_Argc() that can correctly handle
   say "!cmd arg0 arg1"
as well as
   say !cmd arg0 arg1
The client uses the latter for messages typed in the console
and the former when the message is typed in the chat popup
====================
*/
int Q_SayArgc() 
{
	int c = 1;
	char *s;

	s = ConcatArgs(0);
	if(!*s) return 0;
	while(*s) {
		if(*s == ' ') {
			s++;
			if(*s != ' ') {
				c++;
				continue;
			}
			while(*s && *s == ' ') s++;  
			c++;
		}
		s++;
	}
	return c;
}

/*
====================
G_WeaponNumToStr

Returns a string for
a specific weapon.
====================
*/
char* G_WeaponNumToStr(weapon_t weapon) {
	switch (weapon) {
		case WP_AKIMBO_COLT:			return "akimbo colts";
		case WP_AKIMBO_LUGER:			return "akimbo lugers";
		case WP_AKIMBO_SILENCEDCOLT:	return "silenced akimbo colts";
		case WP_AKIMBO_SILENCEDLUGER:	return "silenced akimbo lugers";
		case WP_AMMO:					return "ammo pack";
		case WP_BINOCULARS:				return "binoculars";
		case WP_COLT:					return "colt";
		case WP_DYNAMITE:				return "dynamite";
		case WP_FLAMETHROWER: 			return "flame thrower";
		case WP_KNIFE:					return "knife";
		case WP_LANDMINE:				return "landmine";
		case WP_LANDMINE_BBETTY:		return "S-mine";
		case WP_LANDMINE_PGAS:			return "poison-gas landmine";
		case WP_LUGER:					return "luger";
		case WP_MEDIC_SYRINGE:			return "syringe";
		case WP_MEDKIT:					return "health pack";
		case WP_MP40:					return "MP40";
		case WP_PANZERFAUST: 			return "panzerfaust";
		case WP_PLIERS:					return "pliers";
		case WP_POISON_SYRINGE:			return "poison syringe";
		case WP_SATCHEL:				return "satchel";
		case WP_SATCHEL_DET:			return "satchel detonator";
		case WP_SILENCED_COLT:			return "silenced colt";
		case WP_SILENCER:				return "silenced luger";
		case WP_SMOKE_BOMB:				return "smoke bomb";
		case WP_SMOKE_MARKER:			return "airstrike marker";
		case WP_STEN:					return "sten";
		case WP_THOMPSON:				return "thompson";
        case WP_M97:                    return "M97";
        case WP_MOLOTOV:                return "molotov cocktail";

		case WP_GRENADE_LAUNCHER:
		case WP_GRENADE_PINEAPPLE:
			return "grenade";

		case WP_GPG40:
		case WP_K43:
		case WP_K43_SCOPE:
		case WP_KAR98:
			return "K43";

		case WP_CARBINE:
		case WP_GARAND:
		case WP_GARAND_SCOPE:
		case WP_M7:
			return "garand";

		case WP_MOBILE_MG42:
		case WP_MOBILE_MG42_SET:
			return "mobile MG42";


		case WP_FG42:
		case WP_FG42SCOPE:
			return "FG42";

		case WP_MORTAR:
		case WP_MORTAR_SET:
			return "mortar";

		case WP_ADRENALINE_SHARE:
		case WP_MEDIC_ADRENALINE:
			return "adrenaline syringe";

		default:
			break;
	}

	return "unknown";
}

void G_ClientSound( gentity_t *ent, const char *sound ) {
	gentity_t *hs_ent;

	if( !ent || !ent->client )
		return;

	hs_ent = G_TempEntity(ent->client->ps.origin, EV_GLOBAL_CLIENT_SOUND);
	hs_ent->s.teamNum = (ent->client - level.clients);
	hs_ent->s.eventParm = G_SoundIndex(sound);
}

void G_ShakeClient( gentity_t *ent, int shake ) {
	gentity_t *tent;

	if (!ent || !ent->client)
		return;

  	tent = G_TempEntity( vec3_origin, EV_SHAKE );
	tent->s.onFireStart = shake;
	tent->r.svFlags |= SVF_SINGLECLIENT;
	tent->r.singleClient = ent-g_entities;
}

void G_StripIPPort( string& ip )
{
    string::size_type pos = ip.find(':');
    if (pos != string::npos) {
        ip.resize(pos);
    }
}

string SanitizeString(const string& str, bool toLower)
{
    string::size_type length = str.length();
    string::size_type i = 0;
    string ret;

    while (i < length) {
		if (str[i] == 27 || str[i] == '^') {
			i++;
			if (i < length)
                i++;
			continue;
		}

		if(str[i] < 32) {
			i++;
			continue;
		}

        ret += toLower ? tolower(str[i++]) : str[i++];
	}

    return ret;
}

void G_DbLoad()
{
    /* Array conectedUsers is the only global location outside of userDB which
     * maintains a convenience list of User pointers for each connected client.
     * After loading, the array must be repopulated.
     */
    struct Record {
        bool   active;
        string guid;
    } data[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        User* const u = connectedUsers[i];
        Record& r = data[i];

        if (!u) {
            r.active = false;
            continue;
        }

        r.active = true;
        r.guid = u->guid;
    }

    // reload levels
    levelDB.load();

    // reload (merge) users
    userDB.load( true );

    // repopulate
    for (int i = 0; i < MAX_CLIENTS; i++) {
        const Record& r = data[i];
        if (!r.active) {
            connectedUsers[i] = NULL;
            continue;
        }

        string err;
        connectedUsers[i] = &userDB.fetchByKey( r.guid, err, true );
    }

    mapDB.load();
    censorDB.load();
}
