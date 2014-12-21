#include "UIDrawer.h"
using namespace std;
UIDrawer::UIDrawer(SampleGrabber* Grabber_)
{
    Grabber=Grabber_;
    //Init SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        cout << "SDL_INIT Error: " << SDL_GetError() << endl;
        exit(0);
    }
    
    //Create a window
    window=SDL_CreateWindow("BBB-Scope", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                        512, 512,
                        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(window==nullptr)
    {
	    cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
	    SDL_Quit();
	    exit(0);
    }
    //Create Renderer
    renderer =SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer==nullptr)
    {
        SDL_DestroyWindow(window);
	    cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
	    SDL_Quit();
	    exit(0);
    }
    eventHandler=new EventHandler(window,renderer,&options,samples,times);
    
}
UIDrawer::~UIDrawer()
{
    delete eventHandler;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int UIDrawer::loop()
{
    while (options.alive)
    {
        GetNewData();
        eventHandler->handleEvents();
        Draw();
    }
    return 0;
}
void UIDrawer::GetNewData()
{
    while(Grabber->sBuffer->size()>1)
    {
        samples.push_back(Grabber->sBuffer->pop_front());
    }
    while(times.size()<samples.size() and Grabber->tBuffer->size()>1 )
    {
        times.push_back(Grabber->tBuffer->pop_front());
    }
}
void UIDrawer::Draw()
{
    //Clear screen
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);
    //Draw samples
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    w/=options.zoomX;
    h/=options.zoomY;
    int i=w;
    long samplesize=samples.size();
    if(samplesize-1 < w) i=samplesize-1;
    SDL_SetRenderDrawColor(renderer,255,0,0,255);
    cout << options.zoomY << " " << h << " " << options.zoomX << " " << w << endl;
    cout << options.offsetY << " " << options.offsetX << endl;
    samplesize-=options.offsetX;
    for(; i > 1; i-=((int)(0.5/options.zoomX))+1)
    {
        SDL_RenderDrawLine(renderer,(w-i)*options.zoomX,(samples[samplesize-i]*h/4096)+options.offsetY,(w-i+1)*options.zoomX,(samples[samplesize-i+1/options.zoomX]*h/4096)+options.offsetY);
    }
    eventHandler->stateHandler();
    SDL_RenderPresent(renderer);
}
