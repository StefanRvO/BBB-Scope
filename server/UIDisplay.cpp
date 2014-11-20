#include "UIDisplay.h"
using namespace std;
UIDisplay::UIDisplay(SampleGrabber *Grabber_,int sizex_,int sizey_)
{
    Grabber=Grabber_;
    if(!al_init() or !al_init_primitives_addon() or !al_install_mouse())
    {
        cout << "failed to initialize allegro!\n";
        exit(-1);
    }
    al_install_keyboard();
    al_set_new_display_flags(ALLEGRO_RESIZABLE);
    sizex=sizex_;
    sizey=sizey_;
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
    al_init_font_addon();
    al_init_ttf_addon();
    font =al_load_ttf_font("FreeSans.ttf",sizex/90,0);
    t1=thread(UIThreadWrapper,this);
}

void UIDisplay::UIloop()
{
    while(true)
    {
        //Grab new stuff from SampleGrabber
        while(Grabber->sBuffer.size())
        {
            samples.push_back(Grabber->sBuffer.pop_front());
        }
        while(times.size()>samples.size())
        {
            times.push_back(Grabber->tBuffer.pop_front());
        }
        updateScreen();
    }
}
void UIDisplay::updateScreen()
{
    al_clear_to_color(al_map_rgb(0,0,0));
    int j=sizex;
    if (j>samples.size()) j=samples.size();
    for(int i=0;i<j-1;i++)
    {
        al_draw_line(i,samples[samples.size()-j+i],i+1,samples[samples.size()-j+i+1],al_map_rgb(127,0,127),1);
    }
    al_flip_display();
    
}
void UIThreadWrapper(UIDisplay *UID)
{
    UID->UIloop();
}
UIDisplay::~UIDisplay()
{
}

