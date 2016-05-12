#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

LevAdd::LevAdd()
    : AbstractBuiltin( "levadd" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "NUM" );
    __descr << "Create a new admin level.";
}

///////////////////////////////////////////////////////////////////////////////

LevAdd::~LevAdd()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
LevAdd::doExecute( Context& txt )
{
    if (txt._args.size() != 2)
        return PA_USAGE;

    string err;
    const string& key = txt._args[1];

    if (levelDB.fetchByKey( key, err ) != Level::BAD) {
        txt._ebuf << "Level " << xvalue( key ) << " already exists.";
        return PA_ERROR;
    }

    // create
    Level& lev = levelDB.fetchByKey( key, err, true );
    if (lev == Level::BAD) {
        txt._ebuf << "NUM " << err << ": " << xvalue( key ) << " .";
        return PA_ERROR;
    }

    Buffer buf;
    buf << _name << ": Level " << xvalue( key ) << " added.";

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
