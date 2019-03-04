from imgproc import *

# robot control module
import i2c


cam = Camera(160, 120)
view = Viewer("Chase", cam.width, cam.height)

# create the robot control
robot = i2c.I2C()


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


		# extract the centre of the largest blob
		target_x = (x + x + width) / 2.0

		# normalise to the range +0.0 to +1.0
		target_direction = target_x / image.width

		# range -0.5 to +0.5
		target_direction -= 0.5

		# range -1.0 to +1.0
		target_direction *= 2.0

		# scale factor for the speed
		speed_scale = 75.0
		# constant added forward speed
		speed_const = 25.0

		# set the speeds
		left_speed = speed_const + (target_direction * speed_scale)
		right_speed = speed_const - (target_direction * speed_scale)

		# drive the motors
		robot.setSpeeds(left_speed, right_speed)

	# display the image copy
	view.displayImage(image_copy)


# finally, stop driving the motors
robot.setSpeeds(0, 0)

