#include "util.h"

#include <SDL/SDL.h>

#include <time.h>


// delay for a given number of milliseconds
void delay(unsigned int msec)
{
	SDL_Delay(msec);
	/*struct timespec tim, tim2;

	tim.tv_sec = msec / 1000;
	tim.tv_nsec = 1000000L * ((long)msec % 1000L);
	nanosleep(&tim, &tim2);*/
}


// Handles the events, just polls all the waiting events checking for a
// quit event.  Returns 0 if there is a quit event, otherwise 1 to continue
unsigned int handleEvents()
{
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		if(event.type == SDL_QUIT){
			return 0;
		}
		else if(event.type == SDL_KEYDOWN){
			if(event.key.keysym.sym == SDLK_ESCAPE){
				return 0;
			}
		}
	}
	// no quit event, so continue
	return 1;
}

