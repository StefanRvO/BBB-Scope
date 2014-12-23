#include "UIDrawer.h"
using namespace std;
UIDrawer::UIDrawer(SampleGrabber* Grabber_):timer(Timer(60))
{
    Grabber=Grabber_;
    //Init SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cout << "SDL_INIT Error: " << SDL_GetError() << endl;
        exit(0);
    }
    // Initialize SDL_ttf library
    if (TTF_Init() != 0)
    {
        cerr << "TTF_Init() Failed: " << TTF_GetError() << endl;
        SDL_Quit();
        TTF_Quit();
        exit(1);
        
    }
    
    //Create a window
    window=SDL_CreateWindow("BBB-Scope", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        512, 512,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(window==nullptr)
    {
	    cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
	    SDL_Quit();
	    TTF_Quit();
	    exit(0);
    }
    //Create Renderer
    renderer =SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer==nullptr)
    {
        SDL_DestroyWindow(window);
	    cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
	    SDL_Quit();
	    TTF_Quit();
	    exit(0);
    }
    eventHandler=new EventHandler(window,renderer,&options,samples,times);
    Pfinder=new PeriodFinder(&options,&samples,window);   
}
UIDrawer::~UIDrawer()
{
    delete eventHandler;
    delete Pfinder;
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int UIDrawer::loop()
{
    while (options.alive)
    {
        GetNewData();
        Pfinder->renewPlans();
        Pfinder->calcPeriode();
        eventHandler->handleEvents();
        eventHandler->stateHandler();
        Draw();
        timer.tick();
        Pfinder->finish();
        cout << Pfinder->getRunningAvgPeriode() << endl;
    }
    return 0;
}
void UIDrawer::GetNewData()
{
    while(!(Grabber->sBuffer.empty()))
    {
        samples.push_back(Grabber->sBuffer.pop_front());
    }
    while(times.size()<samples.size())
    {
        while(Grabber->tBuffer.empty());
        times.push_back(Grabber->tBuffer.pop_front());
    }
}
void UIDrawer::Draw()
{
    //Clear screen
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    drawSamples();
    drawUI();
    SDL_RenderPresent(renderer);
}

void UIDrawer::drawUI()
{
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    
    int scaledW=w/options.zoomX;
    int scaledH=h/options.zoomY;
    //Draw circle at mousepos
    SDL_SetRenderDrawColor(renderer,255,255,0,255);
    int index=(int)samplesize-1-scaledW+options.mouseX/options.zoomX;
    if(index>0 and index<(int)samples.size()-1)
    {
        drawFilledCircle(renderer,options.mouseX,(4096-samples[index])*(scaledH)/4096+options.offsetY,5);
        //write out value
    }
    //Draw axes
    SDL_SetRenderDrawColor(renderer,255,255,0,255);
    SDL_RenderDrawLine(renderer,0,scaledH/2+options.offsetY,w,scaledH/2+options.offsetY);
    SDL_RenderDrawLine(renderer,w/2,0,w/2,h);
    TextDrawer txtDraw("FreeSans.ttf",w/30);
    
    txtDraw.DrawText(renderer,(string("Framerate: ")+std::to_string(timer.getAvgFPS())).c_str(),0,0,200,200,40,0); //print framerate
    if(index>0 and index<(int)samples.size()-1)
    {
        txtDraw.DrawText(renderer,(string("Current value: ") +std::to_string(samples[index])).c_str(),0,h/30,200,200,40,0);
        long long diff=times[index]-times[(int)times.size()-1];
        txtDraw.DrawText(renderer,(string("Current time: ") +std::to_string(diff) +string(" µs")).c_str(),0,2*h/30,200,200,40,0);
        if(options.paused)
        {
            diff=times[index]-times[options.pausedSamplesize-1];
            txtDraw.DrawText(renderer,(string("Current paused time : ") +std::to_string(diff) +string(" µs")).c_str(),0,3*h/30,200,200,40,0);
        }
    }
    
    //Estimate and draw samplerate
    if((int)samples.size()>5)
    {
        long long diff=0;
        int indexpoint=5;
        while(diff<1000000 and indexpoint<(int)times.size())
        {
            diff=times[(int)times.size()-1]-times[(int)times.size()-indexpoint-1];
            indexpoint=indexpoint*1.3+5;
        }
        float rate=1000000./diff;
        rate*=indexpoint;
        txtDraw.DrawText(renderer,(string("samplerate : ") +std::to_string((int)rate) +string(" Hz")).c_str(),0,4*h/30,200,200,40,0);
    }
    
}

void UIDrawer::drawSamples()
{
    //Draw samples
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    w/=options.zoomX;
    h/=options.zoomY;
    if(options.paused)
    {
        samplesize=options.pausedSamplesize-options.offsetX;
    }
    else 
    {   //try to fit to signal so the signal locks in place
        long newsamplesize=(long)samples.size()-options.offsetX;
        long diff=newsamplesize-samplesize;
        
        //cut diff down to multiple of frequency
        int periode=Pfinder->getRunningAvgPeriode();
        if (periode!=0)
        {
            diff/=periode;
            diff*=periode;
        }
        samplesize+=diff;
        //Find the min of the signal in a range backward of periode*1.5. The min+1 have to be bigger than avg of last x samples
        int min=0;
        const int runningavg=3;
        for(int i=1; i<periode*1.5;i++)
        {
            if(samples[samplesize-i]<=samples[samplesize-min] ) 
            {
                float avg=0;
                for(int j=0;j<runningavg;j++)
                {
                    avg+=samples[samplesize-i-j];
                }
                avg/=runningavg;
                if(avg>samples[samplesize-i+1])  min=i;
            }
        }
        samplesize-=min;
        //Hack to make this work with flat-bootom waves.
        
    }
    int i=w;
    if(samplesize-1 < w) i=samplesize-1;
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    /*cout << options.zoomY << " " << h << " " << options.zoomX << " " << w << endl;
    cout << options.offsetY << " " << options.offsetX << endl; */
    //samplesize-=options.offsetX;
    for(; i > 1; i-=((int)(0.5/options.zoomX))+1)
    {
        if(samplesize-i<0) continue;
        //SDL_RenderDrawLine(renderer,(w-i)*options.zoomX,((4096-samples[samplesize-i])*h/4096)+options.offsetY,(w-i+1)*options.zoomX,((4096-samples[samplesize-i+((int)(0.5/options.zoomX))+1])*h/4096)+options.offsetY);
        thickLineRGBA (renderer, (w-i)*options.zoomX,((4096-samples[samplesize-i])*h/4096)+options.offsetY,(w-i+1)*options.zoomX,((4096-samples[samplesize-i+((int)(0.5/options.zoomX))+1])*h/4096)+options.offsetY,
            1, 255,0,0,255);
    }
}
