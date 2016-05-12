#if defined( CGAMEDLL )
#    include <cgame/cg_local.h>
#    define BG_PRINTF CG_Printf
#    define BG_XXCPU  "cl_cpu"
#elif defined( GAMEDLL )
#    include <game/g_local.h>
#    define BG_PRINTF G_Printf
#    define BG_XXCPU  "sv_cpu"
#elif defined( UIDLL )
#    include <ui/ui_local.h>
#    define BG_PRINTF Com_Printf
#    define BG_XXCPU  "ui_cpu"
#else
#    error "DLL-MODULE is not defined."
#endif

#include <sys/utsname.h>

//////////////////////////////////////////////////////////////////////////////

void BG_cpuUpdate() {
    struct utsname info;

    if (!uname( &info )) {
        char cpu[ MAX_CPU_BUFFER ];
        strncpy( cpu, info.machine, sizeof(cpu) );
        cpu[ sizeof(cpu)-1 ] = 0;
        trap_Cvar_Set( BG_XXCPU, cpu );
    }
}
