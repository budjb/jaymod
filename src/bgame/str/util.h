#ifndef BGAME_STR_UTIL_H
#define BGAME_STR_UTIL_H

///////////////////////////////////////////////////////////////////////////////

string  toHexString ( uint32 );
string& toHexString ( uint32, string& );

string  toString ( const vec3_t& );
string& toString ( const vec3_t&, string& );

string  toString ( const void* );
string& toString ( const void*, string& );

string  toString ( entityType_t );
string& toString ( entityType_t, string& );

string  toString ( gamestate_t );
string& toString ( gamestate_t, string& );

string  toString ( gametype_t );
string& toString ( gametype_t, string& );

string  toString( team_t );
string& toString( team_t, string& );

string  toString ( trType_t );
string& toString ( trType_t, string& );

string  toString ( weapon_t );
string& toString ( weapon_t, string& );

int     toSeconds( const string&, bool = false );
string  toStringSecondsRemaining( int, bool = false );
string& toStringSecondsRemaining( int, string&, bool = false );

string::size_type etLength( const string& );

string& etDecolorize ( string& );
string& etAlignLeft  ( const string&, string::size_type, string& );
string& etAlignRight ( const string&, string::size_type, string& );
string  etTruncate   ( const string&, string::size_type );
string& etTruncate   ( string&, string::size_type );

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_STR_UTIL_H
