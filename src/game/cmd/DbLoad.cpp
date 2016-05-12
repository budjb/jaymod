#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

DbLoad::DbLoad()
    : AbstractBuiltin( "dbload" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Reload (read & merge) the Admin System database files.";
}

///////////////////////////////////////////////////////////////////////////////

DbLoad::~DbLoad()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
DbLoad::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    G_DbLoad();

    Buffer buf;
    buf << "loaded: " << xvalue( int(levelDB.mapLEVEL.size()) ) << " level records"
        << '\n' << "loaded: " << xvalue( int(userDB.mapGUID.size()) ) << " user records"
        << '\n' << "loaded: " << xvalue( int(mapDB.mapNAME.size()) ) << " map records";

    if (g_censor.integer)
        buf << '\n' << "loaded: " << xvalue( int(censorDB.wordSet.size()) ) << " censor records";

    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
