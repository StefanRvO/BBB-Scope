#pragma once
#include <cstdlib> 
#include <sys/time.h>
#include <complex>
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
    int lockmode=2;
    int8_t sampleMaxMin=0;
    bool connected=0;
    bool adjusted=0;
    u_int8_t viewMode=0;
};
struct sample
{
    double value;
    int64_t time; //time in ms
};
struct FFTOps
{
    long start;
    long end; //0 means whole range (to N/2)
    long size;
    long totalsize;
    std::complex<double> *mem;
};
