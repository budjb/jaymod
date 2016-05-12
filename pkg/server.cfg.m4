changequote(<<, >>)dnl
include(<<project.m4>>)dnl
dnl
dnl
dnl
///////////////////////////////////////////////////////////////////////////////
//
// __title (r<<>>__repoLCRev)
//
//
// SAMPLE CONFIGURATION FILE.
//
// If you have any questions regarding a specific cvar, check the included
// documentation.
//
// __copyright
// __website
// __irc
//
//
///////////////////////////////////////////////////////////////////////////////

// PUNKBUSTER
pb_sv_enable

// JAYMOD
exec jaymod.cfg

// SIMPLE REPEATING MAP SETUP
set nextmap "map fueldump"
set com_watchdog_cmd "map fueldump"
vstr com_watchdog_cmd
