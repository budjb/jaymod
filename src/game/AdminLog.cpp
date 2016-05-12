#include <bgame/impl.h>

///////////////////////////////////////////////////////////////////////////////

AdminLog::AdminLog()
{
}

///////////////////////////////////////////////////////////////////////////////

AdminLog::~AdminLog()
{
    if (_out.is_open())
        _out.close();
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::cvarCallback( Cvar& cvar )
{
    adminLog.recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::init()
{
    recompute();
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::log( Client* actor, const vector<string>& args, bool denied )
{
    if (!_out.is_open())
        return;

    if (args.empty())
        return;

    struct Entry {
        const char* guid;
        const char* name;
        int         slot;
    };

    Entry entry;
    if (!actor) {
        entry.guid = "";
        entry.name = "console";
        entry.slot = -1;
    }
    else {
        User& user = *connectedUsers[ actor->slot ];
        entry.guid = user.guid.c_str();
        entry.name = user.name.c_str();
        entry.slot = actor->slot;
    }

    // compute timestamp
    time_t now = time( NULL );
    tm* lt = localtime( &now );
    char stime[32];
    strftime( stime, sizeof(stime), "%c", lt);

    string cline;
    str::concatArgs( args, cline );

    ostringstream oss;
    oss << (denied ? '-' : '+')
        << '[' << stime << ']'
        << " [" << setw(2) << entry.slot << ']'
        << " [" << setfill('-') << setw(32) << entry.guid << '/' << entry.name << ']'
        << ' ' << cline;
    _out << oss.str() << endl;
}

///////////////////////////////////////////////////////////////////////////////

void
AdminLog::recompute()
{
    const string s = cvars::g_adminLog.svalue;

    if (_out.is_open() && s.empty()) {
        _out.close();
        _out.clear();
        _filename.clear();
    }
    else if ((!_out.is_open() && !s.empty()) || (_out.is_open() && s != _filename)) {
        _out.close();
        _out.clear();
        _filename = s;
        _out.open( _filename.c_str(), ios::app );

        if (_out.rdstate()) {
            _out.close();
            _out.clear();
            _filename.clear();

            ostringstream msg;
            msg.str( "" );
            msg << "-------" << endl
                << "------- WARNING: unable to open " << _filename << " ." << endl
                << "------- Please verify file is available for write access." << endl
                << "-------" << endl;
            trap_Printf( msg.str().c_str() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

AdminLog adminLog;
