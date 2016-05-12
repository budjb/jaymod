#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Ban::Ban()
    : AbstractBuiltin( "ban" )
{
    __usage << xvalue( "!" + _name )
            << ' ' << xvalue( "PLAYER" )
            << ' ' << _ovalue( "SECONDS" )
            << ' ' << _ovalue( "REASON" );

    __descr << "Ban an online player.";
}

///////////////////////////////////////////////////////////////////////////////

Ban::~Ban()
{
}

///////////////////////////////////////////////////////////////////////////////

void
Ban::doBan( User& user, User& authority, int duration, const string& reason, Buffer& out, const Client* client )
{
    out << xvalue( user.namex ) << " banned " << (duration ? "for " : "")
        << (duration ? str::toStringSecondsRemaining( duration, true ) : "permanently")
        << '.';

    userDB.unindex( user );

    // Set up ban record
    user.banned = true;
    user.banTime = time( NULL );
    user.banExpiry = duration ? user.banTime + duration : 0; // 0 is a permanent ban
    user.banReason = reason;
    user.banAuthority = authority.name;
    user.banAuthorityx = authority.namex;

    userDB.index( user );

    if (!client)
        return;

    Buffer buf;
    buf << '\n' << "user: " << xvalue( user.namex )
        << '\n'
        << '\n' << "duration:"
        << '\n' << xvalue( duration ? str::toStringSecondsRemaining( duration, true ) : "PERMANENT" )
        << '\n'
        << '\n' << "reason:"
        << '\n' << xvalue( reason );
    SEngine::dropClient( client->slot, buf, "You have been banned." );
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Ban::doExecute( Context& txt )
{
    if (txt._args.size() < 2)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    User& user = *connectedUsers[target->slot];
    if (user == txt._user) {
        txt._ebuf << "You cannot ban yourself.";
        return PA_ERROR;
    }

    if (isBotError( *target, txt ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
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

    Buffer buf;
    buf << _name << ": ";
    doBan( user, txt._user, banDuration, banReason, buf, target );
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
