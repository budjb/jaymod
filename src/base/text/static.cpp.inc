#include <base/text/impl.h>

namespace text {

///////////////////////////////////////////////////////////////////////////////
//
// All manipulator codes must be unique and fall within range { 0x80..0xff }.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// Standard ET colors.
//
///////////////////////////////////////////////////////////////////////////////

const ColorManipulator xcblack    ( MANIP_COLOR_BLACK );     // 0
const ColorManipulator xcred      ( MANIP_COLOR_RED );       // 1
const ColorManipulator xcgreen    ( MANIP_COLOR_GREEN );     // 2
const ColorManipulator xcyellow   ( MANIP_COLOR_YELLOW );    // 3
const ColorManipulator xcblue     ( MANIP_COLOR_BLUE );      // 4
const ColorManipulator xccyan     ( MANIP_COLOR_CYAN );      // 5
const ColorManipulator xcmagenta  ( MANIP_COLOR_MAGENTA );   // 6
const ColorManipulator xcwhite    ( MANIP_COLOR_WHITE );     // 7
const ColorManipulator xcorange   ( MANIP_COLOR_ORANGE );    // 8
const ColorManipulator xcmdgrey   ( MANIP_COLOR_MDGREY );    // 9
const ColorManipulator xcltgrey   ( MANIP_COLOR_LTGREY );    // :
const ColorManipulator xcltgrey2  ( MANIP_COLOR_LTGREY2 );   // ;
const ColorManipulator xcmdgreen  ( MANIP_COLOR_MDGREEN );   // <
const ColorManipulator xcmdyellow ( MANIP_COLOR_MDYELLOW );  // =
const ColorManipulator xcmdblue   ( MANIP_COLOR_MDBLUE );    // >
const ColorManipulator xcmdred    ( MANIP_COLOR_MDRED );     // ?
const ColorManipulator xcmdbrown  ( MANIP_COLOR_MDBROWN );   // @
const ColorManipulator xcltorange ( MANIP_COLOR_LTORANGE );  // A
const ColorManipulator xcdkcyan   ( MANIP_COLOR_DKCYAN );    // B
const ColorManipulator xcmdpurple ( MANIP_COLOR_MDPURPLE );  // C
const ColorManipulator xcmdcyan   ( MANIP_COLOR_MDCYAN );    // D
const ColorManipulator xcpurple   ( MANIP_COLOR_PURPLE );    // E
const ColorManipulator xcbluegrey ( MANIP_COLOR_BLUEGREY );  // F
const ColorManipulator xcltolive  ( MANIP_COLOR_LTOLIVE );   // G
const ColorManipulator xcdkgreen  ( MANIP_COLOR_DKGREEN );   // H
const ColorManipulator xcltred    ( MANIP_COLOR_LTRED );     // I
const ColorManipulator xcrose     ( MANIP_COLOR_ROSE );      // J
const ColorManipulator xcdkorange ( MANIP_COLOR_DKORANGE );  // K
const ColorManipulator xcltbrown  ( MANIP_COLOR_LTBROWN );   // L
const ColorManipulator xcmdgold   ( MANIP_COLOR_MDGOLD );    // M
const ColorManipulator xcltgold   ( MANIP_COLOR_LTGOLD );    // N
const ColorManipulator xcgold     ( MANIP_COLOR_GOLD );      // O

///////////////////////////////////////////////////////////////////////////////
//
// Pseudo-colors.
//
///////////////////////////////////////////////////////////////////////////////

const ColorManipulator xcnone       ( MANIP_COLOR_NONE );
const ColorManipulator xcnormal     ( MANIP_COLOR_NORMAL );
const ColorManipulator xcbold       ( MANIP_COLOR_BOLD );
const ColorManipulator xcdim        ( MANIP_COLOR_DIM );
const ColorManipulator xcpass       ( MANIP_COLOR_PASS );
const ColorManipulator xcfail       ( MANIP_COLOR_FAIL );
const ColorManipulator xcwarning    ( MANIP_COLOR_WARNING );

const ColorManipulator xcdebug      ( MANIP_COLOR_DEBUG );
const ColorManipulator xcdebugBOLD  ( MANIP_COLOR_DEBUGBOLD );
const ColorManipulator xcdebugDIM   ( MANIP_COLOR_DEBUGDIM );

const ColorManipulator xcheader     ( MANIP_COLOR_HEADER );
const ColorManipulator xcheaderBOLD ( MANIP_COLOR_HEADERBOLD );
const ColorManipulator xcheaderDIM  ( MANIP_COLOR_HEADERDIM );

const ColorManipulator xcvalue      ( MANIP_COLOR_VALUE );
const ColorManipulator xcvalueBOLD  ( MANIP_COLOR_VALUEBOLD );
const ColorManipulator xcvalueDIM   ( MANIP_COLOR_VALUEDIM );

///////////////////////////////////////////////////////////////////////////////
//
// Various no-argument manipulators.
//
///////////////////////////////////////////////////////////////////////////////

const Manipulator xcpop      ( MANIP_CPOP );
const Manipulator xcpush     ( MANIP_CPUSH );
const Manipulator xlindent   ( MANIP_LINDENT );
const Manipulator xlunindent ( MANIP_LUNINDENT );
const Manipulator xnewline   ( MANIP_NEWLINE );
const Manipulator xrindent   ( MANIP_RINDENT );
const Manipulator xrunindent ( MANIP_RUNINDENT );

///////////////////////////////////////////////////////////////////////////////

InlineText xnone       ( xcnone );
InlineText xnormal     ( xcnormal );
InlineText xbold       ( xcbold );
InlineText xdim        ( xcdim );
InlineText xpass       ( xcpass );
InlineText xfail       ( xcfail );
InlineText xwarning    ( xcwarning );
InlineText xdebug      ( xcdebug );
InlineText xdebugBOLD  ( xcdebugBOLD );
InlineText xdebugDIM   ( xcdebugDIM );
InlineText xheader     ( xcheader );
InlineText xheaderBOLD ( xcheaderBOLD );
InlineText xheaderDIM  ( xcheaderDIM );
InlineText xvalue      ( xcvalue );
InlineText xvalueBOLD  ( xcvalueBOLD );
InlineText xvalueDIM   ( xcvalueDIM );

InlineVec3 xvec3;

///////////////////////////////////////////////////////////////////////////////

} // namespace text
