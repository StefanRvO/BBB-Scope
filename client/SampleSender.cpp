#include "SampleSender.h"
#include <iostream>
SampleSender::SampleSender(ADCOptions *options_, RingBuffer<sample,1000000> *RB_,int sPORT, int cPORT, hostent *he, ADC *Adc_)
{
    options=options_;
    RB=RB_;
    Adc=Adc_;
    //Setup Network
    if ((socket_samples = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket Failure!!\n");
        stop=true;
    }
    if ((socket_control = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket Failure!!\n");
        stop=true;
    }

    memset(&server_samples, 0, sizeof(server_samples));
    memset(&server_control, 0, sizeof(server_control));
    server_samples.sin_family = AF_INET;
    server_samples.sin_port = htons(sPORT);
    server_samples.sin_addr = *((struct in_addr *)he->h_addr);
    server_control.sin_family = AF_INET;
    server_control.sin_port = htons(cPORT);
    server_control.sin_addr = *((struct in_addr *)he->h_addr);
    if (connect(socket_samples, (struct sockaddr *)&server_samples, sizeof(struct sockaddr))<0) {
        perror("connect");
        stop=true;
    }
    if (connect(socket_control, (struct sockaddr *)&server_control, sizeof(struct sockaddr))<0) {
        perror("connect");
        stop=true;
    }
    if(!stop) t1=std::thread(controlSocketThreadWrapper,this);
}
SampleSender::~SampleSender()
{
    stop=true;
    t1.join();
    close(socket_control);
    close(socket_samples);
}
bool SampleSender::changeSampleTime(uint64_t time)
{
    if(time<options->sampleTimeMax and time>options->sampleTimeMin)
    {
        options->sampleTime=time;
        Adc->resetSampler();
        return true;
    }
    return false;
}
void SampleSender::controlSocketThread()
{
    controlMessage control;
    if (write(socket_control,(char *)&control, sizeof(controlMessage))== -1) {
        printf( "Failure Sending Message\n");
        stop=true;
        return;
    }
    int size;
    int pointer=0;
    while(!stop)
    {
        if ((size=read(socket_control, ((char*)&control)+pointer, sizeof(controlMessage)-pointer ))== -1) 
        {
            perror("recv");
            stop=true;
            return;
        }
        pointer+=size;
        if(pointer!=sizeof(controlMessage)) continue;
        pointer=0;
        uint64_t newtime=options->sampleTime-control.changespeed;
        control.changespeed=changeSampleTime(newtime)*control.changespeed;
        control.time=options->sampleTime;
        
        if ((write(socket_control,(char *)&control, sizeof(controlMessage))== -1)) {
            printf( "Failure Sending Message\n");
            stop=true;
            return;
        }
    }
}
void SampleSender::sampleSocketThread()
{
    Timer t(100);
    sample cursample;
    while(!stop)
    {
        while(!RB->empty())
        {
            cursample=RB->pop_front();
            if (write(socket_samples,&cursample, sizeof(cursample))== -1) 
            {
                printf("Failure Sending Message\n");
                stop=true;
                return;
            }
        }
        t.tick();
    }
}
void controlSocketThreadWrapper(SampleSender *SS)
{
    SS->controlSocketThread();
}
