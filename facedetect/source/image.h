#ifndef _IMAGE_H_
#define _IMAGE_H_


#include <stdint.h>

#include <Python.h>

#include <SDL/SDL.h>

#include "cv.h"


struct Image {
	unsigned int width;
	unsigned int height;
	uint8_t *data;
	uint8_t *mem_ptr; // pointer for memory deallocation
	
	// container formats
	PyObject * mem_view;
	IplImage * ipl_image;
	SDL_Surface * sdl_surface;
};


// get the width and height of an image
unsigned int imgGetWidth(struct Image * img);
unsigned int imgGetHeight(struct Image * img);

// create a copy of the image
struct Image * imgCopy(struct Image * image);

// draw a rectangle in the pixel data
void imgDrawRect(struct Image * image, int x, int y, unsigned int w, unsigned int h, unsigned int r, unsigned int g, unsigned int b);

// test against a chroma key, set pixel white on pass
void imgChromaKey(struct Image * image, unsigned int key_r, unsigned int key_g, unsigned int key_b, unsigned int threshold);

// detect blobs in the image
PyObject * imgDetectBlobs(struct Image * image);

// detect faces in the image, return as a list of tuples of x,y,w,h rects
PyObject * imgDetectFaces(struct Image * img);

// get the python memoryview for this object for direct access, only used by the python wrapper
PyObject * imgGetMemoryView(struct Image * img);
// get the iplimage in an OpenCV compatible format for extension
PyObject * imgGetOpenCV(struct Image * img);

// free the image and it's associated data
void imgFree(struct Image *img);


#endif // _IMAGE_H_
