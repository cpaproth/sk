![screenshot](https://github.com/cpaproth/sk/raw/master/images/screenshot.jpg)

# *sk*

Skat-Konferenz is the popular German card game Skat played online combined with videoconferencing. Copyright (C) 2012 Carsten Paproth (carpa at freenet dot de).
This program is free software and comes with ABSOLUTELY NO WARRANTY. Licensed under the terms of [GPLv3](http://www.gnu.org/licenses/).

Skat-Konferenz is written in C++ using portable libraries exclusively, so it should be compatible with UNIX/Linux (X11), MS Windows, and Mac OS X. Till now,
it has been tested on Xubuntu 11.10 (32-bit and 64-bit), Xubuntu 12.04 (64-bit), MS Windows XP and Windows 7 ([download](https://github.com/cpaproth/sk/raw/master/downloads/sk_windows_binary_32-bit.zip)).

This software is in [beta state](https://github.com/cpaproth/sk/tags) and needs some testing. All features of the upcoming release version 1.0 are already implemented:
* videoconferencing between 3 people with audio and video compression (v0.1)
* secure dealing out, i.e. one player alone cannot selectively manipulate the deal or identify the other players cards (v0.3)
* bidding procedure and gameplay according to the international Skat Order rules (v0.5)
* some unofficial but popular rules (Kontra, Re, Bock, Ramsch) and keeping a list of the played games (v0.8)
* robust audio broadcast, every audio frame is split into three spectral frames which are broadcasted independently,
thus a lost audio UDP packet will cause only a slight loss of audio quality (v1.0-beta)

Desirable features of version 2.0 are:
* echo cancellation to improve audio quality
* speech recognition, this would be really cool during the bidding procedure or for saying Kontra
* integration of other but similar games, for example Doppelkopf


## Dependencies

Following portable open source libraries are used by *sk*:
* fast light toolkit [FLTK-2.0](http://www.fltk.org) for the graphical user interface and the OpenGL interface
* open source computer vision library [OpenCV](http://www.opencv.org) for the webcam capture interface and image compression (libjpeg and libpng)
* [PortAudio](http://www.portaudio.com) for the audio hardware interface
* the fast Fourier transform implementation of the GNU scientific library [GSL](http://www.gnu.org/software/gsl/) for a simple audio compression algorithm
* [Boost](http://www.boost.org) for multithreading (boost::thread) and asynchronous network I/O (boost::asio)

Playing card images are copied from [GNOME Aisleriot](https://live.gnome.org/Aisleriot) and modified to look more like Skat cards.


## Building

To build *sk*, you need to install the dependencies with the corresponding development files first. If you are using some kind of Linux, they are probably available from
your package manager. Unfortunately, FLTK-2.0 is an inactive branch of FLTK, it is likely that you have to install it
[manually](https://github.com/cpaproth/sk/raw/master/downloads/fltk-2.0.x-alpha-r9204.tar.bz2).
In this case, you need the following OpenGL and X11 dev packages already installed on your system:

    libgl...-dev, libglu...-dev, libx11-dev, libxft-dev, and libxi-dev

*sk* comes with a simple CMakeLists.txt file, thus you can use CMake to generate a makefile. Currently, this procedure works only on Linux,
for other operating systems, you have to create appropriate build files by hand. After installing all the dependencies and maybe CMake, you can build *sk*:

    mkdir build-directory
    cd build-directory
    cmake -D CMAKE_BUILD_TYPE=Release path-to-sk/src/
    make


## Usage

First of all, you need a webcam, a microphone, headphones and two other players.
Skat is a card game played by three people. One player has to be the server. If you want to be the one, start *sk*, go to the system tab, leave the IP address field empty,
and click connect. Then tell the other two players your publicly reachable IP address or hostname, and UDP port per e-mail, telephone, or whatever.
Now, the other two can start *sk* and connect to this disclosed address and UDP port. If a connection can be established, the videoconferencing starts.
If two peers are connected to the server, the server initiates UDP hole punching between these two peers and if successful, the game starts.
The game consists of secure dealing out, bidding, choosing the Skat cards, announcing the game, playing the hand, and especially small talk: Hinten ist die Ente fett!
