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
#include "../server/Timer.h"
#include <ctime>
#define SAMPLESIZE 1
#define LOOPS 1000000
#define PORT 3490
/* ----------------------------------------------------------- */

int main(int argc, char *argv[])
{
    Timer t(100);
    srand(time(NULL));
    struct sockaddr_in server_info;
    struct hostent *he;
    int socket_fd,num;
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
    int size;
    while(1) {
        gettimeofday(&cursample.tv,NULL);
        //fgets(buffer,MAXSIZE-1,stdin);
        //cursample.value=v*2048+rand()%200-100;
        //cursample.value=(sin(v)+sin(v*2)+sin(v*3))*2048/3+rand()%200-100;
        //if(v>1) cursample.value=(2-v)*2048+rand()%200-100;
        cursample.value=sin((cursample.tv.tv_sec*1000.+cursample.tv.tv_usec/1000.)*0.1)*1500;
        t.highPresisionTick();
        //else cursample.value=(sin(v)+sin(v*2)+sin(v*3))*1500/3-1000;
        //else
        //cursample.value=v*2048+rand()%200-100;
        //if(v>2) v=0;
        if (size=write(socket_fd,&cursample, sizeof(cursample))== -1) {
            printf( "Failure Sending Message\n");
            close(socket_fd);
            exit(1);
        }
    }
    close(socket_fd);
    return 0;
}  
    

