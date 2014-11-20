#include "SampleGrabber.h"
#include "UIDisplay.h"
#include <iostream>
#include <unistd.h>
int main()
{
    SampleGrabber Grabber(3490);
    UIDisplay GUI(&Grabber,800,400);
    while(true) usleep(10000);
}
