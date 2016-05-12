#include <bgame/impl.h>

#if defined( CGAMEDLL )
#    define BG_PRINTF  CG_Printf
#    define BG_XXCPU   "cl_cpu"
#elif defined( GAMEDLL )
#    define BG_PRINTF  G_Printf
#    define BG_XXCPU   "sv_cpu"
#elif defined( UIDLL )
#    define BG_PRINTF  Com_Printf
#    define BG_XXCPU   "ui_cpu"
#else
#    error "DLL-MODULE is not defined."
#endif

#define NOGDI
#include <windows.h>

#ifndef PROCESSOR_AMD_X8664
#define PROCESSOR_AMD_X8664  8664
#endif

//////////////////////////////////////////////////////////////////////////////

void BG_cpuUpdate() {
    char* model;

    SYSTEM_INFO info;
    GetSystemInfo( &info );

    switch (info.dwProcessorType) {
        case PROCESSOR_INTEL_386:
            model = "i386";
            break;

        case PROCESSOR_INTEL_486:
            model = "i486";
            break;

        case PROCESSOR_INTEL_PENTIUM:
            model = "i586";
            break;

        case PROCESSOR_INTEL_IA64:
            model = "ia64";
            break;

        case PROCESSOR_AMD_X8664:
            model = "x86-64";
            break;

        default:
            model = "unknown";
            break;
    }

    trap_Cvar_Set( BG_XXCPU, model );
}
