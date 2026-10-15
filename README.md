# unikraft-lib-opencv

> WIP

A Unikraft library for [opencv-mobile](https://github.com/nihui/opencv-mobile).

The OpenCV sources are fetched and prepared by the build system itself (see `Makefile.uk`).

`kraft build` downloads the `opencv-mobile` release archive, applies the
patches in `patches/`, runs OpenCV's CMake configure step to generate the
headers the build needs, copies additional files and then compiles.

