#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#define BACKLOG 10
#define BUFSIZE 8
#define PORT 3490
bool recvstopped=true;

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int status,socket_fd, client_fd,num;
    socklen_t size;

    int buffer[BUFSIZE];

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
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    
    {
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1){
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }

    while(1) 
    {
        size = sizeof(struct sockaddr_in);  
        if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1) 
        {
            perror("accept");
            exit(1);
        }
        printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));
        while(true) {
        if ((num = recv(client_fd, buffer, BUFSIZE,0))== -1) 
        {
            perror("recv");
            exit(1);
        }   
        else if (num == 0) 
        {
            printf("Connection closed\n");
            return 0;
        }
        std::cout << *buffer << std::endl;  }           
    }
    close(client_fd);   
    close(socket_fd);   
    return 0;
}
