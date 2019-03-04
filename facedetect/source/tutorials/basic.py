from imgproc import *

# open the webcam
my_camera = Camera(320, 240)

# grab an image from the camera
my_image = my_camera.grabImage()

# iterate over ever pixel in the image by iterating over each row and each column
for x in range(0, my_image.width):
	for y in range(0, my_image.height):
		# get the value of the current pixel
		red, green, blue = my_image[x, y]

		# check if the red intensity is greater than the green
		if red > green:
			# check if red is also more intense than blue
			if red > blue:
				# this pixel is predominantly red
				# lets set it to blue
				my_image[x, y] = 0, 0, 255
	
# open a view, setting the view to the size of the captured image
my_view = Viewer("Basic image processing", my_camera.width, my_camera.height)
	
# display the image on the screen
my_view.displayImage(my_image)

# wait for 5 seconds, so we can see the image
delay(5000)
