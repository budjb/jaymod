#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

BanUser::BanUser()
    : AbstractBuiltin( "banuser" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "USER" )
            << ' ' << _ovalue( "SECONDS" )
            << ' ' << _ovalue( "REASON" );

    __descr << "Ban a user. Useful for banning players which are not connected.";
}

///////////////////////////////////////////////////////////////////////////////

BanUser::~BanUser()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
BanUser::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    // bail on invalid id
    const string& id = txt._args[1];
    User& user = lookupUSER( id, txt );
    if (user == User::BAD)
        return PA_ERROR;

    if (user == txt._user) {
        txt._ebuf << "You cannot ban yourself.";
        return PA_ERROR;
    }

    if (isHigherLevelError( user, txt ))
        return PA_ERROR;

    int banDuration = 0;
    if (txt._args.size() > 2)
        banDuration = str::toSeconds( txt._args[2] );

    // compute reason text
    string banReason;
    if (txt._args.size() > 3)
        str::concatArgs( txt._args, banReason, 3 );

    if (banDuration < 1) {
        if (!txt._user.hasPrivilege( priv::base::banPermanent )) {
            txt._ebuf << "You must specify a non-permanent ban duration.";
            return PA_ERROR;
        }

        banDuration = 0;
    }

    if (banReason.empty()) {
        if (!txt._user.hasPrivilege( priv::base::reasonNone )) {
            txt._ebuf << "You must specify a ban reason.";
            return PA_ERROR;
        }

        banReason = "none";
    }

    // check if user happens to be online so their connection is dropped
    Client* target = NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (user == *connectedUsers[i]) {
            target = &g_clientObjects[i];
            break;
        }
    }

    Buffer buf;
    buf << _name << ": ";
    Ban::doBan( user, txt._user, banDuration, banReason, buf, target );
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
