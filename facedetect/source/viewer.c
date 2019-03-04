#include "viewer.h"

#include <malloc.h>
#include <SDL/SDL.h>


void viewInit()
{
	// initialise SDL ready for use
	SDL_Init(SDL_INIT_VIDEO);
}


struct Viewer * viewOpen(const char * title, unsigned int width, unsigned int height)
{	
	// set up the view
	struct Viewer * view = malloc(sizeof(*view));
	if(view == NULL){
		fprintf(stderr, "Could not allocate memory for view\n");
		return NULL;
	}
	
	// initialise the screen surface
	view->screen = SDL_SetVideoMode(width, height, 24, SDL_SWSURFACE);
	if(view == NULL){
		fprintf(stderr, "Failed to open screen surface\n");
		return NULL;
	}

	// set the window title
	SDL_WM_SetCaption( title, 0);
	
	// return the completed view object
	return view;
}


void viewClose(struct Viewer * view)
{
	// free the screen surface
	SDL_FreeSurface(view->screen);
	// free the view container
	free(view);
	
	// quit SDL
	//SDL_Quit();
	return;
}


// take an image and display it on the view
void viewDisplayImage(struct Viewer * view, struct Image * image)
{
	// Blit the image to the window surface
	SDL_BlitSurface(image->sdl_surface, NULL, view->screen, NULL);
	
	// Flip the screen to display the changes
	SDL_Flip(view->screen);
}
