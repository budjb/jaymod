#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

Level::Level()
    : _level ( -1 )
    , level  ( _level )
{
}

///////////////////////////////////////////////////////////////////////////////

Level::Level( const Level& lev )
    : level ( _level )
{
    operator=( lev );
}

///////////////////////////////////////////////////////////////////////////////

Level::~Level()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Level::decode( map<string,string>& data )
{
    stringstream ss;

    // Level
    ss.str("");
    ss << data["level"];
    ss >> _level;

    if (_level < 0)
        _level = 0;
    else if (_level > NUM_MAX)
        _level = 0;

    // MIKE TODO: for 2.1.7 release we can remove concat'd PRIVILEGES
    PrivilegeSet::decode( privGranted, privDenied, data["acl"] + ' ' + data["privileges"], data["flags"] );

    name = data["name"];
    namex = data["namex"];

    // namex has priority
    if (namex.empty())
        namex = name;
    else
        name = namex;

    str::etDecolorize( name );

    // Greeting
    greetingText = data["greetingtext"];
    greetingAudio = data["greetingaudio"];
}

///////////////////////////////////////////////////////////////////////////////

void
Level::encode( ostream& out, int recnum )
{
    out << '\n';
    if (recnum > 0)
        out << '\n' << "###############################################################################" << '\n';

    out << '\n' << "level = " << level;
    out << '\n' << "name = " << name;
    out << '\n' << "namex = " << namex;
    out << '\n' << "greetingtext = " << greetingText;
    out << '\n' << "greetingaudio = " << greetingAudio;

    out << '\n' << "acl = ";
    PrivilegeSet::encode( privGranted, privDenied, out );
}

///////////////////////////////////////////////////////////////////////////////

bool
Level::isNull() const
{
    return this == &BAD;
}

///////////////////////////////////////////////////////////////////////////////

bool
Level::operator==( const Level& lev ) const
{
    return this == &lev;
}

///////////////////////////////////////////////////////////////////////////////

bool
Level::operator!=( const Level& lev ) const
{
    return this != &lev;
}

///////////////////////////////////////////////////////////////////////////////

Level&
Level::operator=( const Level& lev )
{   
    _level        = lev._level;
    name          = lev.name;
    namex         = lev.namex;
    greetingText  = lev.greetingText;
    greetingAudio = lev.greetingAudio;
    privGranted   = lev.privGranted;
    privDenied    = lev.privDenied;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

Level Level::BAD;
Level Level::DEFAULT;

const int Level::NUM_MIN = 0;
const int Level::NUM_MAX = 999;

const int Level::CUSTOM_MIN = 1;
const int Level::CUSTOM_MAX = 998;
