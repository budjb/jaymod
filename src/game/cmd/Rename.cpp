#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Rename::Rename()
    : AbstractBuiltin( "rename" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "PLAYER" ) << ' ' << xvalue( "NEW_NAME" );
    __descr << "Change a specific player's name.";
}

///////////////////////////////////////////////////////////////////////////////

Rename::~Rename()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Rename::doExecute( Context& txt )
{
    if (txt._args.size() < 3)
        return PA_USAGE;

    Client* target;
    if (lookupPLAYER( txt._args[1], txt, target ))
        return PA_ERROR;

    if (isHigherLevelError( *target, txt ))
        return PA_ERROR;

    string newname;
    str::concatArgs( txt._args, newname, 2 );

    char userinfo[MAX_INFO_STRING];
    trap_GetUserinfo( target->slot, userinfo, sizeof(userinfo) );
    const string oldname = Info_ValueForKey( userinfo, "name" );

    // set the name
    Info_SetValueForKey( userinfo, "name", newname.c_str() );
    trap_SetUserinfo( target->slot, userinfo );
    ClientUserinfoChanged( target->slot );

    Buffer buf;
    buf << _name << ": " << xvalue( oldname ) << " renamed to " << xvalue( newname );
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
