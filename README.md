# *sk*

Skat-Konferenz is the popular german card game Skat played online combined with videoconferencing. Copyright (C) 2012 Carsten Paproth.
This program is free software and comes with ABSOLUTELY NO WARRANTY. Licensed under the terms of [GPLv3](http://www.gnu.org/licenses/).

Skat-Konferenz is written in C++ by using only portable libraries, so it should be compatible with UNIX/Linux (X11), MS Windows, and Mac OS X. Till now,
it has been tested on Xubuntu 11.10(32bit and 64bit), Xubuntu 12.04(64bit), and MS Windows 7(32bit).

This software is a work in progress, but the networking and videoconferencing part is already functional.


## Dependencies

Following portable open source libraries are used by *sk*:
* fast light toolkit [FLTK-2.0](http://www.fltk.org) for the graphical user interface and the OpenGL interface
* open source computer vision library [OpenCV](http://www.opencv.org) for the webcam capture interface and image compressions (libjpeg and libpng)
* [PortAudio](http://www.portaudio.com) for the audio hardware interface
* the fast Fourier transform implementation of the GNU scientific library [GSL](http://www.gnu.org/software/gsl/) for a simple audio compression algorithm
* [Boost](http://www.boost.org) for multithreading (boost::thread) and asynchronous network I/O (boost::asio)


## Building

To build *sk*, you need to install the dependencies with the corresponding development files first. Probably, they are available from your package manager, if you are using
some kind of Linux. Unfortunately, FLTK-2.0 is an inactive branch of FLTK, it is likely that you have to install it manually. *sk* comes with a simple CMakeLists.txt file,
so you can use CMake to generate a makefile. Currently, this procedure works only on Linux, so for other operating systems you have to create appropriate build files by hand.
After installing all the dependencies and maybe CMake, you can build *sk*:

    mkdir build-directory
    cd build-directory
    cmake path-to-sk/src
    make


## Usage

Skat is a card game played by three people. One starts *sk* as server and tells the other two his or her publicly reachable IP address per e-mail, telephone, or whatever.
Now the other two can start *sk* as client and connect to this disclosed IP address. Then, the server initiates UDP hole punching bewteen the two clients and the
videoconferencing starts. Now the game could start. To be implemented...
