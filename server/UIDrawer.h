#pragma once
#include "EventHandler.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <cstdlib>
#include "SampleGrabber.h"
#include <vector>
#include <sys/time.h>
#include "structures.h"

class UIDrawer
{
    private:
        SampleGrabber* Grabber;
        SDL_Window *window;
        SDL_Renderer* renderer;
        std::vector<unsigned short> samples;
        std::vector<unsigned long long> times;
        EventHandler *eventHandler;
        Options options;
    public:
        UIDrawer(SampleGrabber* Grabber_);
        ~UIDrawer();
        int loop();  
    private:
        void GetNewData();
        void Draw();
};
