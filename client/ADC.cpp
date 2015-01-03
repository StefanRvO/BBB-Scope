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
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../server/Timer.h"
#include <../server/Ringbuffer.h>
#include <../server/structures.h>

#define sPORT 3490
#define cPORT 3491

void controlThread(int &socket_control);
void SenderThread(RingBuffer<sample,1000000> *RB, int &socket_samples );
void SampleThread(pruIo *io,RingBuffer<sample,1000000> *RB);


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
    pruIo *io = pruio_new(PRUIO_DEF_ACTIVE, 0, 0, 0); //! create new driver structure
    pruio_adc_setStep(io, 1, 8, 0, 0, 0); // step 1 for AIN-8

    if (pruio_config(io, 100000, 1 << 1, 6290, 0)) //step 1, 6290ns/sample -> 158,98 KHz
    {
        printf("config failed (%s)\n", io->Errr);}
    else
    {   
        //Create Ringbuffer
        RingBuffer<sample,1000000> RB;
        //start sampling
        pruio_rb_start(io);
        std::thread t1(sampleThread,&RB);
        std::thread t2(SenderThread,&RB,&socket_samples);
        std::thread t2(ControlThread,&RB,&socket_control);
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
void SampleThread(pruIo *io,RingBuffer<sample,1000000> *RB)
{   //Grabs samples from PRU ringbuffer and puts it into global ringbuffer
    Timer t(1000);
    sample cursample;
    int index=0;
    int wrapped=0
    int lastDRam0;
    while(true)
    {
        do()
        {
            if(io->DRam[0] < lastDRam0)
            {
                wrapped++;
            }
            if(index==100000)
            {
                index=0; //make sure index don't overflow
                wrapped--;
            }
            cursample.value=io->Adc->Value[index++];
            gettimeofday(&cursample.time,NULL);
            RB->push_back(cursample);
            lastDRam0=io->DRam[0];
        }
        while(index%100000<io->DRam[0] or wrapped)
        t.tick();
    }
}
void SenderThread(RingBuffer<sample,1000000> *RB, int &socket_samples )
{
    Timer t(1000);
    while(true)
    {
        while(RB->size())
        {
            auto cursample=RB->pop_front();
            if ((size=write(socket_samples,&cursample, sizeof(cursample))== -1)) 
            {
                printf( "Failure Sending Message\n");
                close(socket_control);
                close(socket_samples);
                exit(1);
            }
        }
        t.tick();
    }
}
void controlThread(int &socket_control)
{
    int speed=5;
    int8_t cont;
    while(true)
    {
        if(read(socket_control,&cont,sizeof(cont))==-1)
        {
            perror("recv");
            exit(1);
        }
        if(cont==-1)
        {
            if(speed<50)
            {
                cout << "speedup" << endl;
                speed++;
                cont=1;
             }
             else cont=0;
             
        }
        else if(cont==1) 
        {
            if(speed>2)
            {
                cout << "speeddown" << endl;
                speed--;
                cont=1;
            }
            else cont=0;
        }
        if (write(socket_control,&cont, 1)== -1) {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            exit(1);
        }
    }
}  