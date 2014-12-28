#include "SampleGrabber.h"

SampleGrabber::SampleGrabber(int port)
{
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
    int pointeroffset=0;
    while(!stop)
    {
        size = sizeof(struct sockaddr_in);  
        if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1) 
        {
            perror("accept");
            exit(1);
        }
         printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));
        while(!stop) 
        {
            if ((num = read(client_fd, ((char *)&cursample)+pointeroffset, sizeof(cursample)-pointeroffset))== -1) 
            {
                perror("recv");
                exit(1);
            }   
            else if (num == 0) 
            {
                printf("Connection closed\n");
                break;
            }
            pointeroffset=(pointeroffset+num)%sizeof(cursample);
            if(pointeroffset==0)
            {
                cursample.value+=2048;
                sBuffer.push_back(cursample);
            }
        }
    }
}
SampleGrabber::~SampleGrabber()
{
    stop=true;
    close(client_fd);   
    close(socket_fd);   
}
void sampleWrapper(SampleGrabber* SGrabber)
{
    SGrabber->run();
}

