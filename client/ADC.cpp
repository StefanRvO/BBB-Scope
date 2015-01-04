#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <pruio.h>
#include <thread>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../server/Timer.h"
#include "../server/RingBuffer.h"
#include "../server/structures.h"

#define sPORT 3490
#define cPORT 3491

void controlThread();
void senderThread(RingBuffer<sample,1000000> *RB);
void sampleThread(pruIo *io,RingBuffer<sample,1000000> *RB);
pruIo *io;
int samplelock=0;
struct sockaddr_in server_samples,server_control;
struct hostent *he;
int socket_samples,socket_control,num;
long sampletime=6290;
int minTime=6290;
int maxTime=1000000;
int main(int argc, char **argv)
{
    //Setup network
    if (argc != 2) {
        fprintf(stderr, "Usage: client hostname\n");
        exit(1);
    }

    if ((he = gethostbyname(argv[1]))==NULL) {
        fprintf(stderr, "Cannot get host name\n");
        exit(1);
    }

    if ((socket_samples = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket Failure!!\n");
        exit(1);
    }
    if ((socket_control = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket Failure!!\n");
        exit(1);
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
        //fprintf(stderr, "Connection Failure\n");
        perror("connect");
        exit(1);
    }
    if (connect(socket_control, (struct sockaddr *)&server_control, sizeof(struct sockaddr))<0) {
        //fprintf(stderr, "Connection Failure\n");
        perror("connect");
        exit(1);
    }
    
    //Setup ADC
    int minTime=6290;
    int maxTime=1000000;
    io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0); //! create new driver structure
    pruio_adc_setStep(io, 1, 1, 0, 0, 0); // step 1 for AIN-1

    if (pruio_config(io, 100000, 1 << 1, sampletime, 0)) //step 1, 6290ns/sample -> 158,98 KHz
    {
        printf("config failed (%s)\n", io->Errr);
    }
    else
    {   
        //Create Ringbuffer
        RingBuffer<sample,1000000> RB;
        //start sampling
        pruio_rb_start(io);
        std::thread t1(sampleThread,io,&RB);
        std::thread t2(senderThread,&RB);
        std::thread t3(controlThread);
        while(true)
        {
            usleep(100000);
        }
  }
  pruio_destroy(io);
  close(socket_control);
  close(socket_samples);
  return 0;
}
void sampleThread(pruIo *io,RingBuffer<sample,1000000> *RB)
{   //Grabs samples from PRU ringbuffer and puts it into global ringbuffer
    sample cursample;
    int index=0;
    int wrapped=0;
    int lastDRam0;
    timeval tv;
    while(true)
    {
        while(samplelock) usleep(100);
        lastDRam0=io->DRam[0];
        do
        {
            if(samplelock)
            {
                index=0;
                wrapped=0;
                break;
            }
            index++;
            if(index>=100000)
            {
                index=0; //make sure index don't overflow
            }
            cursample.value=io->Adc->Value[index];
            RB->push_back(cursample);
            lastDRam0=io->DRam[0];
        }
        while(index!=lastDRam0);
        usleep(sampletime/200); //wait at least 5 samples
    }
}
void senderThread(RingBuffer<sample,1000000> *RB)
{
    Timer t(100);
    sample cursample;
    while(true)
    {
        while(!RB->empty())
        {
            cursample=RB->pop_front();
            if (write(socket_samples,&cursample, sizeof(cursample))== -1) 
            {
                printf( "Failure Sending Message\n");
                close(socket_control);
                close(socket_samples);
                pruio_destroy(io);
                exit(1);
            }
        }
        t.tick();
    }
}
void controlThread()
{
    controlMessage control;
    control.time=sampletime;
    if (write(socket_control,&control, sizeof(control))== -1) {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            pruio_destroy(io);
            exit(1);
    }
    while(true)
    {
        if(read(socket_control,&control,sizeof(controlMessage))==-1)
        {
            perror("recv");
            close(socket_control);
            close(socket_samples);
            pruio_destroy(io);
            exit(1);
        }
        if(control.changespeed<=-1)
        {
            if(sampletime>minTime)
            {
                printf("speeddown");
                sampletime+=5*-control.changespeed;
                control.changespeed=-1;
             }
             else control.changespeed=0;
        }
        else if(control.changespeed<=-1) 
        {
            if(sampletime<maxTime)
            {
                printf("speedup");
                sampletime+=5*-control.changespeed;
                control.changespeed=1;
            }
            else control.changespeed=0;
            
        }
        if(control.changespeed)
        {
            samplelock=1;
            if (pruio_config(io, 100000, 1 << 1, sampletime, 0)) //step 1, 6290ns/sample -> 158,98 KHz
            {
                printf("config failed (%s)\n", io->Errr);
            }
            else
            {
                pruio_rb_start(io);
            }
            samplelock=0;
        }
        control.time=sampletime;
        if (write(socket_control,&control, sizeof(control))== -1) {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            pruio_destroy(io);
            exit(1);
        }
    }
}  
