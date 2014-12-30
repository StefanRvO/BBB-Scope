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
#include "SampleRateControl.h"
class UIDrawer
{
    private:
        PeriodFinder *Pfinder;
        SampleGrabber *Grabber;
        SDL_Window *window;
        SDL_Renderer* renderer;
        HugeBuffer<sample,20000000> samples;
        EventHandler *eventHandler;
        SampleRateControl *SRControl;
        Options *options;
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
        void drawDFT();
        void drawAutoCorr();
};
void SDL_RenderDrawThickLine(SDL_Renderer* renderer,
                       int           x1,
                       int           y1,
                       int           x2,
                       int           y2,
                       int           width);
