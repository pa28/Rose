# Rose -- A GUI Library Primarily Targeting Raspberry Pi On Small Displays.
I have started this project mainly to bring my
personal desires to the great features available
from [HamClock](https://www.clearskyinstitute.com/ham/HamClock/).
My advice is that you start there, then check out
my design goals to see if this project may provide
some benefit to you. I am not here to criticize HamClock,
only to bring a slightly different vision to packaging
the features. It has taken on a life of its own as I looked
into the various small environment GUI systems that are
available; I finally decided to write my own.

## Why Write a new GUI Library

The short answer is because none of the existing available alternatives
met all my needs. The long answer is a bit more complicated. I may get 
to writing all about that some time, but for now here is part of the
story.

I started out porting [HamClock](https://www.clearskyinstitute.com/ham/HamClock/)
to Raspberry Pi C++ using [NanoGui](https://nanogui.readthedocs.io/en/latest/).
You can read about that experience in my [other repository GuiPi](https://github.com/pa28/GuiPi).
Eventually most of what is there will end up here. Ultimately though I was not
satisfied with NanoGui. It is a great piece of work and impressive, but not
close enough to what I wanted. So here we are.

# Installing

See [Installing](https://github.com/pa28/Rose/blob/main/docs/Installing.md) documentation.

# Building

Rose is primarily aimed at a Raspberry Pi with a small display. My personal
target environment is a Pi 3B in a SmartyPi case with the Raspberry Pi 7 inch
touchscreen display.

My development environment is Linux (Linux Mint at the moment) using CLion as my IDE.
So everything should work on any Linxu distribution, but certainly distributions which
are like Debian.

A also have a Pi 4B with 8GB of memory I use to generate packages which I host on
[GemFury](https://gemfury.com/). I may replace my Pi 3B with a Pi 4B 2GB model.

## Dependencies

## TL;DR 
`sudo apt install cmake libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libcgicc-dev libsoci-dev libsqlite3-dev libcurl4-openssl-dev libcurlpp-dev`

### CMake 3

### C++17 Compiler moving to C++20 when available on Raspberry Pi OS

### [SDL2](https://www.libsdl.org/) Using packages available in the respective OS repositories
* SDL is used as the graphics drawing package.
* SDL2_IMAGE is required for image manipulation.
* SDL2TTF is required for font manipulation.
* The version of SDL2 distributed with Raspberry Pi OS needs to be modified to change the following lines:
    *
    ```
    /* Note: If you modify this list, update SDL_GetPixelFormatName() */
    enum
    ```
    to:
    ```
    /* Note: If you modify this list, update SDL_GetPixelFormatName() */
    typedef enum
    ```
    * and
    ```
            SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ')
    };
    ```
    to:
    ```
            SDL_DEFINE_PIXELFOURCC('O', 'E', 'S', ' ')
    } SDL_PixelFormatEnum;
    ```

### [CURLPP](http://www.curlpp.org/)
* CURL is used for accessing assets on the WWW.

### [SOCI](http://soci.sourceforge.net/) with sqlite3 plugin
* Database access for:
    * Application settings.

### Other tidbits 

#### Doxygen with graphvis, if you want to build the Doxygen.

`apt install graphviz doxygen`

#### High Readability Font
https://www.brailleinstitute.org/freefont

#### Doxygen tutorial
https://vicrucann.github.io/tutorials/quick-cmake-doxygen/

#### Merging RGB and A from two textures
https://forums.libsdl.org/viewtopic.php?p=48648
