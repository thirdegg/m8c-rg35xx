# m8c port rg35xx

## Introduction

This is the port of the [m8c](https://github.com/laamaa/m8c) project for `rg35xx`.
Graphics interaction has been rewritten to use `SDL1.2` calls, `SDL_gfx` (`stock`, `Garlic OS` does not support `SDL2`).
The code for joystick interaction has been removed, `j2k.so` is used for input translation.
The code for interacting with `libserialport` has been removed (`stock`, `Garlic OS` does not include the `snd_usb_audio` kernel module).
All interaction occurs through `libusb`, M8 command reception has been rewritten to be asynchronous because synchronous calls caused audio glitches.

-------

## Building

```bash
# Clone toolchain
git clone git@github.com:edemirkan/rg35xx-toolchain.git toolchain
cd toolchain

# Run container with toolchain
mkdir workspace
make shell

# Clone project
git clone git@github.com:thirdegg/m8c-rg35xx m8c-rg35xx
cd m8c-rg35xx

# Clone and build SDL_gfx
git clone git@github.com:ferzkopp/SDL_gfx.git deps/SDL_gfx
cd deps/SDL_gfx
./configure --host=arm-linux --enable-mmx=no --enable-shared=no
make
cd -

# Clone and build libusb
git clone git@github.com:libusb/libusb.git deps/libusb
cd deps/libusb
./configure --host=arm-linux 
make
cd -

# Build project
make
exit
```

## Running

1) Create a folder named `m8c/` on the flash drive in `Roms/APPS/`.
2) Copy the executable file `m8c` from the `workspace/` folder into it.
3) Create a file `m8c.sh` in the `Roms/APPS/` folder with the following contents:
```sh
#!/bin/sh
PROG_DIR=$(dirname "$0")/m8c
cd ${PROG_DIR}
HOME=${PROG_DIR}
LD_PRELOAD=./j2k.so ./m8c &>log.txt
sync
```
4) Somewhere, find a `j2k.so` file compatible with the system and move it to `APPS/m8c/`. I used one from the compiled LittleGP Tracker project (I couldn't find links to the project for building j2k.so).
5) Insert the flash drive into the console, connect it via an `OTG` controller with M8.
6) Enjoy!

## Disclaimer
THE CODE MAY HAVE MEMORY LEAKS! Most of the code was written in haste, and debugging takes an unreasonably long time. Use at your own risk and I will be glad to receive your merge requests with fixes.