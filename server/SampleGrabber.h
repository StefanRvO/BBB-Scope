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
#define BACKLOG 10
#define BUFSIZE 8
class SampleGrabber
{
    private:
        int buffer[BUFSIZE];
        struct sockaddr_in server;
        struct sockaddr_in dest;
        int socket_fd, client_fd,num;
        socklen_t size;
        std::thread t1;
        timeval tv;
    
    public:
        RingBuffer<unsigned short,1000000> *sBuffer;
        RingBuffer<unsigned long long,1000000> *tBuffer;
        SampleGrabber(int port);
        ~SampleGrabber();
        void run();           
};
void sampleWrapper(SampleGrabber* SGrabber);
