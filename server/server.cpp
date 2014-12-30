#include "SampleGrabber.h"
#include <thread>
#include "UIDrawer.h"
#include <unistd.h>
using namespace std;
int main(int argc, char *argv[])
{
    SampleGrabber Grabber(3490,3491);
    UIDrawer Drawer(&Grabber);
    Drawer.loop();
    
}
