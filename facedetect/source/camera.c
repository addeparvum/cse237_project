#include "image.h"
#include "camera.h"

#include <Python.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>             /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>


static void errno_exit(const char *s)
{
        fprintf (stderr, "%s error %d, %s\n",
			s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}


static int xioctl(struct Device *dev, int request, void *arg)
{
        int r;

        do r = ioctl (dev->handle, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}


// routine to initialise memory mapped i/o on the device
static void init_mmap(struct Device *dev)
{
	struct v4l2_requestbuffers req;

	memset (&(req), 0, sizeof (req));

	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (dev, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s does not support "
					"memory mapping\n", dev->name);
			exit (EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		fprintf (stderr, "Insufficient buffer memory on %s\n",
				 dev->name);
		exit (EXIT_FAILURE);
	}

	// allocate memory for the buffers
	dev->buffers = calloc (req.count, sizeof (*(dev->buffers)));

	if (!dev->buffers) {
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}

	for (dev->n_buffers = 0; dev->n_buffers < req.count; dev->n_buffers++) {
		struct v4l2_buffer buffer;

		memset (&(buffer), 0, sizeof (buffer));

		buffer.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory      = V4L2_MEMORY_MMAP;
		buffer.index       = dev->n_buffers;

		if (-1 == xioctl (dev, VIDIOC_QUERYBUF, &buffer)){
			errno_exit ("VIDIOC_QUERYBUF");
		}

		// copy the v4l2 buffer into the device buffers
		dev->buffers[dev->n_buffers].buf = buffer;
		// memory map the device buffers
		dev->buffers[dev->n_buffers].start = 
			mmap( NULL, // start anywhere
				  buffer.length,
				  PROT_READ | PROT_WRITE, // required
				  MAP_SHARED, // recommended
				  dev->handle,
				  buffer.m.offset
			);

		if (MAP_FAILED == dev->buffers[dev->n_buffers].start){
			errno_exit ("mmap");
		}
	}
}


// returns an index to the dequeued buffer
unsigned int devDequeueBuffer(struct Device *dev)
{
	while(1){
		fd_set fds;
		struct timeval tv;
		int r;

		
		FD_ZERO (&fds);
		FD_SET (dev->handle, &fds);

		// Timeout.
		tv.tv_sec = 20;
		tv.tv_usec = 0;

		
		r = select (dev->handle + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno){
				continue;
			}
			errno_exit ("select");
		}

		if (0 == r) {
			fprintf (stderr, "select timeout\n");
			exit (EXIT_FAILURE);
		}

		
		// read the frame
		struct v4l2_buffer buffer;
		memset (&(buffer), 0, sizeof (buffer));

		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		
		// dequeue a buffer
		if (-1 == xioctl (dev, VIDIOC_DQBUF, &buffer)) {
			switch (errno) {
				case EAGAIN:
					continue;

				case EIO:
					/* Could ignore EIO, see spec. */
					/* fall through */

				default:
					errno_exit ("VIDIOC_DQBUF");
			}
		}
		assert (buffer.index < dev->n_buffers);

		// return the buffer index handle to the buffer
		return buffer.index;
	}
}


// enqueue a given device buffer to the device
void devEnqueueBuffer(struct Device *dev, unsigned int buffer_index)
{
	// enqueue a given buffer by index
	if(-1 == xioctl(dev, VIDIOC_QBUF, &(dev->buffers[buffer_index].buf) )){
		errno_exit("VIDIOC_QBUF");
	}

	return;
}
	

// grab an image, convert to RGB and store in an image container object
struct Image * devImageFromBuffer(struct Device *dev, unsigned int buffer_index)
{
	// allocate for the image container
	struct Image * image = malloc(sizeof(*image));
	if(image == NULL){
		fprintf(stderr, "Failed to allocate memory for image container\n");
		exit(EXIT_FAILURE);
	}
	
	// set the image width and height to that of the devices
	image->width = dev->img_width;
	image->height = dev->img_height;


	// allocate for image data, 3 byte per pixel, aligned to an 8 byte boundary
	image->mem_ptr = malloc(image->width * image->height * 3 + 8);
	if(image->mem_ptr == NULL){
		fprintf(stderr, "Memory allocation of image data failed\n");
		exit(EXIT_FAILURE);
	}

	// make certain it is aligned to 8 bytes
	unsigned int remainder = ((size_t)image->mem_ptr) % 8;
	if(remainder == 0){
		image->data = image->mem_ptr;
	} else {
		image->data = image->mem_ptr + (8 - remainder);
	}

	
	// copy data from the buffer to the image data, convert to rgb
	uint8_t *buffer_ptr = dev->buffers[buffer_index].start;
	uint8_t *image_ptr = image->data;
	
	// iterate 2 pixels at a time, so 4 bytes for YUV and 6 bytes for RGB
	for(uint32_t i = 0, j = 0; i < (image->width * image->height * 2); i+=4, j+=6){
		uint8_t *buffer_pos = buffer_ptr + i;
		uint8_t *image_pos = image_ptr + j;
		
		// USED TO BE UNSIGNED, THINK IT MAY BE UNDERFLOWING CAUSING ARTIFACTS AT NEAR BLACK

		// YCbCr to RGB conversion (from: http://www.equasys.de/colorconversion.html);
		int y0 = buffer_pos[0];
		int cb = buffer_pos[1];
		int y1 = buffer_pos[2];
		int cr = buffer_pos[3];
		int r;
		int g;
		int b;

		// first RGB
		r = y0 + ((357 * cr) >> 8) - 179;
		g = y0 - (( 87 * cb) >> 8) +  44 - ((181 * cr) >> 8) + 91;
		b = y0 + ((450 * cb) >> 8) - 226;
		// clamp to 0 to 255
		image_pos[2] = r > 254 ? 255 : (r < 0 ? 0 : r);
		image_pos[1] = g > 254 ? 255 : (g < 0 ? 0 : g);
		image_pos[0] = b > 254 ? 255 : (b < 0 ? 0 : b);
		
		// second RGB
		r = y1 + ((357 * cr) >> 8) - 179;
		g = y1 - (( 87 * cb) >> 8) +  44 - ((181 * cr) >> 8) + 91;
		b = y1 + ((450 * cb) >> 8) - 226;
		image_pos[5] = r > 254 ? 255 : (r < 0 ? 0 : r);
		image_pos[4] = g > 254 ? 255 : (g < 0 ? 0 : g);
		image_pos[3] = b > 254 ? 255 : (b < 0 ? 0 : b);
	}

	// http://stackoverflow.com/questions/5015884/exposing-a-c-string-without-copying-to-python-3-x-code
	
	// image data is filled, create the container
	Py_buffer * pybuf = malloc(sizeof(*pybuf));
	if(pybuf == NULL){
		fprintf(stderr, "Memory allocation of python buffer object failed\n");
		exit(EXIT_FAILURE);
	}

	// Fill the python info buffer
	if( PyBuffer_FillInfo(pybuf, NULL, image->data, image->width * image->height * 3, 0, PyBUF_WRITABLE) == -1){
		fprintf(stderr, "Error filling buffer info\n");
		exit(EXIT_FAILURE);
	}

	// fill info doesn't allocate this and it's needed to access via a memoryview
	pybuf->shape = malloc(1);
	if(pybuf->shape == NULL){
		fprintf(stderr, "Could not allocate single byte for pybuffer shape structure\n");
		exit(EXIT_FAILURE);
	}


	// Create a memoryview from the buffer
	image->mem_view = PyMemoryView_FromBuffer(pybuf);


	// Fill the SDL_Surface container
	image->sdl_surface = SDL_CreateRGBSurfaceFrom(
				image->data,
				image->width,
				image->height,
				24, 
				image->width * 3,
				0xff0000,
				0x00ff00,
				0x0000ff,
				0x000000
	);
	
	// check the surface was initialised
	if(image->sdl_surface == NULL){
		fprintf(stderr, "Failed to initialise RGB surface from pixel data\n");
		exit(EXIT_FAILURE);
	}


	// Create the IplImage container structure
	IplImage * iplimg = malloc(sizeof(*iplimg));
	if(iplimg == NULL){
		fprintf(stderr, "Memory allocation of iplimage container structure failed\n");
		exit(EXIT_FAILURE);
	}


	// Setup the ipl structure, need to check alignment and origin are correct
	// Also ipl expects BGR I think and image is actually RGB
	// colorModel, channelSeq, BorderMode and BorderConst are all unused by opencv
	// so don't initialise, also don't try and free them
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


	// initialise the image container's iplimg
	image->ipl_image = iplimg;


	// return the image container container
	return image;
}


unsigned int devGetCaptureWidth(struct Device *dev)
{
	return dev->img_width;
}


unsigned int devGetCaptureHeight(struct Device *dev)
{
	return dev->img_height;
}


void devSetFormat(struct Device *dev, unsigned int width, unsigned int height)
{
	//printf("Setting device format\n");

	struct v4l2_format fmt;
	unsigned int min;

	memset (&(fmt), 0, sizeof (fmt));
	
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width; 
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (-1 == xioctl (dev, VIDIOC_S_FMT, &fmt)){
		errno_exit ("VIDIOC_S_FMT");
	}

    // Note VIDIOC_S_FMT may change width and height.
	
	// Buggy driver paranoia.
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	// ONLY CHANGES IN IMAGE SIZE ARE HANDLED ATM
	// set device image size to the returned width and height.
	dev->img_width = fmt.fmt.pix.width;
	dev->img_height = fmt.fmt.pix.height;
	

	//printf("Initialising memory mapped i/o\n");
	
	// initialise for memory mapped io
	init_mmap (dev);
	
	
	// initialise streaming for capture
	enum v4l2_buf_type type;
	
	// queue buffers ready for capture
	for (unsigned int i = 0; i < dev->n_buffers; ++i) {
		// buffers are initialised, so just call the enqueue function
		devEnqueueBuffer(dev, i);		
	}
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	// turn on streaming
	if (-1 == xioctl (dev, VIDIOC_STREAMON, &type)){
		if(errno == EINVAL){
			fprintf(stderr, "buffer type not supported, or no buffers allocated or mapped\n");
			exit(EXIT_FAILURE);
		} else if(errno == EPIPE){
			fprintf(stderr, "The driver implements pad-level format configuration and the pipeline configuration is invalid.\n");
			exit(EXIT_FAILURE);
		} else {
			errno_exit ("VIDIOC_STREAMON");
		}
	}
}


// close video capture device
void devClose(struct Device *dev)
{
	//printf("Stopping camera capture\n");

	// stop capturing
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl (dev, VIDIOC_STREAMOFF, &type)){
		errno_exit ("VIDIOC_STREAMOFF");
	}


	//printf("Uninitialising device\n");

	// uninitialise the device
	for (unsigned int i = 0; i < dev->n_buffers; ++i){
		if(-1 == munmap(dev->buffers[i].start, dev->buffers[i].buf.length)){
			errno_exit("munmap");
		}
	}
	
	// free buffers
	free (dev->buffers);
	
	//printf("Closing device\n");

	// close the device
	if (-1 == close(dev->handle)){
		errno_exit ("close");
	}

	free(dev);
	dev->handle = -1;
}


// Open a video capture device
struct Device * devOpen(char *dev_name)
{
	//printf("Opening the device\n");
	
	
	// initialise the device
	struct stat st; 

	if (-1 == stat (dev_name, &st)) {
		fprintf (stderr, "Cannot identify '%s': %d, %s\n",
			dev_name, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}

	if (!S_ISCHR (st.st_mode)) {
		fprintf (stderr, "%s is no device\n", dev_name);
		exit (EXIT_FAILURE);
	}

	
	// set up the device
	struct Device *dev = malloc(sizeof(*dev));
	if(dev == NULL){
		fprintf(stderr, "Could not allocate memory for device structure\n");
		exit(EXIT_FAILURE);
	}
	
	// open the device
	dev->handle = open(dev_name, O_RDWR | O_NONBLOCK, 0);
	dev->name = dev_name;

	if (-1 == dev->handle) {
		fprintf (stderr, "Cannot open '%s': %d, %s\n",
			 dev_name, errno, strerror (errno));
		exit (EXIT_FAILURE);
	}
	
	
	//printf("Initialising device\n");
	
	
	// initialise the device
	struct v4l2_capability cap;

	// check capabilities
	if (-1 == xioctl (dev, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			fprintf (stderr, "%s is no V4L2 device\n",
					 dev->name);
			exit (EXIT_FAILURE);
		} else {
				errno_exit ("VIDIOC_QUERYCAP");
		}
	}

	// check capture capable
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		fprintf (stderr, "%s is no video capture device\n",
					 dev->name);
		exit (EXIT_FAILURE);
	}


	// check for memory mapped io
	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		fprintf (stderr, "%s does not support streaming i/o\n",
			 dev->name);
		exit (EXIT_FAILURE);
	}
	
	return dev;
}
