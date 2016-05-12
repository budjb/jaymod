#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

UserDelete::UserDelete()
    : AbstractBuiltin( "userdelete" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USERID" );
    __descr << "Remove the user specified by " << xvalue( "USERID" );
}

///////////////////////////////////////////////////////////////////////////////

UserDelete::~UserDelete()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
UserDelete::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    // bail on invalid id
    const string& id = txt._args[1];
    User& target = lookupUSER( id, txt );
    if (target == User::BAD)
        return PA_ERROR;

    // bail if trying to remove online-user
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (connectedUsers[i] == &target) {
            txt._ebuf << "Cannot remove connected user: " << xvalue( target.namex );
            return PA_ERROR;
        }
    }

    Buffer buf;
    buf << _name << ": User ID " << xvalue( id ) << " (" << xvalue( target.namex ) << ") removed.";

    userDB.remove( target ); // CAUTION: reference now invalid

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
