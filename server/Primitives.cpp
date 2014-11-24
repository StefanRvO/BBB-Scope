#include "Primitives.h"
void drawFilledCircle(SDL_Renderer* renderer, int x0, int y0, int radius)
{ //Uses the midpoint circle algorithm to draw a filled circle
  //https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    int x=radius;
    int y=0;
    int radiusError=1-x;
    while(x>=y)
    {
        SDL_RenderDrawLine(renderer,x+x0,y+y0,-x+x0,y+y0);
        SDL_RenderDrawLine(renderer,y+x0,x+y0,-y+x0,x+y0);
        SDL_RenderDrawLine(renderer,-x+x0,-y+y0,x+x0,-y+y0);
        SDL_RenderDrawLine(renderer,-y+x0,-x+y0,y+x0,-x+y0);
        y++;
        if(radiusError<0) radiusError+=2*y+1;
        else
        {
            x--;
            radiusError+=2*(y-x+1);
        }
    }
}
void drawCircle(SDL_Renderer* renderer, int x0, int y0, int radius)
{ //Uses the midpoint circle algorithm to draw a filled circle
  //https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    int x=radius;
    int y=0;
    int radiusError=1-x;
    while(x>=y)
    {
        SDL_RenderDrawPoint(renderer,x+x0,y+y0);
        SDL_RenderDrawPoint(renderer,y+x0,x+y0);
        SDL_RenderDrawPoint(renderer,-x+x0,y+y0);
        SDL_RenderDrawPoint(renderer,-y+x0,x+y0);
        SDL_RenderDrawPoint(renderer,-x+x0,-y+y0);
        SDL_RenderDrawPoint(renderer,-y+x0,-x+y0);
        SDL_RenderDrawPoint(renderer,x+x0,-y+y0);
        SDL_RenderDrawPoint(renderer,y+x0,-x+y0);
        y++;
        if(radiusError<0) radiusError+=2*y+1;
        else
        {
            x--;
            radiusError+=2*(y-x+1);
        }
    }
}
