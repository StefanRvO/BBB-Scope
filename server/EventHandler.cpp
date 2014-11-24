#include "EventHandler.h"
#include <iostream>
EventHandler::EventHandler(SDL_Window *window_, SDL_Renderer* renderer_,Options *options_ )
{
    window=window_;
    renderer=renderer_;
    options=options_;
}
void EventHandler::stateHandler()
    //Keyboard
    int lenght;
    const Uint8* keyState=SDL_SDL_GetKeyboardState(lenght);
    
    //Mouse
    int xpos,ypos;
    Uint32 mouseState=SDL_GetMouseState(&x,&y);
    
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
