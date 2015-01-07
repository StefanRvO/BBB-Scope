#include "EventHandler.h"
#include <iostream>
EventHandler::EventHandler(SDL_Window *window_, SDL_Renderer* renderer_,Options *options_, HugeBuffer<sample,20000000> *samples_,PeriodFinder *pFinder_,SampleGrabber* SGrabber_)
{
    window=window_;
    renderer=renderer_;
    options=options_;
    samples=samples_;
    pFinder=pFinder_;
    SGrabber=SGrabber_;
}
void EventHandler::stateHandler()
{
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    //Keyboard
    int lenght;
    const Uint8* keyState=SDL_GetKeyboardState(&lenght);
    if (keyState[SDL_SCANCODE_KP_PLUS] and !keyState[SDL_SCANCODE_LCTRL] and options->zoomX<3)
    {
        double newZoomX=options->zoomX+options->zoomX*0.02;
        double zoom_ratio=newZoomX/options->zoomX;
        options->offsetX-=(w/(options->zoomX*zoom_ratio))*(1-zoom_ratio)/2;
        options->zoomX=newZoomX;
    }
    else if(keyState[SDL_SCANCODE_KP_PLUS] and keyState[SDL_SCANCODE_LCTRL])
    {
        double newZoomY=options->zoomY-options->zoomY*0.02;
        double zoom_ratio=newZoomY/options->zoomY;
        options->offsetY-=(h/(options->zoomY*zoom_ratio))*(1-zoom_ratio)/2;
        options->zoomY=newZoomY;
    }
    else if(keyState[SDL_SCANCODE_KP_MINUS] and !keyState[SDL_SCANCODE_LCTRL])
    {
        double newZoomX=options->zoomX-options->zoomX*0.02;
        double zoom_ratio=newZoomX/options->zoomX;
        options->offsetX-=(w/(options->zoomX*zoom_ratio))*(1-zoom_ratio)/2;
        options->zoomX=newZoomX;
    }
    else if(keyState[SDL_SCANCODE_KP_MINUS] and keyState[SDL_SCANCODE_LCTRL])
    {
        double newZoomY=options->zoomY+options->zoomY*0.02;
        double zoom_ratio=newZoomY/options->zoomY;
        options->offsetY-=(h/(options->zoomY*zoom_ratio))*(1-zoom_ratio)/2;
        options->zoomY=newZoomY;
    }
    if(keyState[SDL_SCANCODE_LEFT])
    {
        options->offsetX+=5/(options->zoomX);
    }
    if(keyState[SDL_SCANCODE_RIGHT])
    {
        options->offsetX-=5/(options->zoomX);
    }
    if(keyState[SDL_SCANCODE_UP])
    {
        options->offsetY+=5/(options->zoomY);
    }
    if(keyState[SDL_SCANCODE_DOWN])
    {
        options->offsetY-=5/(options->zoomY);
    }
    if(options->offsetX<(long long)options->pausedSamplesize-(long long)samples->size() and options->paused)
    {
        //std::cout << (long long)options->pausedSamplesize-(long long)samples->size() << std::endl;
        options->offsetX=(long long)options->pausedSamplesize-(long long)samples->size();
    }
    else if(options->offsetX<0 and !options->paused)
    {
        options->offsetX=0;
    }
    if(options->offsetX>samples->size()-1)
    {
        options->offsetX=samples->size()-1;
    }
    //Mouse
    int xpos,ypos;
    SDL_GetMouseState(&xpos,&ypos);
    options->mouseX=xpos;
    options->mouseY=ypos;
}    
void EventHandler::handleEvents()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                options->alive=false;
                break;
            case SDL_KEYDOWN:
                handleKeyDownEvent(event);
                break;
            case SDL_KEYUP:
                handleKeyUpEvent(event);
                break;
            case SDL_WINDOWEVENT:
                handleWindowEvent(event);
                break;
            case SDL_MOUSEBUTTONDOWN:
                handleMouseDownEvent(event);
                break;
        }
    }
}

void EventHandler::handleKeyDownEvent(SDL_Event &event)
{
    if(event.key.keysym.scancode==SDL_SCANCODE_R)
    {
        if(event.key.keysym.mod==KMOD_LCTRL)
        {
            options->zoomY=1;
            options->zoomX=1;
            options->offsetY=0;
        }
        options->offsetX=0;
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_SPACE) 
    {
        if(not (event.key.keysym.mod==KMOD_LCTRL))
        options->paused^=1;
        if(options->paused) options->pausedSamplesize=pFinder->findSamplesize(samples->size(),options->lockmode,pFinder->getRunningAvgPeriode())+options->offsetX;
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_M)
    {
        options->lockmode++;
        if(options->lockmode>4) options->lockmode=1;
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_A)
    {
        if(event.key.keysym.mod==KMOD_LCTRL) options->lockmode=0; //set lockmode to auto
        //calculate zoomlevel to show 5 periodes on screen
        int w,h;
        SDL_GetWindowSize(window,&w,&h);
        std::cout << options->zoomX << std::endl;
        double zoom= ((double)w)/(5*pFinder->getRunningAvgPeriode());
        if(zoom==0 or zoom==std::numeric_limits<double>::infinity()) zoom=options->zoomX;
        options->offsetX=0;
        options->zoomX=zoom;
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_O)
    {
        SGrabber->RequestChangedRate(50);
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_P)
    {
        SGrabber->RequestChangedRate(-50);
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_F)
    {
        if(options->viewMode<2)
        {
            options->viewMode++;
            if(options->viewMode>=2) options->viewMode=0;
        }
    }
    else if(event.key.keysym.scancode==SDL_SCANCODE_S)
    {
        options->autoSampleRate^=1;
    }
}
void EventHandler::handleKeyUpEvent(SDL_Event &event)
{

}
void EventHandler::handleMouseDownEvent(SDL_Event &event)
{

}
void EventHandler::handleWindowEvent(SDL_Event &event)
{

}
