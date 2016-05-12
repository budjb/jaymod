#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

CensorDB::CensorDB()
    : wordSet( _wordSet )
{
}

///////////////////////////////////////////////////////////////////////////////

CensorDB::~CensorDB()
{
}

///////////////////////////////////////////////////////////////////////////////

bool
CensorDB::open()
{
    ostringstream msg;
    _stream.clear();

    if (!g_censor.integer)
        return false;

    // Get paths
    char buffer[ MAX_CVAR_VALUE_STRING ];
    trap_Cvar_VariableStringBuffer( "fs_homepath", buffer, sizeof( buffer ));
    string fname = buffer;
    fname += "/";

    trap_Cvar_VariableStringBuffer( "fs_game", buffer, sizeof( buffer ));
    fname += buffer;
    fname += "/";
    fname += "censor.db";

    // Set mode, print function
    msg << "Reading from " << fname << " ..." << endl;

    trap_Printf( msg.str().c_str() );

    // Open file
    _stream.clear();
    _stream.open( fname.c_str(), ios::in );
    if (!_stream.rdstate())
        return false;

    // Error
    msg.str( "" );
    msg << "-------" << endl
        << "------- WARNING: unable to open " << fname << " ." << endl
        << "------- Please verify file is available for access." << endl
        << "-------" << endl;
    trap_Printf( msg.str().c_str() );

    return true;
}

///////////////////////////////////////////////////////////////////////////////

void
CensorDB::close()
{
    _stream.close();
    _stream.clear();
}

///////////////////////////////////////////////////////////////////////////////

void
CensorDB::load() {
    if (open())
        return;

    _wordSet.clear();
    string word;

    do {
        _stream >> word;

        if (!word.empty()) {
            str::toLower( word );
            _wordSet.insert( word );
        }
    } while (!_stream.rdstate());

    close();
}

///////////////////////////////////////////////////////////////////////////////

// This is converted from the ET SDK SanitizeString()
void
CensorDB::sanitize( string& s, bool lower )
{
    string str;

        string::size_type i = 0;
        while(i < s.length()) {
                if(s[i] == 27 || s[i] == '^') {
                        i++;            // skip color code
                        if(i < s.length()) i++;
                        continue;
                }

                if(s[i] < 32) {
                        i++;
                        continue;
                }

                str += (lower) ? tolower(s[i++]) : s[i++];
        }

    s = str;
}

///////////////////////////////////////////////////////////////////////////////

bool
CensorDB::isAlphaNum( char c ) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
}

///////////////////////////////////////////////////////////////////////////////

bool
CensorDB::filterWord( string& word ) {
    string compare = word;
    
    sanitize(compare, true);

    WordSet::const_iterator found = _wordSet.find(compare);

    if (found == _wordSet.end()) {
        // Word is ok
        return false;
    } else {
        // Censor it
        string::size_type i;
        string::size_type length;
        string censor;

        for (length = word.length(), i = 0; i < length; i++) {
            if (word[i] != '^') {
                censor += '*';
            } else {
                censor += '^';
                if (++i < length) {
                    censor += word[i];
                }
            }
        }
        word = censor;
        return true;
    }
}

///////////////////////////////////////////////////////////////////////////////

bool
CensorDB::filter( string& str )
{
    string  word;
    string  buf;
    bool    censored = false;

    string::size_type i;
    string::size_type length;
    for (i = 0, length = str.length(); i < length; i++) {
        const char c = str[i];

        // Color codes support
        if (c == '^') {
            if (word.length()) {
                word += c;
            } else {
                buf += c;
            }

            // We also need the color if it exists
            if (++i < length) {
                if (word.length()) {
                    word += str[i];
                } else {
                    buf += str[i];
                }
            }
        }
        else if (isAlphaNum(c)) {
            // Add to censor check
            word += c;
        }
        else {
            // This character is not a word character
            // If there is an outstanding word, now's the time to test it
            if (word.length()) {
                if (filterWord(word))
                    censored = true;
                buf += word;
                word = "";
            }

            // Still, add char
            buf += c;
        }
    }

    // Lastly, make sure there is no straggler
    if (word.length()) {
        if (filterWord(word))
            censored = true;
        buf += word;
    }

    str = buf;

    return censored;
}

///////////////////////////////////////////////////////////////////////////////

CensorDB censorDB;
