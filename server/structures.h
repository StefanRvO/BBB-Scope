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
    int8_t viewMode=0;
    uint64_t sampletime=0;
    bool locked=0;
    bool autoSampleRate=true;
};
struct sample
{
    int16_t value; //samplevalue
};
struct FFTOps
{
    long start;
    long end; //0 means whole range (to N/2)
    long size;
    long totalsize;
    std::complex<double> *mem;
};
struct controlMessage
{
    int16_t changespeed=0;   //should speed be faster or slower?
    uint64_t time=0;        //time between samples is changed/should be changed to this number of ns
};
