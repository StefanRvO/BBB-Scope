#pragma once
#include "structures.h"
#include <complex.h>
#include <fftw3.h>
#include <SDL2/SDL.h>
#include <thread>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <vector>
#include <mutex>
#include "RingBuffer.h"
#include "HugeBuffer.h"
#include "Timer.h"
#include "SampleGrabber.h"
/*
This is my attempt of detection the frequency of the signal.
It uses autocorrelation to find the frequency.
The autocorrelation is performed by a fft and a inverse fft
This is probably too slow to run on each frame update, so it just runs independetly.

*/
#define AVGSIZE 5
#define UPDATERATE 10 //How many times a second the periodelenght is calculated.
#define FSAMPLESIZEACC 5000 //accuracy of findsamplesize
class PeriodFinder {

    private:
        fftw_plan forward;
        fftw_plan backward;
        Options *options;
        SampleGrabber *SGrabber;
        HugeBuffer<sample,20000000> *samples;
        long windowFFTSize;
        long windowFFTFirst;
        long windowFFTLast;
        double *final;
        std::complex<double> *out;
        std::complex<double> *con;
        std::complex<double> *windowFFT;
        void fastAutocorrelate();
        void calcSize();
        void calcPlacement();
        long size=0;
        SDL_Window *window;
        long placement;
        std::thread t1;
        int periode;
        RingBuffer<int,AVGSIZE> runningAvgBuf;
        Timer t;
        float avgperiode;
        int FindBestLockMode(long samplesize, int periode);
        void renewPlans(); //make new plans (eg. if the display has been resized)
        void findPeriode();
        bool stop=false;
        std::mutex fftlock;
    public:
        PeriodFinder(Options *options_, HugeBuffer<sample,20000000> *samples_, SDL_Window *window_,SampleGrabber *SGrabber_,FFTOps fftOps);
        int getPeriode();
        ~PeriodFinder();
        int getRunningAvgPeriode();
        long findSamplesize(long samplesize,int mode, int periode);
        void calcPeriodeThread();
        void changeFFTOps(FFTOps fftOps);
        FFTOps getFFT();
};
void calcPeriodeWrapper(PeriodFinder *finder);
