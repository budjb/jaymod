#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

LevDelete::LevDelete()
    : AbstractBuiltin( "levdelete" )
{
    __usage << xvalue( "!" + _name ) << ' ' << xvalue( "LEVEL" ) << ' ' << _ovalue( "MIGRATE_LEVEL" );

    __descr << "Remove the admin level specified by " << xvalue( "LEVEL" ) << '.'
           << " Effected users will automatically be migrated to the level optionally specified"
           << " by " << xvalue( "MIGRATE_LEVEL" ) << " (default: " << xvalue( 0 ) << ").";
}

///////////////////////////////////////////////////////////////////////////////

LevDelete::~LevDelete()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
LevDelete::doExecute( Context& txt )
{
    if (txt._args.size() < 2 || txt._args.size() > 3)
        return PA_USAGE;

    // bail on invalid level
    Level& lev = lookupLEVEL( txt._args[1], txt );
    if (lev == Level::BAD)
        return PA_ERROR;

    // bail if trying to remove built-in
    if (lev.level < Level::CUSTOM_MIN || lev.level > Level::CUSTOM_MAX) {
        txt._ebuf << "Cannot remove a builtin level.";
        return PA_ERROR;
    }

    // bail on invalid migration level
    const Level* mlev = &Level::DEFAULT;
    if (txt._args.size() > 2) {
        string err;
        mlev = &lookupLEVEL( txt._args[2], txt, "MIGRATION_LEVEL" );
        if (*mlev == Level::BAD )
            return PA_ERROR;

        if (*mlev == lev) {
            txt._ebuf << xvalue( "LEVEL" ) << " and " << xvalue( "MIGRATION_LEVEL" ) << " are the same.";
            return PA_ERROR;
        }
    }

    Buffer buf;
    buf << _name << ": Level ";
    if (!lev.namex.empty())
        buf << xvalue( lev.level ) << " (" << lev.namex << ')';
    else
        buf << xvalue( lev.level );
    buf << " removed.";

    const uint32 mnum = levelDB.remove( lev, *mlev ); // CAUTION: reference now invalid

    buf << ' ' << xvalue( mnum) << " user" << (mnum == 1 ? "" : "s")
        << " migrated to level " << xvalue( mlev->level ) << '.';

    printCpm( txt._client, buf, true );
    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
