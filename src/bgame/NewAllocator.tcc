#ifndef GAME_NEWALLOCATOR_TCC
#define GAME_NEWALLOCATOR_TCC

///////////////////////////////////////////////////////////////////////////////

template <typename C>
void
NewAllocator<C>::operator delete( void* p, size_t size )
{
    if (!p)
        return;

    _heapUsed -= size;
    delete[] static_cast<char*>(p);
}

///////////////////////////////////////////////////////////////////////////////

template <typename C>
void*
NewAllocator<C>::operator new( size_t size )
{
    _heapUsed += size;
    if (_heapHighWatermark < _heapUsed)
        _heapHighWatermark = _heapUsed;

    return new char[size];
}

///////////////////////////////////////////////////////////////////////////////

template <typename C>
size_t NewAllocator<C>::_heapUsed;

template <typename C>
size_t NewAllocator<C>::_heapHighWatermark;

template <typename C>
const size_t& NewAllocator<C>::heapUsed = NewAllocator<C>::_heapUsed;

template <typename C>
const size_t& NewAllocator<C>::heapHighWatermark = NewAllocator<C>::_heapHighWatermark;

///////////////////////////////////////////////////////////////////////////////

#endif // GAME_NEWALLOCATOR_TCC
