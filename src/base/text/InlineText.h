#ifndef BASE_TEXT_INLINETEXT_H
#define BASE_TEXT_INLINETEXT_H

///////////////////////////////////////////////////////////////////////////////

class InlineText
{
private:
    void compute( const string& );

    ostringstream _oss;
    string        _value;

public:
    InlineText  ( );
    InlineText  ( const InlineText& );
    InlineText  ( const ColorManipulator& );
    InlineText  ( bool );
    InlineText  ( float );
    InlineText  ( double );
    InlineText  ( int8 );
    InlineText  ( int16 );
    InlineText  ( int32 );
    InlineText  ( int64 );
    InlineText  ( uint8 );
    InlineText  ( uint16 );
    InlineText  ( uint32 );
    InlineText  ( uint64 );
    InlineText  ( const void* );
    InlineText  ( const char* );
    InlineText  ( const string& );
#if defined( JAYMOD_OSX )
    InlineText  ( size_t );
#endif // JAYMOD_OSX
    ~InlineText ( );

    InlineText& operator=( const InlineText& );

    InlineText& operator() ( bool );
    InlineText& operator() ( float );
    InlineText& operator() ( double );
    InlineText& operator() ( int8 );
    InlineText& operator() ( int16 );
    InlineText& operator() ( int32 );
    InlineText& operator() ( int64 );
    InlineText& operator() ( uint8 );
    InlineText& operator() ( uint16 );
    InlineText& operator() ( uint32 );
    InlineText& operator() ( uint64 );
    InlineText& operator() ( const void* );
    InlineText& operator() ( const char* );
    InlineText& operator() ( const string& );
#if defined( JAYMOD_OSX )
    InlineText& operator() ( size_t );
#endif // JAYMOD_OSX

    ColorManipulator color;
    ios::fmtflags    flags;
    streamsize       precision;
    streamsize       width;
    string           prefix;
    string           prefixOutside;
    string           suffix;
    string           suffixOutside;

    const string&    value;
};

///////////////////////////////////////////////////////////////////////////////

class InlineVec3 : public InlineText
{
public:
    InlineVec3 ( );
    InlineVec3 ( const InlineVec3& );
    InlineVec3 ( const float* );

    ~InlineVec3();

    InlineVec3& operator()( const float* );
};

///////////////////////////////////////////////////////////////////////////////

extern InlineText xnone;
extern InlineText xnormal;
extern InlineText xbold;
extern InlineText xdim;
extern InlineText xpass;
extern InlineText xfail;
extern InlineText xwarning;
extern InlineText xdebug;
extern InlineText xdebugBOLD;
extern InlineText xdebugDIM;
extern InlineText xheader;
extern InlineText xheaderBOLD;
extern InlineText xheaderDIM;
extern InlineText xvalue;
extern InlineText xvalueBOLD;
extern InlineText xvalueDIM;

extern InlineVec3 xvec3;

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_TEXT_INLINETEXT_H
