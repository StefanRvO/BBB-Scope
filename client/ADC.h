#include <cstdio>
#include <pruio.h>
#include <thread>
#include "../server/Timer.h"
#include "../server/RingBuffer.h"
#include "../server/structures.h"
#include "ADCOptions.h"
class ADC
{
    private:
        pruIo *io;
        bool sampleLock=false;
        ADCOptions *options;
        RingBuffer<sample,1000000> *RB;
        bool stop=false;
        bool sampleStop=false;
        std::thread t1;
        int sampleIndex=0;
    public:
        ADC(ADCOptions *options_, RingBuffer<sample,1000000> *RB_);
        ~ADC();
        void resetSampler();
        void SampleThread();
};
void SampleThreadWrapper(ADC *Adc);
