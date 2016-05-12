#ifndef GAME_CMD_UTIL_H
#define GAME_CMD_UTIL_H

///////////////////////////////////////////////////////////////////////////////

AbstractCommand* commandForName( const string& );
AbstractCommand::PostAction lockTeams( Client*, const vector<string>&, Buffer&, bool );

bool entityHasPermission ( const gentity_t*, const Privilege& priv );
int  levelForEntity      ( const gentity_t* );

bool matchClient  ( const string&, Client*&, Buffer& );
bool matchClients ( const string&, vector<Client*>&, Buffer& );

void print     ( Client*, const Buffer&, bool = false );
void printChat ( Client*, const Buffer&, bool = false );
void printCpm  ( Client*, const Buffer&, bool = false );
void printPm   ( Client*, const Buffer&, bool );

bool process( Client*, bool = false, string* = NULL );

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_CMD_UTIL_H
