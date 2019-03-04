#include "image.h"

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include <Python.h>

#include "cv.h"
#include "highgui.h"



// function to test the list/tuple creating, was a segfault with code like this
// caused by the cyclic garbage collector.  Turning off the cyclic garbage collector
// causes a memory leak
PyObject * testFunction()
{
	PyObject * blob_list = PyList_New(1024);

	for(unsigned int i = 0; i < 1024; i++){
		printf("%u\n", i);

		PyObject * blobs = PyList_New(5);
		
		PyList_SetItem(blobs, 0, PyInt_FromLong(1));
		PyList_SetItem(blobs, 1, PyInt_FromLong(1));
		PyList_SetItem(blobs, 2, PyInt_FromLong(1));
		PyList_SetItem(blobs, 3, PyInt_FromLong(1));
		PyList_SetItem(blobs, 4, PyInt_FromLong(1));

		PyList_SetItem(blob_list, i, blobs);	
	}


	return blob_list;
}




unsigned int imgGetWidth(struct Image * img)
{
	return img->width;
}

unsigned int imgGetHeight(struct Image * img)
{
	return img->height;
}


// Create a copy of the image data, this copies code from devImageFromBuffer, might want
// to factor that out somehow
struct Image * imgCopy(struct Image * image)
{
	// allocate the copy container
	struct Image * img_cpy = malloc(sizeof(*img_cpy));
	if(img_cpy == NULL){
		return NULL;
	}
	

	// fill in some details
	img_cpy->width = image->width;
	img_cpy->height = image->height;
	
	unsigned int img_size = img_cpy->width * img_cpy->height * 3;


	// allocate for image data, 3 byte per pixel, aligned to an 8 byte boundary
	img_cpy->mem_ptr = malloc(img_size + 8);
	if(img_cpy->mem_ptr == NULL){
		fprintf(stderr, "Memory allocation of image data failed\n");
		free(img_cpy);
		return NULL;
	}

	// make certain it is aligned to 8 bytes
	unsigned int remainder = ((size_t)img_cpy->mem_ptr) % 8;
	if(remainder == 0){
		img_cpy->data = img_cpy->mem_ptr;
	} else {
		img_cpy->data = img_cpy->mem_ptr + (8 - remainder);
	}
	

	// copy the data from the source image to the copy
	img_cpy->data = memcpy( img_cpy->data, image->data, img_size);

	// we shouldn't be use any member from image from here on

	
	// create the python buffer
	Py_buffer * pybuf = malloc(sizeof(*pybuf));
	if(pybuf == NULL){
		fprintf(stderr, "Memory allocation of python buffer object failed\n");
		exit(EXIT_FAILURE);
	}

	// Fill the python info buffer
	if( PyBuffer_FillInfo(pybuf, NULL, img_cpy->data, img_size, 0, PyBUF_WRITABLE) == -1){
		fprintf(stderr, "Error filling buffer info\n");
		exit(EXIT_FAILURE);
	}

	// fill info doesn't allocate this and it's needed to access via a memoryview
	pybuf->shape = malloc(1);
	if(pybuf->shape == NULL){
		fprintf(stderr, "Could not allocate single byte for pybuffer shape structure\n");
		exit(EXIT_FAILURE);
	}

	// Create a memoryview from the buffer and assign to the copy, deallocated later by python
	img_cpy->mem_view = PyMemoryView_FromBuffer(pybuf);


	// Fill the SDL_Surface container
	img_cpy->sdl_surface = SDL_CreateRGBSurfaceFrom(
				img_cpy->data,
				img_cpy->width,
				img_cpy->height,
				24, 
				img_cpy->width * 3,
				0xff0000,
				0x00ff00,
				0x0000ff,
				0x000000
	);


	// check the surface was initialised
	if(img_cpy->sdl_surface == NULL){
		fprintf(stderr, "Failed to initialise RGB surface from pixel data\n");
		free(img_cpy->mem_ptr);
		free(img_cpy);
		return NULL;
	}


	// Create the IplImage container structure
	IplImage * iplimg = malloc(sizeof(*iplimg));
	if(iplimg == NULL){
		fprintf(stderr, "Memory allocation of iplimage container structure failed\n");
		exit(EXIT_FAILURE);
	}

	// initialise members
	iplimg->nSize = sizeof(*iplimg);
	iplimg->ID = 0;			 			  // version, always 0
	iplimg->nChannels = 3;
	iplimg->alphaChannel = 0;			  // ignored by opencv
	iplimg->depth = IPL_DEPTH_8U; 		  // uint8 pixel depth
	iplimg->dataOrder = 0; 			      // 0 = Interleaved colour channels
	iplimg->origin = 0; 				  // top left image origin (should be inline with SDL)
	iplimg->align = 8; 					  // ignored by opencv, uses widthstep instead
	iplimg->width = image->width;
	iplimg->height = image->height;
	iplimg->roi = NULL; 				  // region of interest

	iplimg->maskROI = NULL;
	iplimg->imageId = NULL; // null for opencv
	iplimg->tileInfo = NULL;

	iplimg->imageSize = image->width * image->height * iplimg->nChannels;
	iplimg->imageData = (char *)image->data;  // pointer to aligned data (check alignment)
	iplimg->widthStep = image->width * iplimg->nChannels; // size of an aligned row in bytes
	iplimg->imageDataOrigin = (char *)image->mem_ptr; // pointer to unaligned image data for deallocation


	// initialise the copy's ipl_image
	img_cpy->ipl_image = iplimg;


	// return the copy
	return img_cpy;
}


// Draw a pixel without locking the SDL surface, probably unnecessary anyway, shouldn't matter
// for SW surface and we access it directly via python anyway
static void imgDrawPixelNoLock(struct Image * image, unsigned int x, unsigned int y, unsigned int r, unsigned int g, unsigned int b)
{
	unsigned int offset = (x + y * image->width) * 3;
	image->data[offset + 0] = b;
	image->data[offset + 1] = g;
	image->data[offset + 2] = r;
}


// draw a rectangle clipped to the image size, this misses the bottom right element
void imgDrawRect(struct Image * image, int x, int y, unsigned int w, unsigned int h, unsigned int r, unsigned int g, unsigned int b)
{
	
	
	if(x > image->width) return;
	if(y > image->width) return;
	unsigned int x0 = x < 0 ? 0 : x;
	unsigned int y0 = y < 0 ? 0 : y;
	unsigned int x1 = (x + w) > image->width ? image->width : (x + w);
	unsigned int y1 = (y + h) > image->height ? image->height : (y + h);

	// lock the surface if necessary
	if(SDL_MUSTLOCK(image->sdl_surface)){
		SDL_LockSurface(image->sdl_surface);
	}

	// draw lines
	unsigned tx, ty;
	for(tx = x0, ty = y0; tx < x1; tx++) imgDrawPixelNoLock(image, tx, ty, r, g, b);
	for(tx = x0, ty = y1; tx < x1; tx++) imgDrawPixelNoLock(image, tx, ty, r, g, b);
	for(ty = y0, tx = x0; ty < y1; ty++) imgDrawPixelNoLock(image, tx, ty, r, g, b);
	for(ty = y0, tx = x1; ty < y1; ty++) imgDrawPixelNoLock(image, tx, ty, r, g, b);


	// unlock surface
	if(SDL_MUSTLOCK(image->sdl_surface)){
		SDL_UnlockSurface(image->sdl_surface);
	}
}


// Compare against a given key by comparing euclidean distance with the threshold
//http://www.youtube.com/watch?v=aE0F4F5WIuI
void imgChromaKey(struct Image * image, unsigned int key_r, unsigned int key_g, unsigned int key_b, unsigned int threshold)
{
	unsigned int offset;
	unsigned int r;
	unsigned int g;
	unsigned int b;
	unsigned int dist_sq;
	uint8_t * img_ptr = image->data;
	unsigned int threshold_sq = threshold * threshold;


	// calculate the euclidean distance between each point

	unsigned int x_step = 3;
	unsigned int y_step = image->width * 3;
	unsigned int img_size = image->width * image->height * 3;
	for(unsigned int x = 0; x < (image->width * 3); x+=x_step){
		for(unsigned int y = 0; y < img_size; y+=y_step){
			offset = x + y;
			r = img_ptr[offset + 2] - key_r;
			g = img_ptr[offset + 1] - key_g;
			b = img_ptr[offset + 0] - key_b;
			dist_sq = (r * r) + (g * g) + (b * b);

			// if the square distance is less than the squared threshold, we pass, set to white
			if(dist_sq < threshold_sq){
				img_ptr[offset + 2] = 255;
				img_ptr[offset + 1] = 255;
				img_ptr[offset + 0] = 255;
			}
			// otherwise set the pixel black
			else {
				img_ptr[offset + 2] = 0;
				img_ptr[offset + 1] = 0;
				img_ptr[offset + 0] = 0;
			}
		}
	}

	return;
}


struct blob {
	unsigned int min_x;
	unsigned int min_y;
	unsigned int max_x;
	unsigned int max_y;
	unsigned int n_pixels;
};


static void fill_blob(struct Image * img, unsigned int x, unsigned int y, uint8_t * blob_mask, struct blob * blob)
{
	unsigned int offset = x + y * img->width;

	if(img->data[offset * 3] != 0){
		if(blob_mask[offset] == 0){
			blob_mask[offset] = 1;
			blob->n_pixels++;
			if(x < blob->min_x) blob->min_x = x;
			if(y < blob->min_y) blob->min_y = y;
			if(x > blob->max_x) blob->max_x = x;
			if(y > blob->max_y) blob->max_y = y;

			// expand to adjacent pixels
			if( (x < img->width - 1) && (y < img->height - 1) && (x > 0) && (y > 0) ){
				fill_blob(img, x + 0, y - 1, blob_mask, blob);
				fill_blob(img, x - 1, y + 1, blob_mask, blob);
				fill_blob(img, x + 1, y + 1, blob_mask, blob);
				fill_blob(img, x + 0, y + 1, blob_mask, blob);
			}
		}
	}
}


PyObject * imgDetectBlobs(struct Image * image)
{
	uint8_t * blob_mask = calloc(image->width * image->height, 1);
	if(blob_mask == NULL){
		fprintf(stderr, "Could not allocate memory for blob mask\n");
		return NULL;
	}

	// create a new list of tuples
	PyObject * blob_list = PyList_New(0);

	// iterate over every pixel
	for(unsigned int x = 0; x < image->width; x++){
		for(unsigned int y = 0; y < image->height; y++){
			unsigned int offset = x + y * image->width;

			if(image->data[offset * 3] != 0){
				if(blob_mask[offset] == 0){
					struct blob blob = { x, y, 0, 0, 0 };

					fill_blob(image, x, y, blob_mask, &blob);
					
					// creating the PyTuple was segfaulting.  Cyclic GC has been turned off, so
					// now this has a memory leak
					PyObject * blob_tuple = PyTuple_Pack(5, 
							PyInt_FromLong(blob.min_x),
							PyInt_FromLong(blob.min_y),
							PyInt_FromLong(blob.max_x - blob.min_x),
							PyInt_FromLong(blob.max_y - blob.min_y),
							PyInt_FromLong(blob.n_pixels)
					);

					// append the blob information to the list
					PyList_Append(blob_list, blob_tuple);
				}
			}
		}
	}

	free(blob_mask);

	return blob_list;
}


// Detect faces uses the OpenCV example as a basis.  uses a hardcoded path to the libopencv-dev packages cascade
PyObject * imgDetectFaces(struct Image * image)
{
	CvHaarClassifierCascade * cascade = 0;

	const char * face_cascade_name = "/usr/share/opencv/haarcascades/haarcascade_frontalface_alt.xml";

	// create memory storage
	CvMemStorage * storage = cvCreateMemStorage(0);
	
	// load the face cascade
	cascade = (CvHaarClassifierCascade *)cvLoad(face_cascade_name, 0, 0, 0);
	if( !cascade ){
		fprintf(stderr, "Failed to load face classifier cascade\n");
		exit(EXIT_FAILURE);
	}


	// detect
	CvSeq * faces = cvHaarDetectObjects(
			image->ipl_image,
			cascade, 
			storage,
			1.1,
			2,
			CV_HAAR_DO_CANNY_PRUNING, 
			cvSize(5, 5), 
			cvSize(200,200) 
	);


	PyObject * face_list = PyList_New(faces->total);


	for(unsigned int i = 0; i < faces->total; i++){
		CvRect face_rect = *(CvRect*)cvGetSeqElem(faces, i);

		// create a tuple of length 4
		PyObject * rect_tuple = PyTuple_New(4);

		// populate the tuple
		PyTuple_SET_ITEM(rect_tuple, 0, PyInt_FromLong(face_rect.x));
		PyTuple_SET_ITEM(rect_tuple, 1, PyInt_FromLong(face_rect.y));
		PyTuple_SET_ITEM(rect_tuple, 2, PyInt_FromLong(face_rect.width));
		PyTuple_SET_ITEM(rect_tuple, 3, PyInt_FromLong(face_rect.height));

		// add the tuple to the face_list
		PyList_SetItem(face_list, i, rect_tuple);
	}
	
	// release the memory storage
	cvReleaseMemStorage(&storage);

	// return the face list
	return face_list;
}


// Get as a memoryview
PyObject * imgGetMemoryView(struct Image * image)
{
	return image->mem_view;
}


// get the image in an opencv compatible form
PyObject * imgGetOpenCV(struct Image * image)
{
	return NULL;
}



// Python will handle the memoryview deallocation
void imgFree(struct Image * image)
{
	// free the ipl image structures
	free(image->ipl_image->roi);
	free(image->ipl_image->maskROI);
	free(image->ipl_image->tileInfo);

	// free ipl image container
	free(image->ipl_image);
	image->ipl_image = NULL;

	// deallocate the sdl surface
	SDL_FreeSurface(image->sdl_surface);
	image->sdl_surface = NULL;
	
	// free the image data
	free(image->mem_ptr);
	image->mem_ptr = NULL;
	image->data = NULL;

	// free the image container
	free(image);
}

