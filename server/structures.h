#pragma once
#include <cstdlib> 
#include <sys/time.h>
struct Options
{
    bool alive=true;
    double zoomX=1.;
    double zoomY=1.;
    double offsetY=0;
    double offsetX=0;
    int mouseX=0;
    int mouseY=0;
    bool paused=false;
    size_t pausedSamplesize;
    int lockmode=4;
};
struct sample
{
    short value;
    timeval tv;
};
