#include "SampleGrabber.h"

SampleGrabber::SampleGrabber(int port)
{
    sBuffer=new RingBuffer<unsigned short,1000000>;
    tBuffer=new RingBuffer<unsigned long long,1000000>;
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) 
    {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }
    int yes=1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
    {
        perror("setsockopt");
        exit(1);
    }
    memset(&server, 0, sizeof(server));
    memset(&dest,0,sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    
    {
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1)
    {
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
    t1=std::thread(sampleWrapper,this); 
}
void SampleGrabber::run()
{
    while(true)
    {
        size = sizeof(struct sockaddr_in);  
        if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1) 
        {
            perror("accept");
            exit(1);
        }
         printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));
        while(true) 
        {
            gettimeofday(&tv,NULL);
            if ((num = recv(client_fd, buffer, BUFSIZE,0))== -1) 
            {
                perror("recv");
                exit(1);
            }   
            else if (num == 0) 
            {
                printf("Connection closed\n");
                break;
            }
            tBuffer->push_back(tv.tv_sec*1000000+tv.tv_usec);
            sBuffer->push_back(*buffer);
        }
        
    }
}
SampleGrabber::~SampleGrabber()
{
    close(client_fd);   
    close(socket_fd);   
}
void sampleWrapper(SampleGrabber* SGrabber)
{
    SGrabber->run();
}

