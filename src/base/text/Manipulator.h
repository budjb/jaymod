#ifndef BASE_TEXT_MANIPULATOR_H
#define BASE_TEXT_MANIPULATOR_H

///////////////////////////////////////////////////////////////////////////////

class Manipulator
{
private:
    Manipulator();  // not permitted

public:
    Manipulator( uint8 );
    virtual ~Manipulator();

    uint8 code;
};

///////////////////////////////////////////////////////////////////////////////

class ColorManipulator : public Manipulator
{
private:
    ColorManipulator();  // not permitted

public:
    ColorManipulator( uint8 );
    ~ColorManipulator();

    ColorManipulator& operator=( const ColorManipulator& );
};

///////////////////////////////////////////////////////////////////////////////
//
// Standard ET colors.
//
///////////////////////////////////////////////////////////////////////////////

extern const ColorManipulator xcblack;     // 0
extern const ColorManipulator xcred;       // 1
extern const ColorManipulator xcgreen;     // 2
extern const ColorManipulator xcyellow;    // 3
extern const ColorManipulator xcblue;      // 4
extern const ColorManipulator xccyan;      // 5
extern const ColorManipulator xcmagenta;   // 6
extern const ColorManipulator xcwhite;     // 7
extern const ColorManipulator xcorange;    // 8
extern const ColorManipulator xcmdgrey;    // 9
extern const ColorManipulator xcltgrey;    // :
extern const ColorManipulator xcltgrey2;   // ;
extern const ColorManipulator xcmdgreen;   // <
extern const ColorManipulator xcmdyellow;  // =
extern const ColorManipulator xcmdblue;    // >
extern const ColorManipulator xcmdred;     // ?
extern const ColorManipulator xcmdbrown;   // @
extern const ColorManipulator xcltorange;  // A
extern const ColorManipulator xcmdcyan;    // B
extern const ColorManipulator xcmdpurple;  // C
extern const ColorManipulator xcmdcyan;    // D
extern const ColorManipulator xcpurple;    // E
extern const ColorManipulator xcbluegrey;  // F
extern const ColorManipulator xcltolive;   // G
extern const ColorManipulator xcdkgreen;   // H
extern const ColorManipulator xcltred;     // I
extern const ColorManipulator xcrose;      // J
extern const ColorManipulator xcdkorange;  // K
extern const ColorManipulator xcltbrown;   // L
extern const ColorManipulator xcmdgold;    // M
extern const ColorManipulator xcltgold;    // N
extern const ColorManipulator xcgold;      // O

///////////////////////////////////////////////////////////////////////////////
// 
// Pseudo-colors.
//
///////////////////////////////////////////////////////////////////////////////

extern const ColorManipulator xcnone;        // placebo
extern const ColorManipulator xcnormal;      // set color for normal body text
extern const ColorManipulator xcbold;        // set color for bold/standout
extern const ColorManipulator xcdim;         // set color for dim/dark
extern const ColorManipulator xcpass;        // set color for pass/good/ok
extern const ColorManipulator xcfail;        // set color for fail/error
extern const ColorManipulator xcwarning;     // set color for warnings

extern const ColorManipulator xcdebug;       // set color for debug
extern const ColorManipulator xcdebugBOLD;
extern const ColorManipulator xcdebugDIM;

extern const ColorManipulator xcheader;      // set color for headers/titles
extern const ColorManipulator xcheaderBOLD;
extern const ColorManipulator xcheaderDIM;

extern const ColorManipulator xcvalue;       // set color for values (eg. name/value pair)
extern const ColorManipulator xcvalueBOLD;
extern const ColorManipulator xcvalueDIM;

///////////////////////////////////////////////////////////////////////////////
//
// Various no-argument manipulators.
//
///////////////////////////////////////////////////////////////////////////////

extern const Manipulator xcpop;       // pop color off stack
extern const Manipulator xcpush;      // push color on stack
extern const Manipulator xlindent;    // push left-indent on stack, standard amount
extern const Manipulator xlunindent;  // pop left-indent off stack
extern const Manipulator xnewline;    // end table
extern const Manipulator xrindent;    // push right-indent on stack, standard amount
extern const Manipulator xrunindent;  // pop right-indent off stack

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_TEXT_MANIPULATOR_H
