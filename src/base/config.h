#ifndef BASE_CONFIG_H
#define BASE_CONFIG_H

///////////////////////////////////////////////////////////////////////////////

#if defined( JAYMOD_LINUX )
#    include <base/linux/public.h>
#elif defined( JAYMOD_MINGW )
#    include <base/mingw/public.h>
#elif defined( JAYMOD_OSX )
#    include <base/osx/public.h>
#elif defined( JAYMOD_WINDOWS )
#    include <base/windows/public.h>
#else
#    error "JAYMOD platform is not defined."
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __i386__ )
#    define JAYMOD_LITTLE_ENDIAN
#elif  defined( __ppc__ )
#    define JAYMOD_BIG_ENDIAN
#else
#    define JAYMOD_LITTLE_ENDIAN
#endif

///////////////////////////////////////////////////////////////////////////////

#if defined( __GNUC__ )
#    define JAYMOD_FUNCTION __PRETTY_FUNCTION__
#elif defined( _MSC_VER )
#    define JAYMOD_FUNCTION __FUNCTION__
#else
#    define JAYMOD_FUNCTION __FUNCTION__
#endif

///////////////////////////////////////////////////////////////////////////////

#define USE_MDXFILE

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_CONFIG_H
