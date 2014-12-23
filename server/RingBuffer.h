//Implements a ringbuffer on top of the array class
//You put things in the buffer with push_back(T) and get them off with pop_front()
#pragma once
#include <stdexcept>
#include <exception>
#include <array>
#include <mutex>
template <class T,std::size_t _size>
class RingBuffer 
{
    private:
        std::array<T,_size> *Buffer;
        size_t cap;
        long long HEAD;
        long long TAIL;
        long long ioTotal;
        std::mutex lock;
    public:
        RingBuffer()
        {
            Buffer=new std::array<T,_size>;
            HEAD=0;
            TAIL=0;
            ioTotal=0;
            cap=_size;
        }
        
        size_t size()
        {
            return ioTotal;
        }

        size_t capacity()
        {
            return cap;
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
                lock.lock();
                *(Buffer->data()+HEAD)=element;
                HEAD=(HEAD+1)%capacity();
                TAIL=(TAIL+1)%capacity();
                lock.unlock();
                return;
            }
            lock.lock();
            *(Buffer->data()+HEAD)=element;
            HEAD=(HEAD+1)%capacity();
            ioTotal++;
            lock.unlock();
        }

        T pop_front()
        {
            if (empty()) throw(std::range_error("Buffer is empty"));
            lock.lock();
            T returnval=*(Buffer->data()+TAIL);
            TAIL=(TAIL+1)%capacity();
            ioTotal--;
            lock.unlock();
            return returnval;
        }
        void clear()
        {
            lock.lock();
            HEAD=0;
            TAIL=0;
            ioTotal=0;
            lock.unlock();
        }
};

