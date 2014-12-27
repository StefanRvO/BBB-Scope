#include "HugeBuffer.h"
template <class T,size_t _size>
HugeBuffer<T,_size>::HugeBuffer(std::string filename_)
{
    filebuff =fopen(filename_.c_str(),"wb+"); //open file
    //start memory handler thread
    memoryHandlerThread=std::thread(memoryHandlerWrapper<T,_size>,this);
    filename=filename_;
}
template <class T,size_t _size>
HugeBuffer<T,_size>::HugeBuffer()
{
    timeval tv;
    gettimeofday(&tv,NULL);
    srand(tv.tv_sec^tv.tv_usec);
    filename=std::to_string(rand());
    filebuff =fopen(filename.c_str(),"wb+"); //open file
    //start memory handler thread
    memoryHandlerThread=std::thread(memoryHandlerWrapper<T,_size>,this);
}

template <class T,size_t _size>
HugeBuffer<T,_size>::~HugeBuffer()
{
    //stop and join memory handler thread
    stop=true;
    tmpMemMtx.unlock(); //unlock mutexes to free up potential locks
    fileAccess.unlock();
    fileinAccess.unlock();
    fileoutAccess.unlock();
    memoryHandlerThread.join();
    fclose(filebuff);
    remove(filename.c_str()); //remove file
    for(TempAlloc<T> &block : tmpAllocks) delete[] block.memblock; //delete tmpallocated stuff
}

template <class T,size_t _size>
size_t HugeBuffer<T,_size>::size()
{
    fileAccess.lock(); //this may be problematic
    size_t tsize=rBuffer.size()+(filepointer+1);
    fileAccess.unlock();
    return tsize;
}

template <class T,size_t _size>
void HugeBuffer<T,_size>::push_back(T element)
{
    rBuffer.push_back(element);
}

template <class T,size_t _size>
T HugeBuffer<T,_size>::at(long index)
{
    //Test if element is in loaded buffer
    if(index<0) throw(std::range_error("negative index not allowed!"));
    //else if(index>size()) throw(std::range_error("out of bounds!"));
    fileAccess.lock(); //this may be problematic
    if(index-filepointer>0)
    {
        T tmp=rBuffer.at(index-(filepointer+1));
        fileAccess.unlock(); //this may be problematic
        return tmp;
    }
    else //not in primary buffer. Grab from bufferpool
    {
        fileAccess.unlock();
        return bufferPool(index);
    }
    
}

template <class T,size_t _size>
T HugeBuffer<T,_size>::bufferPool(long index)
{
    //check if index really are in pool
    if(filepointer<index) throw(std::range_error("indexerror in Hugebuffer"));
    
    //search tmpallocated memory for index
    while(true)
    {
        tmpMemMtx.lock();
        for(TempAlloc<T> &block : tmpAllocks)
        {
            if(not (block.startindex<=index and block.endindex>=index)) continue;
            //we got the right block
            gettimeofday(&block.lastAccess,NULL);
            T temp=block.memblock[index-block.startindex];
            tmpMemMtx.unlock();
            return temp;
        }
        tmpMemMtx.unlock();
        //the sample is not in the memory pool :(
        loadBlock(index);
    } 
}

template <class T,size_t _size>
void HugeBuffer<T,_size>::loadBlock(long index) //load block into memory pool
{
    long startindex=index-_size/20;
    long endindex=index+_size/20;
    fileAccess.lock();
    if(endindex>filepointer) endindex=filepointer;
    fileAccess.unlock();
    if (startindex<0) startindex=0;
    //Make sure we don't doubleload anything
    tmpMemMtx.lock();
    for(TempAlloc<T> &block : tmpAllocks)
    {
        //if endindex is in allocated block, make this block end just before that
        //if startindex is in allocated block, make this block start just after that
        if (endindex<=block.endindex and endindex>=block.startindex) endindex=block.startindex-1;
        if (startindex<=block.endindex and startindex>=block.startindex) startindex=block.endindex+1;
    }
    tmpMemMtx.unlock();
    if(startindex>endindex) return; //the block is already allocated
    
    fileAccess.lock();
    if(endindex>filepointer) endindex=filepointer;
    fileAccess.unlock();
    if (startindex<0) startindex=0;
    
    TempAlloc<T> block;
    //allocate memory
    block.size=endindex-startindex+1;
    block.memblock=new T[block.size];
    block.startindex=startindex;
    block.endindex=endindex;
    
    //load data from file and insert in pool
    fileinAccess.lock();
    fseek(filebuff,startindex*sizeof(T),SEEK_SET);
    fread(block.memblock,sizeof(T),block.size,filebuff);
    fileinAccess.unlock();
    //std:: cout << block.memblock[0] << " " <<block.memblock[1] << std::endl;
    //std:: cout << block.startindex*sizeof(T) << " " << block.size*sizeof(T) << " " << block.endindex*sizeof(T) <<  std::endl;
    tmpMemMtx.lock();
    gettimeofday(&block.lastAccess,NULL);
    tmpAllocks.insert(tmpAllocks.begin(),block);
    tmpMemMtx.unlock();
} 

 
template <class T,size_t _size>  
void HugeBuffer<T,_size>::memoryHandler()    
{   //search through memory pool and remove timed out segments
    //move data from ringbuffer to file if it's close to be filled
    Timer t(HBUFFER_CLEANRATE);
    timeval tv;
    while(!stop)
    {
        t.tick();
        //search for timed out blocks
        tmpMemMtx.lock();
        gettimeofday(&tv,NULL);
        for(int i=0; i<tmpAllocks.size();i++)
        {
            if((long long)tv.tv_sec*1000000+tv.tv_usec-HBUFFER_TOUT>(long long)tmpAllocks[i].lastAccess.tv_sec*1000000+tmpAllocks[i].lastAccess.tv_usec)
            {
                delete[] tmpAllocks[i].memblock;
                tmpAllocks.erase(tmpAllocks.begin()+i);
                i--;
            }
        }
        tmpMemMtx.unlock();
        //check if buffer is within 20% of full. if so, fill into file. But do it just rBuffercapacity/1000 peices a time to prevent locking
        if((float)rBuffer.size()/rBuffer.capacity()>0.8)
        {
            while((float)rBuffer.size()/rBuffer.capacity()>0.7)
            {
                //allocate memory
                T *mem=new T[rBuffer.capacity()/1000];
                //Copy data 
                for(int i=0;i<rBuffer.capacity()/1000; i++)
                {
                    mem[i]=rBuffer.at(i);
                }
                //write to file
                fileoutAccess.lock();
                fileinAccess.lock();
                fseek(filebuff,0,SEEK_END);
                fwrite(mem,sizeof(T),rBuffer.capacity()/1000,filebuff);
                fileinAccess.unlock();
                fileoutAccess.unlock();
                fileAccess.lock();
                filepointer+=rBuffer.capacity()/1000;
                rBuffer.eraseElements(rBuffer.capacity()/1000);
                fileAccess.unlock();
                delete[] mem;
            }
        }
    }
}  

template <class T,size_t _size>  
void memoryHandlerWrapper(HugeBuffer<T,_size> *HB)
{
    HB->memoryHandler();
}
