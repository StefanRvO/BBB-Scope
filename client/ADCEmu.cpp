#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../server/structures.h"
#include "../server/Timer.h"
#define sPORT 3490
#define cPORT 3491

void controlThread();
void senderThread();
int samplelock=0;
struct sockaddr_in server_samples,server_control;
struct hostent *he;
int socket_samples,socket_control,num;
long sampletime=6290;
int minTime=6290;
int maxTime=1000000;
Timer t(1000000000./sampletime);
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
    std::thread t2(senderThread);
    std::thread t3(controlThread);
    while(true)
    {
        usleep(100000);
    }
  close(socket_control);
  close(socket_samples);
  return 0;
}
void senderThread()
{
    sample cursample;
    int size;
    double v=0;
    while(true)
    {
        v+=0.0001;
        cursample.value=sin(v)*2048+2048;
        if ((size=write(socket_samples,&cursample, sizeof(cursample))== -1)) 
        {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            exit(1);
        }
        t.tick();
    }
}
void controlThread()
{
    controlMessage control;
    control.time=sampletime;
    int pointer=0;
    int size;
    if ((write(socket_control,(char *)&control, sizeof(controlMessage))== -1)) {
        printf( "Failure Sending Message\n");
        close(socket_control);
        close(socket_samples);
        exit(1);
    }
    while(true)
    {
        if ( (size=read(socket_control, ((char*)&control)+pointer, sizeof(controlMessage)-pointer ))== -1) 
        {
            perror("recv");
            close(socket_control);
            close(socket_samples);
            exit(1);
        }
        pointer+=size;
        if(pointer!=sizeof(controlMessage)) continue;
        pointer=0;
        if(control.changespeed<=-1)
        {
            if(sampletime+5*-control.changespeed<maxTime)
            {
                printf("speeddown\n");
                sampletime+=5*-control.changespeed;
                control.changespeed=-1;
             }
             else control.changespeed=0;
        }
        else if(control.changespeed>=1) 
        {
            if(sampletime+5*-control.changespeed>minTime)
            {
                printf("speedup\n");
                sampletime+=5*-control.changespeed;
                control.changespeed=1;
            }
            else control.changespeed=0;
            
        }
        if(control.changespeed)
        {
            t.setFPS(1000000000./sampletime);
            printf("%ld\n",sampletime);
        }
        control.time=sampletime;
        if ((write(socket_control,(char *)&control, sizeof(controlMessage))== -1)) {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            exit(1);
        }
    }
}  
