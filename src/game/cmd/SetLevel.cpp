#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

SetLevel::SetLevel()
    : AbstractBuiltin( "setlevel" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << xvalue( "LEVEL" );
    __descr << "Change a specific player's admin level.";
}

///////////////////////////////////////////////////////////////////////////////

SetLevel::~SetLevel()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
SetLevel::doExecute( Context& txt )
{
    if (txt._args.size() != 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    Level& lev = lookupLEVEL( txt._args[2], txt );
    if (lev == Level::BAD)
        return PA_ERROR;

    // bail if raising level
    if (txt._user.authLevel < lev.level) {
        txt._ebuf << "You cannot set a level higher than your own.";
        return PA_ERROR;
    }

    // bail if fake GUID
    User& targetUser = *connectedUsers[target->slot];
    if (targetUser.fakeguid) {
        txt._ebuf << xvalue( targetUser.namex ) << " has no GUID.";
        return PA_ERROR;
    }

    targetUser.authLevel = lev.level;

    // Report success
    Buffer buf;
    buf << _name << ": " << xvalue( targetUser.namex ) << "'s level set to " << xvalue( lev.level );
    printCpm(txt._client, buf, true);

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
