#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

AdminTest::AdminTest()
    : AbstractBuiltin( "admintest", true )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "-all" );
    __descr << "Displays your current admin level.";
}

///////////////////////////////////////////////////////////////////////////////

AdminTest::~AdminTest()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
AdminTest::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    // bail if console
    if (txt._user == User::CONSOLE) {
        txt._ebuf << "You are on the console.";
        return PA_ERROR;
    }

    // fetch level
    string err;
    const Level& lev = levelDB.fetchByKey( txt._user.authLevel, err );

    Buffer buf;
    buf << _name << ": " << xvalue( txt._user.namex ) << " is a ";

    if (lev.namex.empty())
        buf << "level " << xvalue( lev.level ) << " admin.";
    else
        buf << xvalue( lev.namex ) << " (level " << xvalue( lev.level ) << " admin).";

    bool broadcast = false;
    if (txt._args.size() > 1) {
        string s = txt._args[1];
        str::toLower( s );
        if (s == "-all" )
            broadcast = true;
        else
            return PA_USAGE;
    }

    printCpm( txt._client, buf, broadcast );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
