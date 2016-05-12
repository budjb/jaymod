#ifndef BGAME_IMPL_H
#define BGAME_IMPL_H

///////////////////////////////////////////////////////////////////////////////

#if defined( CGAMEDLL )
#    include <cgame/cg_local.h>
#elif defined( GAMEDLL )
#    include <game/g_local.h>
#elif defined( UIDLL )
#    include <ui/ui_local.h>
#else
#    error "DLL-MODULE is not defined."
#endif

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_IMPL_H
