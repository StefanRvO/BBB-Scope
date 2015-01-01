#pragma once
#include <thread>
#include "HugeBuffer.h"
#include "SampleGrabber.h"
#include "Timer.h"
#include "structures.h"
class SampleRateControl
{
    private:
        double accuracy;
        bool stop=false;
        SampleGrabber *SGrabber;
        Options *options;
        HugeBuffer<sample,20000000> *samples;
        std::thread t1;
        RingBuffer<double,5> Rates;
    public:
        SampleRateControl(double accuray_, HugeBuffer<sample,20000000>  *samples, SampleGrabber *SGrabber_,Options *options);
        ~SampleRateControl();
        void run();
        double getSampleRate();

};
void SampleRateControlWrapper(SampleRateControl *SRC);
