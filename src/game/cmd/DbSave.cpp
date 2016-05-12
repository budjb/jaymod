#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

DbSave::DbSave()
    : AbstractBuiltin( "dbsave" )
{
    __usage << xvalue( "!" + _name );
    __descr << "Save the in-memory Admin System database to disk.";
}

///////////////////////////////////////////////////////////////////////////////

DbSave::~DbSave()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
DbSave::doExecute( Context& txt )
{
    if (txt._args.size() != 1)
        return PA_USAGE;

    levelDB.save();
    userDB.save();

    Buffer buf;
    buf << "saved: " << xvalue( int(levelDB.mapLEVEL.size()) ) << " levels\n"
        << "saved: " << xvalue( int(userDB.mapGUID.size()) ) << " users\n";
    printCpm( txt._client, buf, true );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
