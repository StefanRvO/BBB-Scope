#include "SampleGrabber.h"

SampleGrabber::SampleGrabber(int sPort,int cPort)
{
    if ((socket_serv_samples = socket(AF_INET, SOCK_STREAM, 0))== -1) 
    {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }
    if ((socket_serv_control = socket(AF_INET, SOCK_STREAM, 0))== -1) 
    {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }
    int yes=1;
    if (setsockopt(socket_serv_samples, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
    {
        perror("setsockopt");
        exit(1);
    }
    if (setsockopt(socket_serv_control, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
    {
        perror("setsockopt");
        exit(1);
    }
    memset(&server_samples, 0, sizeof(server_samples));
    memset(&dest_samples,0,sizeof(dest_samples));
    memset(&server_control, 0, sizeof(server_control));
    memset(&dest_control,0,sizeof(dest_control));
    server_samples.sin_family = AF_INET;
    server_control.sin_family = AF_INET;
    server_samples.sin_port = htons(sPort);
    server_control.sin_port = htons(cPort);
    server_samples.sin_addr.s_addr = INADDR_ANY; 
    server_control.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_serv_samples, (struct sockaddr *)&server_samples, sizeof(struct sockaddr )))== -1)    
    {
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }
    if ((bind(socket_serv_control, (struct sockaddr *)&server_control, sizeof(struct sockaddr )))== -1)    
    {
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }

    if ((listen(socket_serv_control, BACKLOG))== -1)
    {
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }
    if ((listen(socket_serv_samples, BACKLOG))== -1)
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
        if ((socket_cli_samples = accept(socket_serv_samples, (struct sockaddr *)&dest_samples, &size))==-1) 
        {
            perror("accept");
            exit(1);
        }
        if ((socket_cli_control = accept(socket_serv_control, (struct sockaddr *)&dest_control, &size))==-1) 
        {
            perror("accept");
            exit(1);
        }
        printf("Server got connection from client %s\n", inet_ntoa(dest_samples.sin_addr));
        while(!stop) 
        {
            if ((num = read(socket_cli_samples, ((char *)&cursample)+pointeroffset, sizeof(cursample)-pointeroffset))== -1) 
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
bool SampleGrabber::RequestFastRate()
{
    int8_t cont=1;
    if ((write(socket_cli_control,&cont, sizeof(cont))== -1)) {
        printf( "Failure Sending Message\n");
        close(socket_serv_control);
        close(socket_serv_control);
        exit(1);
    }
    if ( read(socket_cli_control, (char *)&cont, sizeof(cont) )== -1) 
    {
        perror("recv");
        exit(1);
    }
    return cont;   
        
}
bool SampleGrabber::RequestSlowerRate()
{
    int8_t cont=-1;
    if ((write(socket_cli_control,&cont, sizeof(cont))== -1)) {
        printf( "Failure Sending Message\n");
        close(socket_serv_control);
        close(socket_serv_control);
        exit(1);
    }
    if ( read(socket_cli_control, (char *)&cont, sizeof(cont) )== -1) 
    {
        perror("recv");
        exit(1);
    }
    return cont;   
}
SampleGrabber::~SampleGrabber()
{
    stop=true;
    close(socket_cli_control);
    close(socket_cli_samples);
    close(socket_serv_control); 
    close(socket_serv_samples);
}
void sampleWrapper(SampleGrabber* SGrabber)
{
    SGrabber->run();
}


