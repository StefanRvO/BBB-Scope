#pragma once
#include <SDL2/SDL.h>
#include "structures.h"
class EventHandler
{
    private:
        SDL_Window *window;
        SDL_Renderer* renderer;
        Options *options;
    public:
        EventHandler(SDL_Window *window_, SDL_Renderer* renderer_,Options *options_ );
        void handleEvents();
    private:
        void handleKeyUpEvent(SDL_Event &event);
        void handleKeyDownEvent(SDL_Event &event);
        void handleMouseDownEvent(SDL_Event &event);
        void handleWindowEvent(SDL_Event &event);
};
