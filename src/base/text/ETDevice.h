#ifndef BASE_TEXT_ETDEVICE_H
#define BASE_TEXT_ETDEVICE_H

///////////////////////////////////////////////////////////////////////////////

class ETDevice : public AbstractDevice
{
public:
    ETDevice  ( uint32 = 80, uint32 = 24, uint32 = 2 );
    ~ETDevice ( );

    char cnormal;
    char cbold;
    char cdim;
    char cpass;
    char cfail;
    char cwarning;

    char cdebug;
    char cdebugBOLD;
    char cdebugDIM;

    char cheader;
    char cheaderBOLD;
    char cheaderDIM;

    char cvalue;
    char cvalueBOLD;
    char cvalueDIM;

protected:
    void doFormat( const Buffer&, list<string>& );
};

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_TEXT_ETDEVICE_H
