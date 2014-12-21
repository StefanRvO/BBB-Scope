#include "EventHandler.h"
#include <iostream>
EventHandler::EventHandler(SDL_Window *window_, SDL_Renderer* renderer_,Options *options_, std::vector<unsigned short> &samples_, std::vector<unsigned long long> &times_)
{
    window=window_;
    renderer=renderer_;
    options=options_;
    samples=&samples_;
    times=&times_;
}
void EventHandler::stateHandler()
{
    int w,h;
    SDL_GetWindowSize(window,&w,&h);
    //Keyboard
    int lenght;
    const Uint8* keyState=SDL_GetKeyboardState(&lenght);
    if (keyState[SDL_SCANCODE_KP_PLUS] and !keyState[SDL_SCANCODE_LCTRL])
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
    if(options->offsetX<0)
    {
        options->offsetX=0;
    }
    //Mouse
    int xpos,ypos;
    Uint32 mouseState=SDL_GetMouseState(&xpos,&ypos);
    SDL_SetRenderDrawColor(renderer,255,255,0,255);
    if((int)samples->size()-1-w+xpos>=0 and samples->size()>1+w-xpos)
    {
        drawFilledCircle(renderer,xpos,samples->at((int)samples->size()-1-w+xpos)*h/4096,5);
    }
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
