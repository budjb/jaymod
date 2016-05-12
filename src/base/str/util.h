#ifndef BASE_STR_UTIL_H
#define BASE_STR_UTIL_H

///////////////////////////////////////////////////////////////////////////////

void concatArgs( const vector<string>&, string&, vector<string>::size_type = 0 );

string& toLower     ( string& );
string  toLowerCopy ( const string& );

string& toUpper     ( string& );
string  toUpperCopy ( const string& );

string::size_type iFind     ( string, string, string::size_type );

bool isIndex( const string& );

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_STR_UTIL_H
