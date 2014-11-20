#pragma once
#include "SampleGrabber.h"
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <sys/time.h>
#include <thread>
class UIDisplay
{
    private:
        SampleGrabber* Grabber;
        std::vector<int> samples;
        std::vector<timeval> times;
        ALLEGRO_DISPLAY *display=NULL;
        ALLEGRO_EVENT_QUEUE *event_queue=NULL;
        int sizex,sizey;
        ALLEGRO_EVENT ev;
        ALLEGRO_FONT *font;
        std::thread t1;
    public:
        UIDisplay(SampleGrabber *Grabber_,int sizex_,int sizey_);
        ~UIDisplay();
        void UIloop();
        void updateScreen();        
        
};
void UIThreadWrapper(UIDisplay *UID);
