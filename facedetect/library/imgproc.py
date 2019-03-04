''' This is the wrapper module for the camera and image processing functionality, provides low
	level pixel access, as well as high level functions to detect blobs and faces in an image.'''


# handles the interface to the C library
from ctypes import *


# import and disable cyclic garbage collection
# this solves our segfault + crash but turning it off creates a slow memory leak
import gc
gc.disable()


# load the image processing shared object
lib = cdll.LoadLibrary("libimgproc.so")



# TEST
lib.testFunction.restype = py_object

def testFunction():
	my_list = lib.testFunction()
	print(my_list)
	return my_list




# assign function result types
lib.imgGetMemoryView.restype = py_object
lib.imgDetectFaces.restype = py_object
lib.imgDetectBlobs.restype = py_object
lib.devImageFromBuffer.restype = c_void_p
lib.imgCopy.restype = c_void_p


# initialise SDL for use by the various library functions
lib.viewInit()


''' Delay for the number of milliseconds given in the first argument '''
def delay(msec):	
	lib.delay(msec)


''' Continually poll for events, returns True on success, and False on quit '''
def handleEvents():
	if lib.handleEvents() == 1:
		return True
	else:
		return False


''' Viewer class, handles the creation and manipulation of a window, you may only have
one window open at a time '''
class Viewer:
	''' Open an SDL window, ready to display an image '''
	def __init__(self, title, width, height):
		self.width = width
		self.height = height
		self.c_ptr = lib.viewOpen(title, width, height)


	''' Display an image in the window '''
	def displayImage(self, image):
		lib.viewDisplayImage(self.c_ptr, image.c_ptr)
		
		
	''' Close the view '''
	def __del__(self):
		lib.viewClose(self.c_ptr)


''' Image is a container for image data, with pixel level access as well as high level
functionality. '''
class Image:
	''' Initialises an image based on a C pointer '''
	def __init__(self, image_ptr):
		self.c_ptr = image_ptr  # Pointer to the C image object		
		self.view   = lib.imgGetMemoryView(self.c_ptr)
		self.width  = lib.imgGetWidth(self.c_ptr)
		self.height = lib.imgGetHeight(self.c_ptr)


	''' Create a copy of an image container and its data, and return it '''
	def copy(self):
		return Image(lib.imgCopy(self.c_ptr))


	''' Draw a unfilled coloured rectangle on an image '''
	def drawRect(self, x, y, w, h, r, g, b):
		lib.imgDrawRect(self.c_ptr, x, y, w, h, r, g, b)


	''' Test the image against a Chroma key, setting non-passing pixels to black '''
	def chromaKey(self, r_key, g_key, b_key, threshold):
		lib.imgChromaKey(self.c_ptr, r_key, g_key, b_key, threshold)


	''' Detect blobs in the image, returning a list of 5 component tuples containing the x, y, 
		width and height of each blobs' rectangle bounds, and the number of pixels the blob
		covers '''
	def detectBlobs(self):
		return lib.imgDetectBlobs(self.c_ptr)


	''' Detect faces in the image, returning a list of tuples of the x, y, width and height of
		the rectangle bounds of the discovered face '''
	def detectFaces(self):
		return lib.imgDetectFaces(self.c_ptr)


	''' Get a tuple of the red, green and blue value of a pixel.  Co-ordinates are provided
		as a tuple of their x and y position.  example "red, green, blue = img[x, y]" will set
		red, green and blue to their respective intensities of the pixel '''
	def __getitem__(self, index):
		x, y = index
		i = ((y * self.width) + x) * 3
		r = ord( self.view[i + 2] )
		g = ord( self.view[i + 1] )
		b = ord( self.view[i + 0] )
		return r, g, b
	
	
	''' Set the value of a pixel at a given co-ordinate provided in the tuple 'key'.  Key is a
		tuple of the x and y position, and value is a 3 component tuple of the red, green and
		blue intensities.  example. "img[64, 32] = 255, 128, 0" sets the pixel at position x: 64
		and y: 32 to a bright orange colour. '''
	def __setitem__(self, key, value):
		x, y = key
		r, g, b = value
		
		i = ((y * self.width) + x) * 3
		
		self.view[i + 2] = chr(r)
		self.view[i + 1] = chr(g)
		self.view[i + 0] = chr(b)


	''' Handles the destruction of the image data '''
	def __del__(self):
		lib.imgFree(self.c_ptr)
		self.view = None


''' Camera capture device, interfaces with a webcam allowing the user to grab images '''
class Camera:
	''' Initialise the camera, setting the size of the capture to 'width' pixels wide and
		'height' pixels high '''
	def __init__(self, width, height):
		# default try and open the most likely video device
		self.device = lib.devOpen("/dev/video0")
		
		# set the format of the video device
		lib.devSetFormat(self.device, width, height)
		
		# get the actual width and height of the camera capture
		self.width = lib.devGetCaptureWidth(self.device)
		self.height = lib.devGetCaptureHeight(self.device)
	
	
	''' Grabs a single image from the camera '''
	def grabImage(self):
		# dequeue and requeue several frames, we only want the most recent
		buffer_index = lib.devDequeueBuffer(self.device)
		lib.devEnqueueBuffer(self.device, buffer_index)
		buffer_index = lib.devDequeueBuffer(self.device)
		lib.devEnqueueBuffer(self.device, buffer_index)
		buffer_index = lib.devDequeueBuffer(self.device)
		lib.devEnqueueBuffer(self.device, buffer_index)


		# dequeue the buffer for the image we want
		# create an image from the returned C pointer
		buffer_index = lib.devDequeueBuffer(self.device)
		image = lib.devImageFromBuffer(self.device, buffer_index)
		lib.devEnqueueBuffer(self.device, buffer_index)

		return Image( image)
	
	
	''' Free the camera capture device '''
	def __del__(self):
		# close the handle to the device
		lib.devClose(self.device)



