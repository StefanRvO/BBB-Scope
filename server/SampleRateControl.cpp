#include "SampleRateControl.h"
SampleRateControl::SampleRateControl(double accuray_, HugeBuffer<sample,20000000> *samples_, SampleGrabber *SGrabber_,Options *options_)
{
    accuracy=accuray_;
    samples=samples_;
    SGrabber=SGrabber_;
    options=options_;
    t1=std::thread(SampleRateControlWrapper,this);
}
SampleRateControl::~SampleRateControl()
{
    stop=true;
    t1.join();
}

void SampleRateControl::run()
{
    long ticks=0;
    Timer t(5);
    while(!stop)
    {
        t.tick();
        if(samples->size()>5)
        {
            ticks++;
            long long diff=0;
            long indexpoint=5;
            long previndexpoint=5;
            while(diff<100000 and indexpoint<(int)samples->size())
            {
                diff=samples->at((long)samples->size()-1).time-samples->at((long)samples->size()-indexpoint-1).time;
                previndexpoint=indexpoint;
                indexpoint=indexpoint*1.3+5;
            }
            //cout << previndexpoint << "\t" << diff << endl;
            double rate=previndexpoint/(double) diff;
            rate*=1000000; //get Hz istead of Mhz
            Rates.push_back(rate);
        }
        /*if(ticks>10)
        {
            std::cout << Rates.getRealativeMaxDiff() << std::endl;
            if(options->connected and !options->adjusted and options->sampleMaxMin==0 and Rates.getRealativeMaxDiff()>accuracy)
            {
                if(SGrabber->RequestSlowerRate()) options->sampleMaxMin=0;
                else options->sampleMaxMin=-1;
            }
            ticks=0;
        }*/
    }
}
double SampleRateControl::getSampleRate()
{
    return Rates.getAvg();
}
void SampleRateControlWrapper(SampleRateControl *SRC)
{
    SRC->run();
}
