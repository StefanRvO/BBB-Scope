//Implements a ringbuffer on top of the array class
//You put things in the buffer with push_back(T) and get them off with pop_front()
#pragma once
#include <stdexcept>
#include <exception>
#include <array>
template <class T,std::size_t _size>
class RingBuffer 
{
    private:
        std::array<T,_size> Buffer;
        int HEAD;
        int TAIL;
        int ioTotal;
    public:
        RingBuffer()
        {
            HEAD=0;
            TAIL=0;
            ioTotal=0;
        }
        
        size_t size()
        {
            return ioTotal;
        }

        size_t capacity()
        {
            return Buffer.size();
        }

        bool empty()
        {
            return ioTotal==0;
        }

        bool full()
        {
            return ioTotal>=capacity();
        }

        void push_back(T element)
        {
            if (full()) 
            {
                TAIL=(TAIL+1)%capacity();
                ioTotal--;
            }
            Buffer[HEAD]=element;
            HEAD=(HEAD+1)%capacity();
            ioTotal++;
        }

        T pop_front()
        {
            if (empty()) throw(std::range_error("Buffer is empty"));
            T returnval=Buffer[TAIL];
            TAIL=(TAIL+1)%capacity();
            ioTotal--;
            return returnval;
        }
        void clear()
        {
            HEAD=0;
            TAIL=0;
            ioTotal=0;
        }
};

