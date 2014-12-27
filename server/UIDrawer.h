#pragma once
#include <string>
#include "EventHandler.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <iostream>
#include <cstdlib>
#include "SampleGrabber.h"
#include <vector>
#include <sys/time.h>
#include "structures.h"
#include "TextDrawer.h"
#include "Timer.h"
#include "PeriodFinder.h"
#include "HugeBuffer.h"

class UIDrawer
{
    private:
        PeriodFinder *Pfinder;
        SampleGrabber *Grabber;
        SDL_Window *window;
        SDL_Renderer* renderer;
        HugeBuffer<sample,65000000> samples;
        EventHandler *eventHandler;
        Options options;
        Timer timer;
        long samplesize=0;
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
