#include "SampleGrabber.h"
#include "EventHandler.h"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
using namespace std;
void updateScreen(int sizex, int sizey,vector<int> &samples,vector<timeval> times)
{
    al_clear_to_color(al_map_rgb(0,0,0));
    int j=sizex;
    if (j>samples.size()) j=samples.size();
    for(int i=0;i<j-1;i++)
    {
        al_draw_line(i,samples[samples.size()-j+i]*sizey/4096,i+1,samples[samples.size()-j+i+1]*sizey/4096,al_map_rgb(127,0,127),1);
        
    }
    al_flip_display();
}
int main()
{

    //INIT

    SampleGrabber Grabber(3490);
    ALLEGRO_DISPLAY *display=NULL;
    int sizex=800,sizey=400;
    ALLEGRO_EVENT_QUEUE *event_queue=NULL;
    if(!al_init() or !al_init_primitives_addon() or !al_install_mouse())
    {
        cout << "failed to initialize allegro!\n";
        exit(-1);
    }
    al_install_keyboard();
    al_set_new_display_flags(ALLEGRO_RESIZABLE);
    display = al_create_display(sizex, sizey);
    if(!display)
    {
        cout << "failed to create display!\n";
        exit(-1);
    }
    event_queue = al_create_event_queue();
    if(!event_queue) 
    {
        cout << "failed to create event_queue!\n";
    }
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    vector<int> samples;
    vector<timeval> times;
    EventHandler eHandler(display, event_queue,sizex,sizey);
    
    //INIT END

    //MAIN LOOP
    while(true)
    {
        //Grab new stuff from SampleGrabber
        while(Grabber.sBuffer->size())
        {
            samples.push_back(Grabber.sBuffer->pop_front());
        }
        while(times.size()>samples.size())
        {
            times.push_back(Grabber.tBuffer->pop_front());
        }
        eHandler.HandleEvents();
        updateScreen(sizex,sizey,samples,times);
    
    }
    while(true) usleep(10000);
}
