---
show_downloads: ["yes"]
---

### Rose - *Rendering On Small Environments*

Rose is intended, primarily as a Graphic User Interface library for
Raspberry Pi computers with a small touch screen to support application
that can run directly on the screen frame buffer `/dev/fb0` in an
*Application* or *Kiosk* mode. Rose uses [SDL2](https://www.libsdl.org/)
as the underlying graphics engine which makes it possible to run under 
desktop graphics environments (X11) and multiple OSs and distributions.

The project in the reposition builds under [Cmake](https://cmake.org/)
on Debian Linux derived distributions and produces the following 
artifacts:
* The Rose library.
* Sample programs and applications implements on Rose.
* Debian packages with:
    * The library, sample programs and application with supporting
    resources;
    * A package for developers.
