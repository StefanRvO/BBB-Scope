#include "ADC.h"
ADC::ADC(ADCOptions *options_, RingBuffer<sample,1000000> *RB_)
{
    options=options_;
    RB=RB_;
    io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0); //! create new driver structure
    pruio_adc_setStep(io, 1, 1, 0, 0, 0); // step 1 for AIN-1

    if (pruio_config(io, 100000, 1 << 1, options->sampleTime, 0)) //step 1, 6290ns/sample -> 158,98 KHz
    {
        printf("config failed (%s)\n", io->Errr);
    }
    else
    {
        pruio_rb_start(io);
        sampleIndex=io->DRam[0];
        lastDRam0=io->DRam[0];
        t1=std::thread(SampleThreadWrapper,this);
    }
    
}
ADC::~ADC()
{
  stop=true;
  t1.join();
  pruio_destroy(io);
}

void ADC::sampleThread(pruIo *io,RingBuffer<sample,1000000> *RB)
{   //Grabs samples from PRU ringbuffer and puts it into global ringbuffer
    sample cursample;
    while(!sampleStop and !stop)
    {
        {
            sampleIndex++;
            if(sampleIndex>=100000)
            {
                sampleIndex=0; //make sure index don't overflow
            }
            cursample.value=io->Adc->Value[sampleIndex];
            RB->push_back(cursample);
            lastDRam0=io->DRam[0];
        }
        while(sampleIndex!=lastDRam0);
        usleep(sampletime/200); //wait at least 5 samples
    }
}
ADC::resetSampler()
{
    sampleStop=true;
    t1.join();
    if (pruio_config(io, 100000, 1 << 1, options->sampletime, 0)) //step 1, 6290ns/sample -> 158,98 KHz
    {
        printf("config failed (%s)\n", io->Errr);
        
    }
    else
    {
        pruio_rb_start(io);
        sampleIndex=io->DRam[0];
        lastDRam0=io->DRam[0];
        t1=std::thread(SampleThreadWrapper,this);
    }
}
void SampleThreadWrapper(ADC *Adc)
{
    Adc->SampleThread();
}
