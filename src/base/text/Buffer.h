#ifndef BASE_TEXT_BUFFER_H
#define BASE_TEXT_BUFFER_H

///////////////////////////////////////////////////////////////////////////////

class Buffer
{
private:
    void append( uint8 );
    void append( const string& );

    uint8* _data;
    uint32 _length;
    uint32 _numLines;
    uint32 _size;

public:
    Buffer ( const ColorManipulator& = xcnormal );
    Buffer ( const uint8*, uint32, const ColorManipulator& = xcnormal );

    ~Buffer();

    Buffer& operator<< ( const Buffer& );
    Buffer& operator<< ( const InlineText );
    Buffer& operator<< ( const Manipulator& );
    Buffer& operator<< ( char );
    Buffer& operator<< ( int );
    Buffer& operator<< ( const char* );
    Buffer& operator<< ( string );

    void dump();
    void reset( const ColorManipulator& = xcnormal );

    void toCommands( const string&, list<string>&, int = 1022 ) const;
    bool toReport( vector<Buffer*>&, uint32, uint32 ) const;

    uint8* const& data;
    const uint32& length;
    const uint32& numLines;
};

///////////////////////////////////////////////////////////////////////////////

#endif // BASE_TEXT_BUFFER_H
