#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <thread>
#include "RingBuffer.h"
#include "structures.h"
#define BACKLOG 10000
class SampleGrabber
{
    private:
        sample cursample;
        struct sockaddr_in server_samples;
        struct sockaddr_in dest_samples;
        struct sockaddr_in server_control;
        struct sockaddr_in dest_control;
        int socket_serv_samples,socket_serv_control, socket_cli_samples,socket_cli_control,num;
        socklen_t size;
        std::thread t1;
        bool stop=false;
  
    public:
        RingBuffer<sample,10000000> sBuffer;
        SampleGrabber(int sPort,int cPort);
        ~SampleGrabber();
        void run();    
        bool RequestFastRate();
        bool RequestSlowerRate();
    
};
void sampleWrapper(SampleGrabber* SGrabber);
