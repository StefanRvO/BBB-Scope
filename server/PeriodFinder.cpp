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
    for (long i=2; i< size/3; i++)
    {
        //Find largest peak larger than 2
        if((final[first]<final[i] or first==0) and final[i]>final[i-1] and final[i]>final[i-2]) first=i;
    }
    periode = first;
    runningAvgBuf.push_back(periode);
    avgperiode=runningAvgBuf.getAvg()+0.5;
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
    //samplesize-=options->offsetX; //why are we doing this?
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
    return avgperiode;
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
long PeriodFinder::findSamplesize(long samplesize,int mode) //Calculate an offset on the samplesize to "fix" the periodes in place. return the modified samplesize
{
    //This code is not safe. May read out of bounds TOFIX
    if(mode==0)
    {
        //search for best lockmode
        options->lockmode=FindBestLockMode(samplesize);
        mode=options->lockmode;
    }
    if(mode==1) //minlock
    {
        //Find the min of the signal in a range backward of periode. 
        //The min+1 have to be bigger than avg of last x samples
        int min=0;
        const int runningavg=3;
        for(int i=1; i<avgperiode;i++)
        {
            if(*(samples->data()+samplesize-i)<=*(samples->data()+samplesize-min) ) 
            {
                float avg=0;
                for(int j=0;j<runningavg;j++)
                {
                    avg+=*(samples->data()+samplesize-i-j);
                }
                avg/=runningavg;
                if(avg>*(samples->data()+samplesize-i+1))  min=i;
            }
        }
        samplesize-=min;
    }
    else if(mode==2) //steplock
    {
    //Find the biggest (positive) change in the periode relative
    //to the average change of the last x samples
        const int runningavg=5;
        float biggestchange=0;
        int biggestchangeindex=0;
        for(int i=1; i<avgperiode; i++)
        {
            float average=0;
            for(int j=1; j<=runningavg; j++)
            {
                average+=*(samples->data()+samplesize-i-j)-*(samples->data()+samplesize-i-j-1);
            }
            average/=runningavg;
            float diff=*(samples->data()+samplesize-i)-*(samples->data()+samplesize-i-1)-average;
            if(diff>biggestchange)
            {
               biggestchange=diff;
               biggestchangeindex=i;
            }
        }
        samplesize-=biggestchangeindex;
    }
    else if(mode==3) //STEPLOCK
    {
        //Smoth the wave before analysing. Find min.
        if(samples->size()>5 and samplesize>5)
        {
            samplesize-=5;
            int min=0;
            float minval=99999;
            RingBuffer<float,5> smoother;
            for(int i=-5;i<avgperiode; i++)
            {
                smoother.push_back(*(samples->data()+samplesize-i));
                if(i>=0)
                {
                    if (smoother.getAvg()<minval) 
                    {
                        min=i+3;
                        minval=smoother.getAvg();
                    }
                
                }
            
            }
            samplesize-=min;
        }
    }
    else if(mode==4)
    {
         //Find the biggest (positive) change in the periode relative to the average change of the last x samples, but smooth first
         
        if(samples->size()>5 and samplesize>5)
        {
            samplesize-=5;
            int max=0;
            float maxval=-99999;
            RingBuffer<float,5> smoother;
            RingBuffer<float, 5> smoother2;
            float lastval=0;
            for(int i=-5;i<avgperiode; i++)
            {
                smoother.push_back(*(samples->data()+samplesize-i));
                if(i>=0)
                {
                    if(i>=1)
                    {
                        smoother2.push_back(lastval-smoother.getAvg());
                    }
                    if(i>=5)
                    {
                        if(maxval<smoother2.getAvg())
                        {
                            maxval=smoother2.getAvg();
                            max=i;
                        }
                    }
                    lastval=smoother.getAvg();
                }
            }
            samplesize-=max;
        }
    }
    else if(mode==5)
    {
         //Find the biggest (negative) change in the periode relative to the average change of the last x samples, but smooth first
         
        if(samples->size()>5 and samplesize>5)
        {
            samplesize-=5;
            int min=0;
            float minval=99999;
            RingBuffer<float,5> smoother;
            RingBuffer<float, 5> smoother2;
            float lastval=0;
            for(int i=-5;i<avgperiode; i++)
            {
                smoother.push_back(*(samples->data()+samplesize-i));
                if(i>=0)
                {
                    if(i>=1)
                    {
                        smoother2.push_back(lastval-smoother.getAvg());
                    }
                    if(i>=5)
                    {
                        if(minval>smoother2.getAvg())
                        {
                            minval=smoother2.getAvg();
                            min=i;
                        }
                    }
                    lastval=smoother.getAvg();
                }
            }
            samplesize-=min;
        }
    }
    return samplesize;
}
void calcPeriodeWrapper(PeriodFinder *finder)
{
    finder->findPeriode();
    finder->done=true;
}
int PeriodFinder::FindBestLockMode(long samplesize)
{   //Tries the different lockmodes and find the most stable one.
    long besttotaldiff=99999;
    int bestmode=0;
    for(int i=1;i<=5;i++)
    {
        //first find initial lock
        long tmpsamplesize=samplesize;
        long maxdiff=0;
        long mindiff=100000000;
        tmpsamplesize=findSamplesize(tmpsamplesize,i);
        for(int j=0;j<10;j++)
        {
            //go back a few samples each time and calc the diff compared to last time
            long diff=tmpsamplesize-findSamplesize(tmpsamplesize-5,i);
            tmpsamplesize=tmpsamplesize-diff;
            cout << diff << "\t" << i << "\t" << j << endl;
            if (diff>maxdiff) maxdiff=diff;
            if (diff<mindiff) mindiff=diff;
        }
        long totaldiff=maxdiff-mindiff;
        cout << i << "\t" << totaldiff << endl;
        if(totaldiff<besttotaldiff)
        {
            besttotaldiff=totaldiff;
            bestmode=i;
        }
        else if(abs(totaldiff-getRunningAvgPeriode())<besttotaldiff) //to take a potential skipped periode into account, also try with periode substracted
        {
            besttotaldiff=abs(totaldiff-getRunningAvgPeriode());
            bestmode=i;
        }
    }
    return bestmode;
}
