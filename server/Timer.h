#pragma once
#include <unistd.h>
class Timer
{
    private: 
    timeval last;
    timeval slast;
    int framerate;
    public:
    Timer(int fps)
    {
        framerate=fps;
        gettimeofday(&last,NULL);
        gettimeofday(&slast,NULL);
    }
    void tick()
    {
        timeval tv;
        gettimeofday(&tv,NULL);
        long long goal=last.tv_sec*1000000+last.tv_usec+(1000000./framerate);
        long long cur=tv.tv_sec*1000000+tv.tv_usec;
        if(goal>cur)
        {
            long long diff=goal-cur;
            usleep(diff);
        }
        slast=last;
        last=tv;
    }
    void presisionTick() //use more cpu while waiting
    {
        timeval tv;
        gettimeofday(&tv,NULL);
        long long goal=last.tv_sec*1000000+last.tv_usec+(1000000./framerate);
        long long cur=tv.tv_sec*1000000+tv.tv_usec;
        while(goal>cur)
        {
            usleep((goal-cur)/10);
            gettimeofday(&tv,NULL);
            cur=tv.tv_sec*1000000+tv.tv_usec;
        }
        slast=last;
        last=tv;
    }
    
    void highPresisionTick() //use 100% cpu while waiting
    {
        timeval tv;
        gettimeofday(&tv,NULL);
        long long goal=last.tv_sec*1000000+last.tv_usec+(1000000./framerate);
        long long cur=tv.tv_sec*1000000+tv.tv_usec;
        while(goal>cur)
        {
            gettimeofday(&tv,NULL);
            cur=tv.tv_sec*1000000+tv.tv_usec;
        }
        slast=last;
        last=tv;
    }
    void setFPS(int fps)
    {
        framerate=fps;
    }
    int getFPS()
    {
        long long lastT=last.tv_sec*1000000+last.tv_usec;
        long long slastT=slast.tv_sec*1000000+slast.tv_usec;
        long diff=lastT-slastT;
        return (int)((float)(1000000./diff)+0.5);
    }

};
