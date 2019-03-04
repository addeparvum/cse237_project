from imgproc import *

# Create a camera
cam = Camera(160, 120)

# use the camera's width and height to set the viewer size
view = Viewer("Blob finding", cam.width, cam.height)


while handleEvents():
	# x and y position accumulators
	acc_x = 0
	acc_y = 0

	# number of pixels accumulated
	acc_count = 0

	# grab an image from the camera
	image = cam.grabImage()

	# iterate over every pixel
	for x in range(0, image.width):
		for y in range(0, image.height):
			# get the value of the current pixel
			red, green, blue = image[x, y]

			# check if the red intensity is greater than the green and blue
			if red > green and red > blue:
				# add the x and y of the found pixel to the accumulators'
				acc_x += x
				acc_y += y
				# increment the accumulated pixels' count
				acc_count += 1
				# colour pixels which pass the test black
				image[x, y] = 0, 0, 0

	# check the count accumulator is greater than zero, to avoid dividing by zero
	if acc_count > 0:
		# calculate the mean x and y positions
		mean_x = acc_x / acc_count
		mean_y = acc_y / acc_count
		
		# draw a small cross in red at the mean position
		image[mean_x + 0, mean_y - 1] = 255, 0, 0
		image[mean_x - 1, mean_y + 0] = 255, 0, 0
		image[mean_x + 0, mean_y + 0] = 255, 0, 0
		image[mean_x + 1, mean_y + 0] = 255, 0, 0
		image[mean_x + 0, mean_y + 1] = 255, 0, 0

	#display the image on the viewer
	view.displayImage(image)

