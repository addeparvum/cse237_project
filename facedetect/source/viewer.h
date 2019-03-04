#ifndef _VIEWER_H_
#define _VIEWER_H_

#include "image.h"

#include <SDL/SDL.h>


struct Viewer {
	unsigned int width;
	unsigned int height;
	SDL_Surface *screen;
};


void viewInit();

struct Viewer * viewOpen(const char *title, unsigned int width, unsigned int height);
void viewClose(struct Viewer * view);
void viewDisplayImage(struct Viewer * view, struct Image * image);


#endif // _VIEWER_H_
