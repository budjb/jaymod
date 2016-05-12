#include <bgame/impl.h>

namespace cmd {

///////////////////////////////////////////////////////////////////////////////

Status::Status()
    : AbstractBuiltin( "status" )
{
    __usage << xvalue( "!" + _name ) << ' ' << _ovalue( "-all" );
    __descr << "Server status. Various bits of internal information relating to server resources and operation.";
}

///////////////////////////////////////////////////////////////////////////////

Status::~Status()
{
}

///////////////////////////////////////////////////////////////////////////////

AbstractCommand::PostAction
Status::doExecute( Context& txt )
{
    if (txt._args.size() > 2)
        return PA_USAGE;

    int vUsed = 0;
    int tUsed = 0;
    int cIndex = -1;
    int cUsed = 0;
    int sUsed = 0;

    char csbuf[BIG_INFO_VALUE];
    for (int i = 0; i < CS_MAX; i++) {
        trap_GetConfigstring( i, csbuf, sizeof(csbuf) );
        int len = strlen( csbuf ) + 1;
        sUsed += len;

        switch (i) {
            case CS_SERVERINFO:
                vUsed = len;
                break;

            case CS_SYSTEMINFO:
                tUsed = len;
                break;

            default:
                if (len > cUsed) {
                    cIndex = i;
                    cUsed = len;
                }
                break;
        }
    }
    const int   sLimit = MAX_GAMESTATE_CHARS;
    const float sUsage = 100.0f * float(sUsed) / float(sLimit);

    const int   vLimit = MAX_INFO_STRING;
    const float vUsage = 100.0f * float(vUsed) / float(vLimit);

    const int   tLimit = BIG_INFO_VALUE;
    const float tUsage = 100.0f * float(tUsed) / float(tLimit);

    const int   cLimit = MAX_INFO_STRING;
    const float cUsage = 100.0f * float(cUsed) / float(cLimit);

    const int   pUsed  = level.numConnectedClients;
    const int   pLimit = g_maxGameClients.integer < 1 ? MAX_CLIENTS : g_maxGameClients.integer;
    const float pUsage = 100.0f * float(pUsed) / float(pLimit);

    int eUsed = 0;
    for (int i = MAX_CLIENTS; i < level.num_entities; i++) {
        if ( g_entities[i].inuse )
            eUsed++;
    }
    const int   eLimit = ENTITYNUM_MAX_NORMAL - MAX_CLIENTS;
    const int   eHigh  = level.num_entities - MAX_CLIENTS;
    const float eUsage = 100.0f * float(eUsed)  / float(eLimit);

    InlineText colA;           // resource
    InlineText colB = xvalue;  // used
    InlineText colC = xvalue;  // limit
    InlineText colD = xnone;   // units
    InlineText colE = xvalue;  // usage

    colA.flags |= ios::left;
    colA.suffix = ":";
    colA.width  = 12;

    colB.width = 6;

    colC.flags |= ios::left;
    colC.width = 6;

    colD.width = 6;
    colD.flags |= ios::left;

    colE.suffix = "%";
    colE.width = 7;
    colE.flags |= ios::fixed;
    colE.precision = 2;

    Buffer buf;
    buf << xheader( "-SERVER STATUS" )
<< '\n' << colA("game" ) << xvalue( str::toString( (gametype_t)g_gametype.integer ))
        << '/' << xvalue( str::toString( (gamestate_t)cvars::gameState.ivalue ))
<< '\n' << colA("players"   ) << colB(pUsed) << "/" << colC(pLimit) << colD(""     ) << "(" << colE(pUsage) << ')'
<< '\n' << colA("entities"  ) << colB(eUsed) << "/" << colC(eLimit) << colD(""     ) << "(" << colE(eUsage) << ')'
        << " (" << xvalue(eHigh) << " high)"
<< '\n' << colA("gsdata"    ) << colB(sUsed) << "/" << colC(sLimit) << colD("bytes") << "(" << colE(sUsage) << ')'
<< '\n' << colA("serverinfo") << colB(vUsed) << "/" << colC(vLimit) << colD("bytes") << "(" << colE(vUsage) << ')'
<< '\n' << colA("systeminfo") << colB(tUsed) << "/" << colC(tLimit) << colD("bytes") << "(" << colE(tUsage) << ')'
<< '\n' << colA("largestcs" ) << colB(cUsed) << "/" << colC(cLimit) << colD("bytes") << "(" << colE(cUsage) << ')'
        << " (" << xvalue(cIndex) << " index)";

    colA.width = 7;

    colB.flags |= ios::right;
    colB.suffixOutside = " records";
    colB.width = 4;

    buf << '\n' << xheader( "-DATABASE" )
        << '\n' << colA("user")  << colB( userDB.mapGUID.size() )
            << "  (" << xvalue( userDB.mapBANTIME.size() ) << " bans)"
        << '\n' << colA("level") << colB( levelDB.mapLEVEL.size() )
        << '\n' << colA("map")   << colB( mapDB.mapNAME.size() );

    colA.width = 14;

    colB.flags |= ios::right;
    colB.suffix = " KB";
    colB.suffixOutside = "";
    colB.width = 9;

    colC.flags |= ios::right;
    colC.suffix = " KB";
    colC.width = 8;

    const int bUsed = AbstractBulletModel::heapUsed / 1024;
    const int bHigh = AbstractBulletModel::heapHighWatermark / 1024;

    const int hUsed = AbstractHitModel::heapUsed / 1024;
    const int hHigh = AbstractHitModel::heapHighWatermark / 1024;

    buf << "\n" << xheader( "-HEAP" )
        << "\n" << colA("bullet-model") << colB(bUsed) << "  (" << colC(bHigh) << " high)"
        << "\n" << colA("hit-model")    << colB(hUsed) << "  (" << colC(hHigh) << " high)";

    colA.width = 14;

    colB.flags |= ios::right | ios::fixed;
    colB.suffix = "/s";
    colB.width = 9;
    colB.precision = 2;

    buf << "\n" << xheader( "-RATES" )
        << "\n" << colA("entity spawn") << colB( stats::entitySpawn.avg() )
        << "\n" << colA("entity free")  << colB( stats::entityFree.avg() )
        << "\n" << colA("frames")       << colB( stats::frame.avg() );

    bool broadcast = false;
    if (txt._args.size() > 1) {
        string s = txt._args[1];
        str::toLower( s );
        if (s == "-all" )
            broadcast = true;
        else
            return PA_USAGE;
    }

    print( txt._client, buf, broadcast );

    return PA_NONE;
}

///////////////////////////////////////////////////////////////////////////////

} // namespace cmd
