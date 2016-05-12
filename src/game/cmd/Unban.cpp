#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Unban::Unban()
    : AbstractBuiltin( "unban" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" );
    __descr << "Unban a specific player.";
}

///////////////////////////////////////////////////////////////////////////////

Unban::~Unban()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Unban::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    // bail on invalid id
    const string& id = txt._args[1];
    User& user = lookupUSER( id, txt );
    if (user == User::BAD)
        return PA_ERROR;

    userDB.unindex( user );
    user.banned = false;
    userDB.index( user );

    Buffer buf;
    buf << _name << ": User ID " << xvalue( id ) << " (" << xvalue( user.namex ) << ") unbanned.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
