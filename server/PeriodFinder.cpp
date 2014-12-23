#include "PeriodFinder.h"
#include <iostream>
using namespace std;
PeriodFinder::PeriodFinder(Options *options_, vector<double> *samples_, SDL_Window *window_)
{
    options=options_;
    samples=samples_;
    window=window_;
    
    //Enable threading
    fftw_init_threads();
    fftw_plan_with_nthreads(3);
    
    //Alocate memory
    calcsize();
    calcpointer();
    final=fftw_alloc_real(size*2);
    out = (complex<double> *) fftw_alloc_complex(size+1);
    
    //create plans
    forward=fftw_plan_dft_r2c_1d(size, final, reinterpret_cast<fftw_complex*>(out),FFTW_ESTIMATE);
    backward=fftw_plan_dft_c2r_1d(size, reinterpret_cast<fftw_complex*>(out), final, FFTW_ESTIMATE);
    
}
void PeriodFinder::findPeriode()
{
    if(size==0)
    {
        periode=0;
        runningAvgBuf.push_back(periode);
        return;
    }
    for(int i=0;i<size;i++)
    {
        final[i]=in[i]*(0.54-0.46*cos((2*M_PI*i)/(size-1)));
    }
    for(int i=size; i<size*2; i++)
    {
        final[i]=0;
    }
    fastAutocorrelate();
    long first=0;
    for (long i=1; i< size/3; i++)
    {
        //Find largest peak not at 0
        if((final[first]<final[i] or first==0) and final[i]>final[i-1]) first=i;
    }
    periode = first;
    runningAvgBuf.push_back(periode);
}
void PeriodFinder::fastAutocorrelate()
{
    fftw_execute(forward);
    for(int i=0; i<size/2+1;i++) out[i]*=conj(out[i]);
    fftw_execute(backward);
}

PeriodFinder::~PeriodFinder()
{
    fftw_destroy_plan(forward);
    fftw_destroy_plan(backward);
    fftw_free(out);
    fftw_free(final);
}
void PeriodFinder::calcsize() //return the number of samples to perform fft on
{
    long samplesize;
    if(options->paused)
    {
        samplesize=options->pausedSamplesize;
    }
    else samplesize=samples->size();
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    w/=options->zoomX;
    samplesize-=options->offsetX;
    if(w<samplesize) size = w;
    else size = samplesize;
    /*size=sqrt(size);
    size*=size;*/
    //if(getRunningAvgPeriode()==200) cout << options->zoomX << " " << size << endl;
}
void PeriodFinder::calcpointer()
{
    long samplesize;
    if(options->paused)
    {
        samplesize=options->pausedSamplesize;
    }
    else samplesize=samples->size();
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    w/=options->zoomX;
    samplesize-=options->offsetX;
    long placement=samplesize-w;
    //cout << "pointeroff=" << placement << endl;
    if(placement>0) in=samples->data()+placement;
    else in=samples->data();

}
void PeriodFinder::calcPeriode()
{
     done=false;
     t1=thread(calcPeriodeWrapper,this);
}
void PeriodFinder::finish()
{
    t1.join();
}
bool PeriodFinder::isDone()
{
    return done;
}
int PeriodFinder::getPeriode()
{
    return periode;
}
int PeriodFinder::getRunningAvgPeriode()
{
    return (runningAvgBuf.getAvg()+0.5);
}
void PeriodFinder::updatePlans()
{
    calcpointer();
}
void PeriodFinder::renewPlans()
{
    calcpointer();
    calcsize();
    fftw_free(out);
    fftw_free(final);
    fftw_destroy_plan(forward);
    fftw_destroy_plan(backward);
    final=fftw_alloc_real(size*2);
    out = (complex<double> *) fftw_alloc_complex(size+1);
    forward=fftw_plan_dft_r2c_1d(size, in, reinterpret_cast<fftw_complex*>(out),FFTW_ESTIMATE);
    backward=fftw_plan_dft_c2r_1d(size, reinterpret_cast<fftw_complex*>(out), final, FFTW_ESTIMATE);
    
}
void calcPeriodeWrapper(PeriodFinder *finder)
{
    finder->findPeriode();
    finder->done=true;
}
