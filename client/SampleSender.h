#include <thread>
#include <cstdint>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../server/RingBuffer.h"
#include "../server/structures.h"
#include "../server/Timer.h"
#include "ADCOptions.h"
#include "ADC.h"
//#define sPORT 3490
//#define cPORT 3491
class SampleSender
{
    private:
        sockaddr_in server_samples,server_control;
        hostent *he;
        int socket_samples,socket_control,num;
        bool changeSampleTime(uint64_t time);
        RingBuffer<sample,1000000> *RB
        ADCOptions *options;
        std::thread t1;
        bool stop=0;
        ADC *Adc;
    public:
        SampleSender(ADCOptions *options_, RingBuffer<sample,1000000> *RB_,int sPORT, int cPORT, hostent *he, ADC *Adc_);
        ~SampleSender();
        void controlSocketThread();
        void sampleSocketThread();
};
void controlSocketThreadWrapper(SampleSender *SS);
