CC = g++
ifeq ($(shell uname 2>/dev/null), Darwin)
CFLAGS = -g -DGL_GLEXT_PROTOTYPES -DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED -DOSX -Wno-deprecated-register -Wno-deprecated-declarations -Wno-shift-op-parentheses
INCFLAGS = -I./glm-0.9.7.1 -I/opt/local/include -I./include/
LDFLAGS = -framework GLUT -framework OpenGL -L/opt/homebrew/lib \
  -L"/System/Library/Frameworks/OpenGL.framework/Libraries" \
  -lGL -lGLU -lm -lstdc++ -lfreeimage
else
CFLAGS = -g -DGL_GLEXT_PROTOTYPES 
INCFLAGS = -I./glm-0.9.7.1 -I./include/ -I/usr/X11R6/include -I/sw/include \
		-I/usr/sww/include -I/usr/sww/pkg/Mesa/include
LDFLAGS = -L/usr/X11R6/lib -L/sw/lib -L/usr/sww/lib -L./lib/nix/ \
		-L/usr/sww/bin -L/usr/sww/pkg/Mesa/lib -lGLEW -lglut -lGLU -lGL -lX11 -lfreeimage
endif

RM = /bin/rm -f 
all: raycast
raycast: main.o readfile.o variables.o raytrace.o
	$(CC) $(CFLAGS) -o raycast main.o readfile.o variables.o raytrace.o $(INCFLAGS) $(LDFLAGS) 
main.o: main.cpp readfile.h variables.h raytrace.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c main.cpp
readfile.o: readfile.cpp readfile.h variables.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c readfile.cpp
variables.o: variables.cpp variables.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c variables.cpp
raytrace.o: raytrace.cpp raytrace.h
	$(CC) $(CFLAGS) $(INCFLAGS) -c raytrace.cpp
clean: 
	$(RM) *.o raycast *.png 