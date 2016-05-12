#include <game/g_local.h>

#include <omnibot/common/BotExports.h>
#include <dlfcn.h>

extern bool   g_IsOmnibotLoaded;
extern string g_OmnibotLibPath;

///////////////////////////////////////////////////////////////////////////////

namespace {
    const char* const __LIB_SUFFIX = ".so";
    const char* const __OMNI_DIR   = "omni-bot";
    const char* const __OMNI_LOG   = "OMNIBOT:";
    const char        __PATHSEP    = '/';

    set<string>  __dirSet;
    list<string> __dirList;
    void*        __handle = NULL;

    void addDirectory( string dir ) {
        // skip if already present
        if (!__dirSet.insert( dir ).second)
            return;

        __dirList.push_back( dir );
    }
}

///////////////////////////////////////////////////////////////////////////////

eomnibot_error
Omnibot_LoadLibrary( const int version, const char* const libbase, const char* const customdir )
{
    G_Printf( "%s loader version 0.66\n", __OMNI_LOG );

    const string libname = string( libbase ) + __LIB_SUFFIX;

    // prepare list of path names
    set<string>   dirSet;
    list<string*> dirList;

    // add custom dir
    if (customdir && customdir[0])
        addDirectory( customdir );

    // add fs_homepath based dir
    {
        char buffer[1024];
        trap_Cvar_VariableStringBuffer( "fs_homepath", buffer, sizeof(buffer) );
        if (*buffer) {
            ostringstream path;
            path << buffer << __PATHSEP << __OMNI_DIR;
            addDirectory( path.str() );
        }
    }

    // add fs_basepath based dir
    {
        char buffer[1024];
        trap_Cvar_VariableStringBuffer( "fs_basepath", buffer, sizeof(buffer) );
        if (*buffer) {
            ostringstream path;
            path << buffer << __PATHSEP << __OMNI_DIR;
            addDirectory( path.str() );
        }
    }

    // add HOME based dir
    {
        const char* home = getenv( "HOME" );
        if (home && *home) {
            ostringstream path;
            path << home << __PATHSEP << __OMNI_DIR;
            addDirectory( path.str() );
        }
    }

    // add special zero-length string for final try
    addDirectory( "" );

    // print search path
    G_Printf( "%s search path:\n", __OMNI_LOG );

    const list<string>::iterator end = __dirList.end();
    for ( list<string>::iterator it = __dirList.begin(); it != end; it++ ) {
        const string& dir = *it;

        if (dir.length())
            G_Printf( "    %s\n", dir.c_str() );
        else
            G_Printf( "    <SYSTEM-LOADER>\n" );
    }

    // attempt loading
    for ( list<string>::iterator it = __dirList.begin(); it != end; it++ ) {
        const string& dir = *it;

        string name = dir;
        if (name.length() && name[ name.length()-1 ] != __PATHSEP)
            name += __PATHSEP;

        name += libname;

        __handle = dlopen( name.c_str(), RTLD_NOW );
        if (!__handle) {
            G_Printf( "%s load '%s': failure: %s\n", __OMNI_LOG, name.c_str(), dlerror() );
            continue;
        }

        // success
        G_Printf( "%s load '%s': success\n", __OMNI_LOG, name.c_str() );

        // update buffer indicating bot path
        g_OmnibotLibPath = name;
        break;
    }

    if (!__handle)
        return BOT_ERROR_CANTLOADDLL;

    // check proper dll and initialize it
    pfnGetFunctionsFromDLL pfnGetBotFuncs = 0;
    memset( &g_BotFunctions, 0, sizeof(g_BotFunctions) );

    pfnGetBotFuncs = (pfnGetFunctionsFromDLL)dlsym( __handle, "ExportBotFunctionsFromDLL" );
    G_Printf( "%s address-lookup: %s\n", __OMNI_LOG, pfnGetBotFuncs ? "success" : "failure" );
    if (!pfnGetBotFuncs) {
        G_Printf( "%s dlsym failed: %s\n", __OMNI_LOG, dlerror() );
        return BOT_ERROR_CANTGETBOTFUNCTIONS;
    }

    eomnibot_error oe = pfnGetBotFuncs( &g_BotFunctions, sizeof(g_BotFunctions) );
    G_Printf( "%s pfnGetBotFuncs: %s\n", __OMNI_LOG, oe == BOT_ERROR_NONE ? "success" : "failure" );
    if (oe != BOT_ERROR_NONE)
        return oe;

    oe = g_BotFunctions.pfnInitialize( g_InterfaceFunctions, version );
    g_IsOmnibotLoaded = (oe == BOT_ERROR_NONE);

    G_Printf( "%s initialization: %s\n", __OMNI_LOG, g_IsOmnibotLoaded ? "success" : "failure" );
    return oe;
}

///////////////////////////////////////////////////////////////////////////////

void
Omnibot_FreeLibrary()
{
    if (__handle) {
    	dlclose( __handle );
	    __handle = NULL;
	}

    memset( &g_BotFunctions, 0, sizeof(g_BotFunctions) );

    delete g_InterfaceFunctions;
    g_InterfaceFunctions = 0;

    g_IsOmnibotLoaded = false;
}
