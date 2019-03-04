from imgproc import *


cam = Camera(160, 120)
view = Viewer("Blob detection", cam.width, cam.height)


while handleEvents():
	image = cam.grabImage()

	# create a copy of the image
	image_copy = image.copy()

	# test the original image against the key
	image.chromaKey(160, 64, 64, 96)

	# detect blobs in the image
	blobs = image.detectBlobs()

	# our current biggest
	largest_blob = None

	# the current biggest's area
	largest_blob_area = 0

	for blob in blobs:
		# extract the blobs information
		x, y, width, height, area = blob

		# check if the area is greater than the largest blob's
		if area > largest_blob_area:
			# area is greater, so the current blob is the new largest
			largest_blob = blob

			# set the largest area to the current blobs area
			largest_blob_area = area

	# check the largest_blob is set
	if largest_blob is not None:
		# extract largest blobs information
		x, y, width, height, area = largest_blob

		# draw a purple rectangle around it on the image copy
		image_copy.drawRect(x, y, width, height, 128, 0, 255)

	# display the image copy
	view.displayImage(image_copy)

