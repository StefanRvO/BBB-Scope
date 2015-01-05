#include "PeriodFinder.h"
#include <iostream>
using namespace std;
PeriodFinder::PeriodFinder(Options *options_, HugeBuffer<sample,20000000> *samples_, SDL_Window *window_,SampleGrabber *SGrabber_,FFTOps fftOps): t(Timer(UPDATERATE))
{
    options=options_;
    samples=samples_;
    window=window_;
    SGrabber=SGrabber_;
    windowFFTSize=fftOps.size;
    windowFFTFirst=fftOps.start;
    windowFFTLast=fftOps.end;
    windowFFT=(complex<double> *) fftw_alloc_complex(windowFFTSize);
    //Enable threading
    fftw_init_threads();
    fftw_plan_with_nthreads(3);
    for(int i=0;i<AVGSIZE; i++) runningAvgBuf.push_back(10000);
    //Alocate memory
    calcSize();
    calcPlacement();
    final=fftw_alloc_real(size*2);
    out = (complex<double> *) fftw_alloc_complex(size+1);
    con= (complex<double> *) fftw_alloc_complex(size+1);
    
    //create plans
    forward=fftw_plan_dft_r2c_1d(size*2, final, reinterpret_cast<fftw_complex*>(out),FFTW_ESTIMATE);
    backward=fftw_plan_dft_c2r_1d(size*2, reinterpret_cast<fftw_complex*>(con), final, FFTW_ESTIMATE);
    t1=thread(calcPeriodeWrapper,this);
}
void PeriodFinder::findPeriode()
{
    if(size==0 or placement<size)
    {
        periode=0;
        runningAvgBuf.push_back(periode);
        return;
    }
    long tmpplacement=placement-size;
    for(long i=0;i<size;i++)
    {
        final[i]=(samples->at(tmpplacement+i).value);
    }
    //remove bias
    double mean=0;
    for(long i=0;i<size;i++) mean+=final[i]/size;
    for(long i=0;i<size;i++) final[i]=(final[i]-mean)*(0.54-0.46*cos((2*M_PI*i)/(size-1)));
    
    
    for(long i=size; i<size*2; i++)
    {
        final[i]=0;
    }
    
    fastAutocorrelate();
    double t=final[0];
    
    for (long i=0; i< size; i++) final[i]/=t; //normalize
    long j=0;
    //find first valey
    double min=1;
    long minplacement=0;
    while(final[j]*0.95<min and j<size/3) 
    {
        if(final[j]<min)
        {
            min=final[j];
            minplacement=j;
        }
        j++;
    }
    //find peak
    j=minplacement;
    long maxplacement=0;
    double max=-1;
    /*while(final[j]>max*0.95 and j<size/3)
    {
        
        if(final[j]>max)
        {
            max=final[j];
            maxplacement=j;
        }
        j++;
    }*/
    
    for(long i=minplacement;i<size/3;i++)
    {
        if(final[i]>max)
        {
            maxplacement=i;
            max=final[i];
        }
    }
    j=maxplacement;

    if(j==0) j=size/3; //if none found, set periode to size/2

    periode = j;
    
    runningAvgBuf.push_back(periode);
    avgperiode=runningAvgBuf.getAvg()+0.5;
}
void PeriodFinder::fastAutocorrelate()
{
    fftw_execute(forward);
    
    fftlock.lock();
    double start,end;
    start=windowFFTFirst;
    end=windowFFTLast;
    if(start<0) start=0;
    if(start>size/2-2) start=size/2-2;
    if(end==0 or end>size/2-1) end=size/2-1;
    else if(start>end) end=start+1;
    double step=(end-start)/(double)windowFFTSize;
    long j=0;
    for( double i=start; /*i<=end and*/ j<windowFFTSize ;i+=step, j++) windowFFT[j]=out[(long)i];
    fftlock.unlock();
    
    for(int i=0; i<size+1;i++) con[i]=out[i]*conj(out[i]);
    fftw_execute(backward);
}

PeriodFinder::~PeriodFinder()
{
    stop=true;
    t1.join();
    fftw_destroy_plan(forward);
    fftw_destroy_plan(backward);
    fftw_free(out);
    fftw_free(final);
    fftw_free(con);
    fftw_free(windowFFT);
}
void PeriodFinder::calcSize() //calculate the number of samples to perform fft on, normaly about 5*last measured periode
{
    auto tmpPeriode=periode;
    if(tmpPeriode<5000) tmpPeriode=5000;
    if(tmpPeriode>100000) tmpPeriode=100000;
    long tmpSize=tmpPeriode*20;
    long samplesize;
    if(options->paused)
    {
        samplesize=options->pausedSamplesize;
    }
    else samplesize=samples->size();
    if(tmpSize>samplesize) tmpSize = samplesize;
    tmpSize=sqrt(tmpSize);
    tmpSize*=tmpSize; //limit size to be a square number. Gives generally better results.
    size=tmpSize;
    //std::cout << size << std::endl;
}
void PeriodFinder::calcPlacement()
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
    long placement_tmp=samplesize-w;
    //cout << "pointeroff=" << placement << endl;
    if(placement_tmp>0) placement=placement_tmp;
    else placement=size;
    //std::cout << placement << std::endl;

}
int PeriodFinder::getPeriode()
{
    return periode;
}
int PeriodFinder::getRunningAvgPeriode()
{
    return avgperiode;
}
void PeriodFinder::renewPlans()
{
    calcPlacement();
    auto tmpSize=size;
    calcSize();
    if(size==tmpSize) return; //don't renew plans if nothing changed.
    fftw_free(out);
    fftw_free(final);
    fftw_free(con);
    fftw_destroy_plan(forward);
    fftw_destroy_plan(backward);
    final=fftw_alloc_real(size*2);
    out = (complex<double> *) fftw_alloc_complex(size+1);
    con= (complex<double> *) fftw_alloc_complex(size+1);
    forward=fftw_plan_dft_r2c_1d(size*2, final, reinterpret_cast<fftw_complex*>(out),FFTW_ESTIMATE);
    backward=fftw_plan_dft_c2r_1d(size*2, reinterpret_cast<fftw_complex*>(con), final, FFTW_ESTIMATE);
}
long PeriodFinder::findSamplesize(long samplesize,int mode, int periode) //Calculate an offset on the samplesize to "fix" the periodes in place. return the modified samplesize
{
    if (periode==0) return samplesize;
    int stepsize=periode/FSAMPLESIZEACC;
    //stepsize=1;
    if (stepsize==0) stepsize=1;
    //This code is not safe. May read out of bounds TOFIX
    if(mode==4)  return samplesize;
    else if(mode==0)
    {
        //search for best lockmode
        options->lockmode=FindBestLockMode(samplesize,periode);
        mode=options->lockmode;
    }
    if(mode==1) //minlock, smooth
    {
        //Smoth the wave before analysing. Find min.
        long tsamplesize=samplesize;
        while(samplesize+5*stepsize>samples->size()) samplesize-=periode;
        while(samplesize-periode<0) samplesize+=periode;
        if(samplesize+5*stepsize<=samples->size())
        {
            samplesize-=5;
            int min=0;
            float minval=99999;
            RingBuffer<float,5> smoother;
            for(int i=-5*stepsize;i<periode; i+=stepsize)
            {
                smoother.push_back(samples->at(samplesize-i).value);
                if(i>=0)
                {
                    if (smoother.getAvg()<minval) 
                    {
                        min=i+3*stepsize;
                        minval=smoother.getAvg();
                    }
                
                }
            
            }
            samplesize-=min;
        }
        else samplesize=tsamplesize;
    }
    else if(mode==2)
    {
         //Find the biggest (positive) change in the periode relative to the average change of the last x samples, but smooth first
         
        long tsamplesize=samplesize;
        while(samplesize+5*stepsize>samples->size()) samplesize-=periode;
        while(samplesize-periode<0) samplesize+=periode;
        if(samplesize+5*stepsize<=samples->size())
        {
            samplesize-=5;
            int max=0;
            float maxval=-99999;
            RingBuffer<float,5> smoother;
            RingBuffer<float, 5> smoother2;
            float lastval=0;
            for(int i=-5*stepsize;i<periode; i+=stepsize)
            {
                smoother.push_back(samples->at(samplesize-i).value);
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
        else samplesize=tsamplesize;
    }
    else if(mode==3)
    {
         //Find the biggest (negative) change in the periode relative to the average change of the last x samples, but smooth first
        long tsamplesize=samplesize;
        while(samplesize+5*stepsize>samples->size()) samplesize-=periode;
        while(samplesize-periode<0) samplesize+=periode;
        if(samplesize+5*stepsize<=samples->size())
        {
            samplesize-=5;
            int min=0;
            float minval=99999;
            RingBuffer<float,5> smoother;
            RingBuffer<float, 5> smoother2;
            float lastval=0;
            for(int i=-5*stepsize;i<periode; i+=stepsize)
            {
                smoother.push_back(samples->at(samplesize-i).value);
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
        else samplesize=tsamplesize;
    }
    return samplesize;
}

int PeriodFinder::FindBestLockMode(long samplesize,int periode)
{   //Tries the different lockmodes and find the most stable one.
    long besttotaldiff=99999;
    int bestmode=0;
    for(int i=1;i<=3;i++)
    {
        //first find initial lock
        long tmpsamplesize=samplesize;
        long maxdiff=0;
        long mindiff=100000000;
        tmpsamplesize=findSamplesize(tmpsamplesize,i,periode);
        for(int j=0;j<10;j++)
        {
            //go back a few samples each time and calc the diff compared to last time
            long diff=tmpsamplesize-findSamplesize(tmpsamplesize-5,i,periode);
            tmpsamplesize=tmpsamplesize-diff;
           // cout << diff << "\t" << i << "\t" << j << endl;
            if (diff>maxdiff) maxdiff=diff;
            if (diff<mindiff) mindiff=diff;
        }
        long totaldiff=maxdiff-mindiff;
     //   cout << i << "\t" << totaldiff << endl;
        if(totaldiff<besttotaldiff)
        {
            besttotaldiff=totaldiff;
            bestmode=i;
        }
        else if(abs(totaldiff-periode)<besttotaldiff) //to take a potential skipped periode into account, also try with periode substracted
        {
            besttotaldiff=abs(totaldiff-periode);
            bestmode=i;
        }
    }
    options->sampleMaxMin=0;
    return bestmode;
}
void PeriodFinder::calcPeriodeThread()
{
    int tick=0;
    uint64_t lasttime=options->sampletime;
    while(!stop)
    {
        tick++;
        t.tick();
        renewPlans();
        findPeriode();
        //cout << runningAvgBuf.getAvg() << " "  << runningAvgBuf.getRelativeStandDiv() << " " << options->connected << " "<< (int)options->sampleMaxMin << endl;
        if(tick%4!=0 or !options->autoSampleRate) continue;
        if(options->connected and !options->locked and  getPeriode()>100000 and lasttime!=options->sampletime)
        {
            lasttime=options->sampletime;
            
            SGrabber->RequestChangedRate(-(options->sampletime/1000)); //change samplerate .1%
        }
        else if(options->connected and lasttime!=options->sampletime and !options->locked and getPeriode()<500)
        {
            lasttime=options->sampletime;
            SGrabber->RequestChangedRate(options->sampletime/1000); //change samplerate .1%
        }
        else options->adjusted=0;
    }
}
void PeriodFinder::changeFFTOps(FFTOps fftOps)
{
    fftlock.lock();
    windowFFTSize=fftOps.size;
    windowFFTFirst=fftOps.start;
    windowFFTLast=fftOps.end;
    fftw_free(windowFFT);
    windowFFT=(complex<double> *) fftw_alloc_complex(size+1);
    fftlock.unlock();
}
FFTOps PeriodFinder::getFFT()
{
    fftlock.lock();
    FFTOps fftOps;
    fftOps.mem=windowFFT;
    fftOps.size=windowFFTSize;
    fftOps.start=windowFFTFirst;
    fftOps.end=windowFFTLast;
    fftOps.totalsize=size;
    fftlock.unlock();
    return fftOps;
}
void calcPeriodeWrapper(PeriodFinder *finder)
{ 
    finder->calcPeriodeThread();
}
