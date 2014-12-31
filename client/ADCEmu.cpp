//Emulates a sine curve and send through network.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cmath>
#include <thread>
#include <fcntl.h>
#include "../server/structures.h"
#include "../server/Timer.h"
#include <ctime>
#include <iostream>
#define SAMPLESIZE 1
#define LOOPS 1000000
#define sPORT 3490
#define cPORT 3491
#define BACKLOG 100
/* ----------------------------------------------------------- */
using namespace std;

void control();

int speed=25;
Timer *t;
struct sockaddr_in server_samples,server_control;
struct hostent *he;
int socket_samples,socket_control,num;
int main(int argc, char *argv[])
{
    timeval tv;
    t=new Timer(10000*speed);
    srand(time(NULL));
    sample cursample;
	int i=0 ,j;
	int buffer_AIN_0[SAMPLESIZE] ={0};

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
    std::thread t1(control);
    double v=1;
    int size;
    while(1) {
        gettimeofday(&tv,NULL);
        cursample.time=tv.tv_sec*1000000+tv.tv_usec;
        v+=0.001*speed;
        cursample.value=sin(v)*2048;
        if(v>67) v=1;
        if ((size=write(socket_samples,&cursample, sizeof(cursample))== -1)) {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            exit(1);
        }
    }
    close(socket_control);
    close(socket_samples);
    return 0;
    delete t;
}  
void control()
{
    int8_t cont;
    while(true)
    {
        if(read(socket_control,&cont,sizeof(cont))==-1)
        {
            perror("recv");
            exit(1);
        }
        if(cont==1)
        {
            if(speed<50)
            {
                cout << "speedup" << endl;
                speed++;
                cont=1;
             }
             else cont=0;
             
        }
        else if(cont==-1) 
        {
            if(speed>2)
            {
                cout << "speeddown" << endl;
                speed--;
                cont=1;
            }
            else cont=0;
        }
        t->setFPS(10000*speed);
        cout << speed << endl;
        if (write(socket_control,&cont, 1)== -1) {
            printf( "Failure Sending Message\n");
            close(socket_control);
            close(socket_samples);
            exit(1);
        }
    }
}  

