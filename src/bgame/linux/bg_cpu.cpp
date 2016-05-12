#if defined( CGAMEDLL )
#    include <cgame/cg_local.h>
#    define BG_PRINTF  CG_Printf
#    define BG_XXCPU   "cl_cpu"
#elif defined( GAMEDLL )
#    include <game/g_local.h>
#    define BG_PRINTF  G_Printf
#    define BG_XXCPU   "sv_cpu"
#elif defined( UIDLL )
#    include <ui/ui_local.h>
#    define BG_PRINTF  Com_Printf
#    define BG_XXCPU   "ui_cpu"
#else
#    error "DLL-MODULE is not defined."
#endif

#include <sys/utsname.h>

//////////////////////////////////////////////////////////////////////////////

static int cpuModelFetch(char* cpuModel, int cpuModelSize) {
    char  buffer[ 4096 ];
    FILE* file;
    int   nread;
    char* p;
    char* pmax;
    int   field;
    char* token;
    char  tokenName[ 32 ];
    char  tokenValue[ 64 ];

    memset( cpuModel, 0, cpuModelSize );

    file = fopen( "/proc/cpuinfo", "r" );
    if (!file)
        return -1;

    nread = fread( buffer, 1, sizeof(buffer), file );
    if (!nread) {
        fclose(file);
        return -1;
    }

    field = 0;
    memset( tokenName, 0, sizeof(tokenName) );
    token = tokenName;
    pmax = buffer + nread;

    for (p = buffer; p < pmax; p++) {
        switch (field) {
            default:
            case 0: // BUILD NAME
                switch (*p) {
                    case '\n':
                    case '\r':
                        field = 0;
                        memset( tokenName, 0, sizeof(tokenName) );
                        token = tokenName;
                        break;

                    case ':':
                        field++;
                        memset( tokenValue, 0, sizeof(tokenValue) );
                        token = tokenValue;
                        break;

                    case ' ':
                    case '\t':
                        break;

                    default:
                        if (token < (tokenName + sizeof(tokenName) - 1)) {
                            *token = *p;
                            token++;
                        }
                        break;
                }
                break;

            case 1: // BUILD VALUE
                switch (*p) {
                    case '\n':
                    case '\r':
                        if (strcasecmp( tokenName, "modelname" ) == 0)
                            strncpy( cpuModel, tokenValue, cpuModelSize-1 );
                        field = 0;
                        memset( tokenName, 0, sizeof(tokenName) );
                        token = tokenName;
                        break;

                    case ' ':
                    case '\t':
                        if (token > tokenValue && token < (tokenValue + sizeof(tokenValue) - 1)) {
                            *token = *p;
                            token++; 
                        }
                        break;

                    default:
                        if (token < (tokenValue + sizeof(tokenValue) - 1)) {
                            *token = *p;
                            token++; 
                        }
                        break;
                }
                break;
        }
    }

    fclose( file );
    return 0;
}

void BG_cpuUpdate() {
    char cpu[ MAX_CPU_BUFFER ];

    if ( !cpuModelFetch( cpu, sizeof(cpu) )) {
        trap_Cvar_Set( BG_XXCPU, cpu );
    }
    else {
        struct utsname info;
        if (!uname( &info ))
            trap_Cvar_Set( BG_XXCPU, info.machine );
    }
}
