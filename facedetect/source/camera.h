#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "image.h"

#include <linux/videodev2.h>
#include <Python.h>


struct Buffer {
	struct v4l2_buffer buf;
	void * start;
};


// device container
struct Device {
	char *name;
	int handle;
	struct Buffer * buffers;
	unsigned int n_buffers;
	unsigned int img_width;
	unsigned int img_height;
};


struct Device * devOpen(char *dev_name);
void devSetFormat(struct Device *dev, unsigned int width, unsigned int height);

unsigned int devGetCaptureWidth(struct Device *dev);
unsigned int devGetCaptureHeight(struct Device *dev);

unsigned int devDequeueBuffer(struct Device *dev);
struct Image * devImageFromBuffer(struct Device *dev, unsigned int buffer);
void devEnqueueBuffer(struct Device *dev, unsigned int buffer);

void devClose(struct Device *dev);


#endif // _CAMERA_H_
