#pragma once
#include <string>
#include "EventHandler.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cstdlib>
#include "SampleGrabber.h"
#include <vector>
#include <sys/time.h>
#include "structures.h"
#include "TextDrawer.h"
#include "Timer.h"
#include "PeriodFinder.h"

class UIDrawer
{
    private:
        PeriodFinder *Pfinder;
        SampleGrabber *Grabber;
        SDL_Window *window;
        SDL_Renderer* renderer;
        std::vector<double> samples;
        std::vector<unsigned long long> times;
        EventHandler *eventHandler;
        Options options;
        Timer timer;
    public:
        UIDrawer(SampleGrabber* Grabber_);
        ~UIDrawer();
        int loop();  
    private:
        void drawUI();
        void drawSamples();
        void GetNewData();
        void Draw();
};
