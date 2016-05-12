#include <bgame/impl.h>

namespace {

///////////////////////////////////////////////////////////////////////////////

class __BufferEngine : public stringbuf
{
private:
    const string _prefix;

protected:
    int
    sync()
    {
        trap_Print( (_prefix + str()).c_str() );
        str( "" );
        return 0;
    }

public:
    __BufferEngine( const string& prefix_ )
        : stringbuf()
        , _prefix( prefix_ + ": " )
    {
    }

    ~__BufferEngine()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////

class __BufferStream : public stringbuf
{
private:
    const string _prefix;
    ostream&     _out;

protected:
    int
    sync()
    {
        _out << _prefix << str() << flush;
        str( "" );
        return 0;
    }

public:
    __BufferStream( const string& prefix_, ostream& out_ )
        : stringbuf ( )
        , _prefix   ( prefix_ + ": " )
        , _out      ( out_ )
    {
    }

    ~__BufferStream()
    {
    }
};

///////////////////////////////////////////////////////////////////////////////

} // namespace anonymous

///////////////////////////////////////////////////////////////////////////////

Logger::Logger( const string& prefix_ )
    : ostream( new __BufferEngine( prefix_ ))
{
}

///////////////////////////////////////////////////////////////////////////////

Logger::Logger( const string& prefix_, ostream& out_ )
    : ostream( new __BufferStream( prefix_, out_ ))
{
}

///////////////////////////////////////////////////////////////////////////////

Logger::~Logger()
{
    delete rdbuf();
}
