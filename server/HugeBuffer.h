/*Buffer to store huge amounts off data, where parts of it will not be used very often
  Holds a certain amount in memory (in a ringbuffer), but when this buffer comes within 20% of being filled,
  some data (till the buffer is 30% filled) will be pushed to HDD.
  If data outside of what is currently in the buffer is accessed, a memory area of 5% the size of the ringbuffer will be read into memory
  And stay there for 1 minute. The timeout will reset if something in that group is accessed again.
  The temporary allocated groups are kept in a sorted list together with their startindex(included) and endindex(included) and last time accessed. It's sorted after most recent access.
  a thread searches through the list every second and deallocates stuff which are timed out.
*/
#pragma once
#include <stdexcept>
#include <exception>
#include <cstdlib>
#include <sys/time.h>
#include <mutex>
#include <vector>
#include <thread>
#include <fstream>
#include<string>
#include<cstdio>
#include <iostream>
#include <algorithm>
#include "RingBuffer.h"
#include "Timer.h"
#define HBUFFER_TOUT 60000000 //timeout in µs for memory blocks
#define HBUFFER_CLEANRATE 1 //times per sec to cleanup of memorypool and ringbuffer
template <class T>
struct TempAlloc //struct to hold info about allocated memory blocks
{
    timeval lastAccess;
    long startindex;
    long endindex;
    long size;
    T *memblock;
};
template <class T,std::size_t _size>
class HugeBuffer
{
    private:
        std::string filename;
        std::vector<TempAlloc<T> > tmpAllocks;
        RingBuffer<T,_size> rBuffer;
        long filepointer=-1; //holds the index for the last sample in the HDD file.
        std::mutex tmpMemMtx; //mutex for the temporary loaded data
        std::mutex fileinAccess; //mutex for filein access
        std::mutex fileoutAccess; //mutex for fileout access
        std::mutex fileAccess; //mutex for general file access
        bool stop=false;
        T *tmpMem;
        FILE *filebuff;
        std::thread memoryHandlerThread;
        void loadBlock(long index);
        T bufferPool(long index);
        static bool TempAllocLarge( TempAlloc<T> a,TempAlloc<T> b);
    public:
        HugeBuffer( std::string filename);
        HugeBuffer();
        ~HugeBuffer();
        void push_back(T element);
        size_t size(); //get elementcount in buffer
        long tmpAllockSize();
        //This is where the magic happens
        T at(long index); //get element at index
        void memoryHandler();

};

template<typename T, size_t _size>
void memoryHandlerWrapper(HugeBuffer<T,_size> *HB);
#include "HugeBuffer.tpp"
