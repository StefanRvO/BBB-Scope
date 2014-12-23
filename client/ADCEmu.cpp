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
#include "../server/structures.h"
#include <ctime>
#define SAMPLESIZE 1
#define LOOPS 1000000
#define PORT 3490
/* ----------------------------------------------------------- */

int main(int argc, char *argv[])
{
    srand(time(NULL));
    struct sockaddr_in server_info;
    struct hostent *he;
    int socket_fd,num;
    sample cursample;
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
        cursample.value=(sin(v)+sin(v*0.3)+sin(v*1.2))*2048/3+(rand()%1000-500);
        v+=0.01*M_PI;
        /*if(v-(int)v>0.7)  cursample.value=1000+rand()%20;
        else cursample.value=-1000+rand()%20;*/
        gettimeofday(&cursample.tv,NULL);
        //usleep(1000);
        if (write(socket_fd,&cursample, sizeof(cursample))== -1) {
            printf( "Failure Sending Message\n");
            close(socket_fd);
            exit(1);
        }
    }
    close(socket_fd);
    return 0;
}  
    

