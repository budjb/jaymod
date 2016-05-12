#include <bgame/impl.h> 

///////////////////////////////////////////////////////////////////////////////

namespace {
    ostringstream accumulators[_TPRINT_MAX];
}

///////////////////////////////////////////////////////////////////////////////

void
CG_ConfigureDevice( text::ETDevice& dev )
{
    glconfig_t& cfg = cgs.glconfig;

    dev.indent = cfg.vidWidth < 1024 ? 2 : 4;
    dev.cols   = cfg.vidWidth / 8 - 4;
    dev.rows   = cfg.vidHeight / 16 - 2;
}

///////////////////////////////////////////////////////////////////////////////

void
CG_TextPrint( TPrint dest, bool accumulate )
{
    ostringstream& accum = accumulators[dest];

    char buffer[MAX_STRING_CHARS];
    trap_Args( buffer, sizeof(buffer) );

    char dec[MAX_STRING_CHARS]; // slightly more than needed but ok
    memset( dec, 0, sizeof(dec) );
    base64::decode( (const unsigned char*)buffer, (unsigned char*)dec, sizeof(dec) );
    accum << dec;

    if (accumulate)
        return;

    string data = accum.str();
    text::Buffer buf( (const uint8*)data.data(), data.length() );

    text::ETDevice dev;
    switch (dest) {
        default:
        case TPRINT_CONSOLE:
        case TPRINT_PM:
        case TPRINT_PM_SILENT:
            break;

        case TPRINT_CHAT:
        case TPRINT_CPM:
            dev.cnormal = 'L';
            break;
    }

    CG_ConfigureDevice( dev );
    list<string> output;
    dev.format( buf, output );

    const list<string>::iterator end = output.end();
    switch (dest) {
        default:
        case TPRINT_CONSOLE:
            for ( list<string>::iterator it = output.begin(); it != end; it++ )
                trap_Print( it->c_str() );
            break;

        case TPRINT_CHAT:
            for ( list<string>::iterator it = output.begin(); it != end; it++ ) {
                CG_AddToTeamChat( it->c_str(), -1, -1 );
                trap_Print( it->c_str() );
            }
            break;

        case TPRINT_CPM:
            for ( list<string>::iterator it = output.begin(); it != end; it++ )
                CG_AddPMItem( PM_MESSAGE, it->c_str(), cgs.media.voiceChatShader, 0 );
            break;

        case TPRINT_PM:
            for ( list<string>::iterator it = output.begin(); it != end; it++ ) {
                CG_AddToTeamChat( it->c_str(), -1, -1 );
                trap_Print( it->c_str() );
            }
            if (cg_pmsounds.integer)
                trap_S_StartLocalSound( cgs.media.pmsound, CHAN_AUTO );
            break;

        case TPRINT_PM_SILENT:
            for ( list<string>::iterator it = output.begin(); it != end; it++ ) {
                CG_AddToTeamChat( it->c_str(), -1, -1 );
                trap_Print( it->c_str() );
            }
            break;
    }

    accum.str( "" );
}
