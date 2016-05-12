#ifndef BGAME_NEWALLOCATOR_H
#define BGAME_NEWALLOCATOR_H

///////////////////////////////////////////////////////////////////////////////

template <typename T>
class NewAllocator
{
private:
    static size_t _heapUsed;
    static size_t _heapHighWatermark;

public:
    static void  operator delete ( void*, size_t );
    static void* operator new    ( size_t );

    static const size_t& heapUsed;
    static const size_t& heapHighWatermark;
};

///////////////////////////////////////////////////////////////////////////////

#endif // BGAME_NEWALLOCATOR_H
