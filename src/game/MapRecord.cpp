#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

MapRecord::MapRecord()
   : name             ( _name )
   , timestamp        ( 0 )
   , count            ( 0 )
   , longestSpree     ( 0 )
   , longestSpreeTime ( 0 )
{

}

///////////////////////////////////////////////////////////////////////////////

MapRecord::MapRecord( const MapRecord& mapRecord )
    : name ( _name )
{
    operator=( mapRecord );
}

///////////////////////////////////////////////////////////////////////////////

MapRecord::~MapRecord()
{

}

///////////////////////////////////////////////////////////////////////////////

void
MapRecord::decode( map<string,string>& data )
{
    // Timestamp
    stringstream ss;
    ss << data["timestamp"];
    ss >> timestamp;
    if (timestamp < 0)
        timestamp = 0;

    // Map counter
    ss.str("");
    ss.clear();
    ss << data["count"];
    ss >> count;
    if (count < 0)
        count = 0;

    // Longest Killing Spree
    ss.str("");
    ss.clear();
    ss << data["longestspree"];
    ss >> longestSpree;
    if (longestSpree > 0) {
        // Timestamp
        ss.str("");
        ss.clear();
        ss << data["longestspreetime"];
        ss >> longestSpreeTime;

        // Name
        longestSpreeName = data["longestspreename"];
        longestSpreeNamex = data["longestspreenamex"];
    }
}

///////////////////////////////////////////////////////////////////////////////

void
MapRecord::encode( ostream& out, int recnum )
{
    out << '\n';
    if (recnum > 0)
        out << '\n' << "###############################################################################" << '\n';

    out << '\n' << "name = " << name;

    char ftbuf[32];
    strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &timestamp ));

    out << '\n' << "timestamp = " << timestamp << " # " << ftbuf;
    out << '\n' << "count = " << count;

    if (longestSpree > 0) {
        out << '\n' << "longestspree = " << longestSpree;

        strftime( ftbuf, sizeof(ftbuf), "%c", localtime( &longestSpreeTime ));
        out << '\n' << "longestspreetime = " << longestSpreeTime << " # " << ftbuf;

        out << '\n' << "longestspreename = " << longestSpreeName;
        out << '\n' << "longestspreenamex = " << longestSpreeNamex;
    }
}

///////////////////////////////////////////////////////////////////////////////

MapRecord&
MapRecord::operator=( const MapRecord& mapRecord )
{
    // Standard stuff
    _name     = mapRecord._name;
    timestamp = mapRecord.timestamp;
    count     = mapRecord.count;

    // Longest killing spree
    longestSpree      = mapRecord.timestamp;
    longestSpreeName  = mapRecord.longestSpreeName;
    longestSpreeNamex = mapRecord.longestSpreeNamex;
    longestSpreeTime  = mapRecord.longestSpreeTime;

    return *this;
}

///////////////////////////////////////////////////////////////////////////////

bool
MapRecord::operator==( const MapRecord& mapRecord ) const
{
    return this == &mapRecord;
}

///////////////////////////////////////////////////////////////////////////////

bool
MapRecord::isNull()
{
    return this == &MapRecord::BAD;
}

///////////////////////////////////////////////////////////////////////////////

MapRecord MapRecord::BAD;
