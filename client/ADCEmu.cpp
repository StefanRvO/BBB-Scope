//Emulates a sine curve and send through network.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cmath>
#define SAMPLESIZE 1
#define LOOPS 1000000
#define PORT 3490
#define MAXSIZE 8
/* ----------------------------------------------------------- */

int main(int argc, char *argv[])
{
    struct sockaddr_in server_info;
    struct hostent *he;
    int socket_fd,num;
    int buffer[MAXSIZE];
    int sample;
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

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket Failure!!\n");
        exit(1);
    }

    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(PORT);
    server_info.sin_addr = *((struct in_addr *)he->h_addr);
    if (connect(socket_fd, (struct sockaddr *)&server_info, sizeof(struct sockaddr))<0) {
        //fprintf(stderr, "Connection Failure\n");
        perror("connect");
        exit(1);
    }
    double v=0;
    while(1) {
        //fgets(buffer,MAXSIZE-1,stdin);
        buffer[0]=(int)(sin(v)*2048)+2048;
        v+=0.1;
        usleep(10000);
        if ((send(socket_fd,buffer, (sizeof(buffer)/sizeof(*buffer)),0))== -1) {
            printf( "Failure Sending Message\n");
            close(socket_fd);
            exit(1);
        }
    }
    close(socket_fd);
    return 0;
}  
    

