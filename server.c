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
#include<allegro.h>
#include <sys/time.h>
#include <math.h>
#include <thread>
#include "server.h"
#include <stdexcept>
#include <segvcatch.h>
//#include <iostream>
#define threading

#define PORT 3490
#define BACKLOG 10
#define BUFSIZE 2
#define ZOOMMIN 0.01
#define ZOOMMAX 10000

const int SCREENSIZE_x=1300;
const int SCREENSIZE_y=700;
volatile int ticks = 0;
BITMAP *bitbuffer;
int buffer[BUFSIZE];
int count = 0;
int* samples = NULL;
int* more_samples = NULL;
long sampletime=0;
long sampletime_tmp=0;
void ticker()
{
    ticks++;
}
END_OF_FUNCTION(ticker)
int updates_per_second = 100000;
timeval time_1;
timeval time_2;


void handle_segv()
{
    //throw std::runtime_error("My SEGV");
    printf("SEGV catched");
}

void handle_fpe()
{
    //throw std::runtime_error("My FPE");
        printf("fpe catched");
}


void Init()
{
    allegro_init();
    install_keyboard();
    install_timer();
    LOCK_VARIABLE(ticks);
    LOCK_FUNCTION(ticker);
    install_int_ex(ticker, BPS_TO_TIMER(updates_per_second));
    set_gfx_mode( GFX_AUTODETECT_WINDOWED, SCREENSIZE_x, SCREENSIZE_y, 0, 0);
    request_refresh_rate(1000);
}


void DrawScreen()
{
//Clear Screen
printf("test");
    int curcount=0;
    int keyval=0;
    float ZOOMFACTOR=1;
    int Y_off=0;  //Y offset
    float Y_Zoom=1;
    float Y_adjust=0;
    while (1){
        if (key[KEY_ESC]) {
            exit(0);
            }
        curcount=count;
        clear_to_color( bitbuffer, makecol( 0, 0, 0));
        
        //Draw something
        try {
             textprintf_centre_ex(bitbuffer, font, SCREENSIZE_x-90, 15, makecol(255,255,255), -1,"Last sample: %.3f V",((float) samples[curcount-1])*(1.8/4096.));
             textprintf_ex(bitbuffer, font, 0, 15, makecol(255,255,255), -1,"ZOOM: %f",ZOOMFACTOR);
             textprintf_ex(bitbuffer, font, 0, 30, makecol(255,255,255), -1,"ZOOMY: %f",Y_Zoom);
             textprintf_ex(bitbuffer, font, 0, 45, makecol(255,255,255), -1,"Y_off: %d",Y_off);
             textprintf_ex(bitbuffer, font, 0, SCREEN_H-30, makecol(255,255,255), -1,"sample frequency: %.1f Hz",(float) (1./(float)(sampletime/40000.))*1000000);
             for (int a=0;a<=SCREEN_W;a++) {
                fastline(bitbuffer,SCREEN_W-a,(SCREEN_H-(samples[curcount-(int) (a*ZOOMFACTOR)-1])*SCREENSIZE_y/4096-(Y_off-Y_adjust))*Y_Zoom,SCREENSIZE_x-a-1,(SCREEN_H-(samples[curcount-(int)((a-1)*ZOOMFACTOR)-1])*SCREENSIZE_y/4096-(Y_off-Y_adjust))*Y_Zoom,makecol( 0, 255, 0));
         }
         
         
         
         
        }
        catch(std::exception& e) {
            printf("segfault catched\n");
            continue;
        }
        vsync();
        blit(bitbuffer, screen, 0, 0, 0, 0, SCREENSIZE_x, SCREENSIZE_y);
        while(keypressed()) {
            keyval=readkey();
            //Fast Zoom
            if((keyval & 0xff) =='+') {
                if (ZOOMFACTOR*SCREEN_W<(curcount*(1/1.001))) {
                    ZOOMFACTOR*=1.1;
                    if(ZOOMFACTOR>ZOOMMAX) {
                        ZOOMFACTOR=ZOOMMAX;
                    }
                }
            }
            else if((keyval & 0xff) =='-') {
                if (ZOOMFACTOR>ZOOMMIN) {
                    ZOOMFACTOR*=(1/1.1);
                    if (ZOOMFACTOR<ZOOMMIN) {
                       ZOOMFACTOR=ZOOMMIN;
                    }
                }
            }
            //Slow Zoom
            else if((keyval & 0xff) =='p') {
                if (ZOOMFACTOR*SCREEN_W<(curcount*(1/1.01))) {
                    ZOOMFACTOR*=1.001;
                    if(ZOOMFACTOR>ZOOMMAX) {
                        ZOOMFACTOR=ZOOMMAX;
                    }
                }
            }
            else if((keyval & 0xff) =='m') {
                if (ZOOMFACTOR>ZOOMMIN) {
                    ZOOMFACTOR*=(1/1.001);
                    if (ZOOMFACTOR<ZOOMMIN) {
                       ZOOMFACTOR=ZOOMMIN;
                    }
                }
            }
            else if( (keyval>>8)==KEY_SPACE) {
                ZOOMFACTOR=1;
                }
            else if( (keyval>>8)==KEY_UP) {
                Y_off++;
                }
            else if( (keyval>>8)==KEY_DOWN) {
                Y_off--;
                }
            else if( (keyval>>8)==KEY_STOP) {
                Y_Zoom*=1.01;
                Y_adjust=-(SCREEN_H)*(1./Y_Zoom-1)/2.;
                }
            else if( (keyval>>8)==KEY_COMMA) {
                Y_Zoom*=(1/1.01);
                Y_adjust=-(SCREEN_H)*(1./Y_Zoom-1)/2.;
                }
        }
    }
}



int main()
{
    struct sockaddr_in server;
    struct sockaddr_in dest;
    int status,socket_fd, client_fd,num;
    socklen_t size;

    int buffer[BUFSIZE];
    //  memset(buffer,0,sizeof(buffer));
    int yes =1;
    int i=0;
    

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0))== -1) {
        fprintf(stderr, "Socket failure!!\n");
        exit(1);
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
    memset(&server, 0, sizeof(server));
    memset(&dest,0,sizeof(dest));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY; 
    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr )))== -1)    { //sizeof(struct sockaddr) 
        fprintf(stderr, "Binding Failure\n");
        exit(1);
    }

    if ((listen(socket_fd, BACKLOG))== -1){
        fprintf(stderr, "Listening Failure\n");
        exit(1);
    }

    while(1) {
        size = sizeof(struct sockaddr_in);  

        if ((client_fd = accept(socket_fd, (struct sockaddr *)&dest, &size))==-1) {
            //fprintf(stderr,"Accept Failure\n");
            perror("accept");
            exit(1);
        }
        printf("Server got connection from client %s\n", inet_ntoa(dest.sin_addr));
        Init();
        bitbuffer=create_bitmap(SCREENSIZE_x, SCREENSIZE_y);
        segvcatch::init_segv();
        segvcatch::init_fpe();
        std::thread t1(DrawScreen);
        //buffer = "Hello World!! I am networking!!\n";

        while(1) {
            if ((num = recv(client_fd, buffer, BUFSIZE,0))== -1) {
                //fprintf(stderr,"Error in receiving message!!\n");
                perror("recv");
                exit(1);
            }   
            else if (num == 0) {
                printf("Connection closed\n");
                return 0;
            }
        //  num = recv(client_fd, buffer, sizeof(buffer),0);
            //buffer[num] = '\1';
            //printf("Message received: %d\n", *buffer);
            //if(i++%10000==0){
            //printf("%d\n",*buffer);
            //}
            count++;
            more_samples = (int*) realloc (samples, count * sizeof(int));
            if (more_samples!=NULL) {
            samples=more_samples;
            samples[count-1]=*buffer;
            
            }
            else {
            free (samples);
            puts ("Error (re)allocating memory");
            exit (1);
            }
            if(count%40000==0) {
                gettimeofday(&time_2,0);
                sampletime_tmp=time_2.tv_usec-time_1.tv_usec;
                if (sampletime_tmp<0) {
                    sampletime_tmp+=1000000;
                }
                sampletime=sampletime_tmp;
                gettimeofday(&time_1,0);
            }
           // if(count%10000==0) {
           // printf("%d\n",count);
           // }
        }

    /*  buff = "I am communicating with the client!!\n";

        if ((send(client_fd,buff, strlen(buff),0))== -1) {
        fprintf(stderr, "Failure Sending Message\n");
        close(client_fd);
        exit(1);
    }
    else {
        printf("Message being sent: %s\nNumber of bytes sent: %d\n",buff, strlen(buff));
    }*/

        close(client_fd);   
        close(socket_fd);   
        //return 0;
    }
    //close(client_fd); 
    return 0;
}
