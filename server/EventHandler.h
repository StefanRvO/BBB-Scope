#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "structures.h"
#include "Primitives.h"
#include "PeriodFinder.h"
class EventHandler
{
    private:
        SDL_Window *window;
        SDL_Renderer* renderer;
        Options *options;
        std::vector<unsigned long long> *times;
        std::vector<double> *samples;
        PeriodFinder *pFinder;
    public:
        EventHandler(SDL_Window *window_, SDL_Renderer* renderer_,Options *options_,std::vector<double> &samples_, std::vector<unsigned long long> &times_,PeriodFinder *pFinder );
        void handleEvents();
        void stateHandler();
    private:
        void handleKeyUpEvent(SDL_Event &event);
        void handleKeyDownEvent(SDL_Event &event);
        void handleMouseDownEvent(SDL_Event &event);
        void handleWindowEvent(SDL_Event &event);
};
