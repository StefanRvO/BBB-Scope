#include "UIDrawer.h"
using namespace std;
UIDrawer::UIDrawer(SampleGrabber* Grabber_):timer(Timer(60))
{
    Grabber=Grabber_;
    options=&Grabber->options;
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
    FFTOps fftOps;
    fftOps.size=5000;
    fftOps.start=0;
    fftOps.end=900;
    Pfinder=new PeriodFinder(options,&samples,window,Grabber,fftOps);   
    eventHandler=new EventHandler(window,renderer,options,&samples,Pfinder,Grabber);
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
    long framecounter=0;
    while (options->alive)
    {
        GetNewData();
        eventHandler->handleEvents();
        eventHandler->stateHandler();
        Draw();
        timer.tick();
    }
    return 0;
}
void UIDrawer::GetNewData()
{
    while(!(Grabber->sBuffer.empty()))
    {
        samples.push_back(Grabber->sBuffer.pop_front());
    }
}
void UIDrawer::Draw()
{
    //Clear screen
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    if(options->viewMode==0) drawSamples();
    else if(options->viewMode==1) drawDFT();
    else if(options->viewMode==2) drawAutoCorr();
    drawUI();
    SDL_RenderPresent(renderer);
}

void UIDrawer::drawUI()
{
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    
    int scaledW=w/options->zoomX;
    int scaledH=h/options->zoomY;
    //Draw circle at mousepos
    SDL_SetRenderDrawColor(renderer,255,255,0,255);
    //review index calculations. May be a bit off.
    long index=samplesize-scaledW+options->mouseX/options->zoomX;
    if(index>0 and index<(int)samples.size()-1)
    {
        drawFilledCircle(renderer,options->mouseX,(4096-samples.at(index).value)*(scaledH)/4096+options->offsetY,5);
        //write out value
    }
    //Draw axes
    SDL_SetRenderDrawColor(renderer,255,255,0,255);
    SDL_RenderDrawLine(renderer,0,scaledH/2+options->offsetY,w,scaledH/2+options->offsetY);
    SDL_RenderDrawLine(renderer,w/2,0,w/2,h);
    TextDrawer txtDraw("FreeSans.ttf",h/30);
    
    txtDraw.DrawText(renderer,(string("Framerate: ")+std::to_string(timer.getAvgFPS())).c_str(),0,0,200,200,40,0); //print framerate
    if(index>0 and index<(int)samples.size()-1)
    {
        txtDraw.DrawText(renderer,(string("Current value: ") +std::to_string(samples.at(index).value)).c_str(),0,h/30,200,200,40,0);
        long long diff= (((long long)index)-((long long)samples.size())-1)*options->sampletime;
        diff/=1000;
        txtDraw.DrawText(renderer,(string("Current time: ") +std::to_string(diff) +string(" µs")).c_str(),0,2*h/30,200,200,40,0);
        if(options->paused)
        {
            long long diff= (((long long)index)-((long long)samplesize)-1)*options->sampletime;
            diff/=1000;
            txtDraw.DrawText(renderer,(string("Current paused time : ") +std::to_string(diff) +string(" µs")).c_str(),0,3*h/30,200,200,40,0);
        }
    }
    
    //Draw samplerate
    txtDraw.DrawText(renderer,(string("Samplerate : ") +std::to_string(1000000000./(options->sampletime)) +string(" Hz")).c_str(),0,4*h/30,200,200,40,0);
    
    txtDraw.DrawText(renderer,(string("Periodelength : ") +std::to_string(Pfinder->getRunningAvgPeriode()) +string(" samples")).c_str(),0,5*h/30,200,200,40,0);
    txtDraw.DrawText(renderer,(string("PeriodeFrequency : ") +std::to_string(1000000000./(options->sampletime)/Pfinder->getRunningAvgPeriode()) +string(" Hz")).c_str(),0,6*h/30,200,200,40,0);
    switch (options->lockmode)
    {
        case 0:
            txtDraw.DrawText(renderer,"Lockmode: Auto",0,7*h/30,200,200,40,0);
            break;
        case 1:
            txtDraw.DrawText(renderer,"Lockmode: Smooth minlock",0,7*h/30,200,200,40,0);
            break;
        case 2:
            txtDraw.DrawText(renderer,"Lockmode: Smooth steplock min",0,7*h/30,200,200,40,0);
            break;
        case 3:
            txtDraw.DrawText(renderer,"Lockmode: Smooth steplock max",0,7*h/30,200,200,40,0);
            break;
        case 4:
            txtDraw.DrawText(renderer,"Lockmode: None",0,7*h/30,200,200,40,0);
            break;
    }
}

void UIDrawer::drawSamples()
{
    //Draw samples
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    w/=options->zoomX;
    h/=options->zoomY;
    
    if(options->paused)
    {
        samplesize=options->pausedSamplesize-options->offsetX;
    }
    else 
    {   //try to fit to signal so the signal locks in place
        samplesize=(long)samples.size()-(options->offsetX);
        samplesize=Pfinder->findSamplesize(samplesize,options->lockmode, Pfinder->getRunningAvgPeriode());
    }
    int i=w;
    if(samplesize-1 < w) i=samplesize-1;
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    /*cout << options->zoomY << " " << h << " " << options->zoomX << " " << w << endl;
    cout << options->offsetY << " " << options->offsetX << endl; */
    //samplesize-=options->offsetX;
    if(samplesize>samples.size()) return;
    for(; i > 1; i-=((int)(0.5/options->zoomX))+1)
    {
        if(samplesize-i<0 ) continue;
        if(samplesize-i+((int)(0.5/options->zoomX))+1>samples.size()) continue;
        Sint16 x1=(w-i)*options->zoomX;
        Sint16 y1=(4096-samples.at(samplesize-i).value)*h/4096+options->offsetY;
        Sint16 x2=(w-i+1)*options->zoomX;
        Sint16 y2=((4096-samples.at(samplesize-i+((int)(0.5/options->zoomX))+1).value)*h/4096)+options->offsetY;
        //thickLineRGBA (renderer, x1, y1, x2, y2, 1, 255,0,0,255);
        SDL_RenderDrawThickLine(renderer, x1,y1,x2,y2,2);
    }
}
void UIDrawer::drawDFT()
{
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    FFTOps fft=Pfinder->getFFT();
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    double max=-99999;
    for(long i=0;i<fft.size;i++) if(max<abs(fft.mem[i])) max=abs(fft.mem[i]);
    //cout << fft.size << " "<< max <<endl;
    for(long i=1;i<fft.size;i++)
    {
        Sint16 x1=((i-1)*w)/fft.size;
        Sint16 x2=(i*w)/fft.size;
        Sint16 y1=h-(abs(fft.mem[i-1])*h)/max;
        Sint16 y2=h-(abs(fft.mem[i])*h)/max;
        //cout << x1 << " " << y1 << endl;
        SDL_RenderDrawThickLine(renderer, x1,y1,x2,y2,2);
    }
    //draw fft
}
void UIDrawer::drawAutoCorr()
{

}
void SDL_RenderDrawThickLine(SDL_Renderer* renderer, //crude hack to avoid thickLineRGBA
                       int           x1,
                       int           y1,
                       int           x2,
                       int           y2,
                       int           width)
{
    float offset=width/2.;
    for(float i=-offset; i<offset; i++)
    SDL_RenderDrawLine(renderer, x1+i,y1,x2+i,y2);
}

