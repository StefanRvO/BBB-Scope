#pragma once
#include "structures.h"
#include <complex.h>
#include <fftw3.h>
#include <SDL2/SDL.h>
#include <thread>
#include <vector>
#include <unistd.h>
#include "RingBuffer.h"
/*
This is my attempt of detection the frequency of the signal.
It uses autocorrelation to find the frequency.
The autocorrelation is performed by a fft and a inverse fft
This is probably too slow to run on each frame update.
Later versions is partly inspired by 
https://stackoverflow.com/questions/4225432/how-to-compute-frequency-of-data-using-fft
*/
#define AVGSIZE 1 
class PeriodFinder {

    private:
        fftw_plan forward;
        fftw_plan backward;
        Options *options;
        std::vector<double> *samples;
        double *final;
        std::complex<double> *out;
        void fastAutocorrelate();
        void calcsize();
        void calcpointer();
        int size;
        SDL_Window *window;
        double *in;
        std::thread t1;
        int periode;
        RingBuffer<int,AVGSIZE> runningAvgBuf;
    public:
        PeriodFinder(Options *options_, std::vector<double> *samples_, SDL_Window *window_);
        void calcPeriode(); //gets the periode. Uses threading and is nonblocking, done is set to 1 when finished
        int getPeriode();
        void updatePlans(); //update inpointer
        void renewPlans(); //make new plans (eg. if the display has been resized)
        ~PeriodFinder();
        void findPeriode();
        int getRunningAvgPeriode();
        void finish();
        bool isDone();
        bool done;
};
void calcPeriodeWrapper(PeriodFinder *finder);
