#ifndef BASE_TEXT_TERMINALDEVICE_H
#define BASE_TEXT_TERMINALDEVICE_H

///////////////////////////////////////////////////////////////////////////////

class TerminalDevice : public AbstractDevice
{
public:
    TerminalDevice  ( uint32 = 160, uint32 = 24, uint32 = 2 );
    ~TerminalDevice ( );

protected:
    void doFormat( const Buffer&, list<string>& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_TEXT_TERMINALDEVICE_H
