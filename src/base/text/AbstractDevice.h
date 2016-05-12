#ifndef BASE_TEXT_ABSTRACTDEVICE_H
#define BASE_TEXT_ABSTRACTDEVICE_H

///////////////////////////////////////////////////////////////////////////////

class Buffer;

///////////////////////////////////////////////////////////////////////////////

class AbstractDevice
{
public:
    virtual ~AbstractDevice();

    list<string>::size_type format( const Buffer&, list<string>& );

    uint32 cols;
    uint32 rows;
    uint32 indent;

protected:
    AbstractDevice( uint32, uint32, uint32 );

    virtual void doFormat( const Buffer&, list<string>& ) = 0;

private:
    AbstractDevice(); // not permitted
};

///////////////////////////////////////////////////////////////////////////////

#include <base/text/ETDevice.h>
#include <base/text/TerminalDevice.h>

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_TEXT_ABSTRACTDEVICE_H
