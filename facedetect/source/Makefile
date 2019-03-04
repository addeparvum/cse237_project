all: camera.o image.o viewer.o util.o
	gcc -shared -Wall -g -ggdb -O2 -Wl,-soname,libimgproc.so,-z,defs -o libimgproc.so camera.o image.o viewer.o util.o -lpython2.7 -lSDLmain -lSDL -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_objdetect
	cp libimgproc.so /usr/lib/libimgproc.so
	cp imgproc.py /usr/local/lib/python2.7/dist-packages/imgproc.py
	chmod 755 /usr/local/lib/python2.7/dist-packages/imgproc.py

%.o: %.c 
	gcc -std=c99 -Wall -fPIC -g -ggdb -O2 -c -o $@ $< -I/usr/include/python2.7/ -I/usr/include/opencv/
