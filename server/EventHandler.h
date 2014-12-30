#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <limits>
#include "structures.h"
#include "Primitives.h"
#include "PeriodFinder.h"
#include "HugeBuffer.h"
#include "SampleGrabber.h"
class EventHandler
{
    private:
        SDL_Window *window;
        SDL_Renderer* renderer;
        Options *options;
        HugeBuffer<sample,20000000> *samples;
        PeriodFinder *pFinder;
        SampleGrabber *SGrabber;
    public:
        EventHandler(SDL_Window *window_, SDL_Renderer* renderer_,Options *options_,HugeBuffer<sample,20000000> *samples_,PeriodFinder *pFinder, SampleGrabber* SGrabber_ );
        void handleEvents();
        void stateHandler();
    private:
        void handleKeyUpEvent(SDL_Event &event);
        void handleKeyDownEvent(SDL_Event &event);
        void handleMouseDownEvent(SDL_Event &event);
        void handleWindowEvent(SDL_Event &event);
};
