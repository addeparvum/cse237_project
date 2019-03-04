from imgproc import *

cam = Camera(160, 120)
view = Viewer(cam.width, cam.height, "Face detection")

while True:
	image = cam.grabImage()

	faces = image.detectFaces()

	for face in faces:
		# extract the x, y, width and height from the tuple
		x, y, width, height = face

		# use this information to draw an orange rectangle around the face
		image.drawRect(x, y, width, height, 128, 0, 255)

	view.displayImage(image)
