#Set all your object files (the object files of all the .c files in your project, e.g. main.o my_sub_functions.o )
OBJ = src/main.o src/serial.o src/slip.o src/command.o src/render.o src/ini.o src/config.o src/input.o src/fx_cube.o src/usb.o src/audio.o src/usb_audio.o src/ringbuffer.o src/inprint2.o src/SDL2_compat.o

#Set any dependant header files so that if they are edited they cause a complete re-compile (e.g. main.h some_subfunctions.h some_definitions_file.h ), or leave blank
DEPS = src/serial.h src/slip.h src/command.h src/render.h src/ini.h src/config.h src/input.h src/fx_cube.h src/audio.h src/ringbuffer.h src/inline_font.h  src/SDL2_compat.h

#Any special libraries you are using in your project (e.g. -lbcm2835 -lrt `pkg-config --libs gtk+-3.0` ), or leave blank
INCLUDES = -L/root/workspace/m8c-rg35xx/deps/libusb/libusb/.libs -L/root/workspace/m8c-rg35xx/deps/SDL_gfx.libs -lSDL_gfx -lusb-1.0 -lSDL

#Set any compiler flags you want to use (e.g. -I/usr/include/somefolder `pkg-config --cflags gtk+-3.0` ), or leave blank
local_CFLAGS = -Wall -O2 -pipe -I. -I/root/workspace/m8c-rg35xx/deps/libusb/libusb/ -I/root/workspace/m8c-rg35xx/deps/SDL_gfx $(shell pkg-config --cflags sdl) -DUSE_LIBUSB=1 -DDEBUG_MSG=1

#Set the compiler you are using ( gcc for C or g++ for C++ )
#CC = arm-buildroot-linux-gnueabihf-gcc
CC = arm-linux-gcc

#Set the filename extensiton of your C files (e.g. .c or .cpp )
EXTENSION = .c

SOURCE_DIR = src/

#define a rule that applies to all files ending in the .o suffix, which says that the .o file depends upon the .c version of the file and all the .h files included in the DEPS macro.  Compile each object file
%.o: %$(EXTENSION) $(DEPS)
	$(CC) -c -o $@ $< $(local_CFLAGS)

#Combine them into the output file
#Set your desired exe output file name here
m8c: $(OBJ)
	$(CC) -o $@ $^ $(local_CFLAGS) $(INCLUDES)

#Cleanup
.PHONY: clean

clean:
	rm -f src/*.o *~ m8c