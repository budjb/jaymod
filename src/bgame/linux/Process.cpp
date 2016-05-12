#if defined( CGAMEDLL )
#    include <cgame/cg_local.h>
#elif defined( GAMEDLL )
#    include <game/g_local.h>
#elif defined( UIDLL )
#    include <ui/ui_local.h>
#else
#    error "DLL-MODULE is not defined."
#endif

//////////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <sys/types.h>

#ifdef _DEBUG
#ifndef __USE_GNU
#   define __USE_GNU
#   include <sys/ucontext.h>
#   undef __USE_GNU
#else
#   include <sys/ucontext.h>
#endif
#endif // __DEBUG

#include <signal.h>
#include <errno.h>
#include <execinfo.h>

namespace {

//////////////////////////////////////////////////////////////////////////////

struct SigData
{
    int num;
    void (*handler)( int, siginfo_t*, void* );
    int flags;
    struct sigaction savedAction;
};

sigset_t sigSavedMask;

//////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void
coreTrace( int num, siginfo_t* info, ucontext_t* context )
{
    int    i;
    void*  array[1024];
    char** elements;
    int    size;

    size = backtrace( array, sizeof(array) / sizeof(void*) );
    printf( "STACK ELEMENTS: %d\n", size-1 );

    // This code segment taken from etpub.
    // Set the actual calling address for accurate stack traces.
    // If we don't do this stack traces are less accurate.
    array[1] = (void*)context->uc_mcontext.gregs[REG_EIP];

    elements = backtrace_symbols(array, size);

    for (i = 1; i < size; i++)
        printf( "[%02d] %s\n", i, elements[i] );
}
#endif // __DEBUG

//////////////////////////////////////////////////////////////////////////////

void
handlerMsg( int num, siginfo_t* info, const char* name, const char* action )
{
    time_t now = time( 0 );
    char fnow[32];
    strftime( fnow, sizeof(fnow), "%c", localtime( &now ));

    ostringstream msg;
    msg <<   "-------"
        << "\n------- TIMESTAMP: " << fnow
        << "\n------- CAUGHT OS SIGNAL: " << name << " (" << num << ")"
        << "\n-------     si_errno = " << info->si_errno
        << "\n-------     si_code  = " << info->si_code
        << "\n-------     si_pid   = " << info->si_pid
        << "\n-------     si_uid   = " << info->si_uid
        << "\n------- ACTION: " << action
        << "\n-------" 
        << endl;

    trap_Print( msg.str().c_str() );
}

//////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
void
handlerCORE( int num, siginfo_t* info, void* context )
{
    printf( "SIGNAL CAUGHT: %d\n", num );
    coreTrace( num, info, (ucontext_t*)context );

    if (raise( num ))
        printf( "WARNING: unable to raise signal(%d): error #%d\n", num, errno );
}
#endif // __DEBUG

//////////////////////////////////////////////////////////////////////////////

void
handlerHUP( int num, siginfo_t* info, void* context )
{
    handlerMsg( num, info, "SIGHUP", "queued shutdown" );
    process.setPendingShutdown( true );

    /* We force stdin to close because if CVAR ttycon=1 and the parent
     * process supplying TTY is killed, etded.x86 likes to spin.
     * It is probably not necessary to do this when ttycon=0 but
     * there's no harm in it either since we're shutting down anyways.
     */
    close( fileno( stdin ));
}

//////////////////////////////////////////////////////////////////////////////

void
handlerTERM( int num, siginfo_t* info, void* context )
{
    handlerMsg( num, info, "SIGTERM", "queued shutdown" );
    process.setPendingShutdown( true );

    /* We force stdin to close because if CVAR ttycon=1 and the parent
     * process supplying TTY is killed, etded.x86 likes to spin.
     * It is probably not necessary to do this when ttycon=0 but
     * there's no harm in it either since we're shutting down anyways.
     */
    close( fileno( stdin ));
}

//////////////////////////////////////////////////////////////////////////////

void
handlerUSR1( int num, siginfo_t* info, void* context )
{
    handlerMsg( num, info, "SIGUSR1", "queued reload" );
    process.setPendingReload( true );
}

//////////////////////////////////////////////////////////////////////////////

SigData sigList[] = {
    { SIGHUP,  handlerHUP,  0 },
    { SIGTERM, handlerTERM, 0 },
    { SIGUSR1, handlerUSR1, 0 },
#ifdef _DEBUG
    { SIGINT,  handlerCORE, SA_RESETHAND },
    { SIGQUIT, handlerCORE, SA_RESETHAND },
    { SIGILL,  handlerCORE, SA_RESETHAND },
    { SIGABRT, handlerCORE, SA_RESETHAND },
    { SIGBUS,  handlerCORE, SA_RESETHAND },
    { SIGFPE,  handlerCORE, SA_RESETHAND },
    { SIGSEGV, handlerCORE, SA_RESETHAND },
#endif // __DEBUG
    { -1 },
};

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

Process::mstime_t
Process::mstime()
{
    timeval tv;
    gettimeofday( &tv, 0 );
    return mstime_t( tv.tv_sec ) * mstime_t( 1000 ) + mstime_t( tv.tv_usec ) / mstime_t( 1000 );
}

//////////////////////////////////////////////////////////////////////////////

void
Process::beginCriticalSection()
{
    sigset_t mask;
    sigfillset( &mask );
    sigprocmask( SIG_BLOCK, &mask, &sigSavedMask );
}

//////////////////////////////////////////////////////////////////////////////

void
Process::endCriticalSection()
{
    sigprocmask( SIG_SETMASK, &sigSavedMask, NULL );
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalInit()
{   
    struct sigaction action;
    SigData* data;

    // replace actions
    for (data = sigList; data->num != -1; data++) {
        memset( &action, 0, sizeof(action) );
        action.sa_flags = data->flags | SA_SIGINFO; // using 3-arg handler
        sigfillset( &action.sa_mask );
        action.sa_sigaction = data->handler;

        if (!sigaction( data->num, &action, &data->savedAction ))
            continue;

        printf( "WARNING: failed save/replace signal(%d): error #%d\n", data->num, errno );
    }
}

//////////////////////////////////////////////////////////////////////////////

void
Process::signalShutdown()
{
    SigData* data;

    for (data = sigList; data->num != -1; data++) {
        if (!sigaction( data->num, &data->savedAction, 0 ))
            continue;

        printf( "WARNING: failed to restore signal(%d) handler: error #%d\n", data->num, errno );
    }
}
