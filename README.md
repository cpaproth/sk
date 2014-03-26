![screenshot](https://github.com/cpaproth/sk/raw/development/images/screenshot.jpg)

# Skat-Konferenz *(sk)*

Skat-Konferenz is the popular German card game Skat played online combined with videoconferencing. Copyright (C) 2012-2014 Carsten Paproth (carpa at freenet dot de).
This program is free software and comes with ABSOLUTELY NO WARRANTY. Licensed under the terms of [GPLv3](http://www.gnu.org/licenses/).

Skat-Konferenz is written in C++ using portable libraries exclusively, so it should be compatible with UNIX/Linux (X11), MS Windows, and Mac OS X. Till now,
it has been tested on Ubuntu 11.10 - 13.04 (32-bit and 64-bit), MS Windows XP and Windows 7 ([download](https://github.com/cpaproth/sk/releases/download/v1.0-beta/sk_windows.zip)).


## History

Currently, this software is in [beta phase](https://github.com/cpaproth/sk/tags) and needs some testing.
* 1.2: MDCT-based audio codec, wavelet-based image codec with range encoding, and a more flexible network protocol
* 1.1: ported from FLTK2.0 to FLTK1.3
* 1.0: a more robust audio broadcast method
* 0.9: all functions are implemented, begin of beta phase
* 0.8: some unofficial but popular rules (Kontra, Re, Bock, Ramsch) and keeping a list of the played games
* 0.5: bidding procedure and gameplay according to the international Skat Order rules
* 0.3: secure dealing out, i.e. one player alone cannot selectively manipulate the deal or identify the other players cards
* 0.1: videoconferencing between 3 people with audio and video compression


## Dependencies

Following portable open source libraries are used by *sk*:
* fast light toolkit [FLTK1.3](http://www.fltk.org) for the graphical user interface and the OpenGL interface
* [Boost](http://www.boost.org) for multithreading (boost::thread), asynchronous network I/O (boost::asio), and random number generation (boost::random)
* open source computer vision library [OpenCV](http://www.opencv.org) for the webcam capture interface
* [PortAudio](http://www.portaudio.com) for the audio hardware interface

Playing card images are copied from [GNOME Aisleriot](https://live.gnome.org/Aisleriot) and modified to look more like Skat cards.


## Building

To build *sk*, you need to install the dependencies with the corresponding development files first, use your package manager (if available) to install them.
*sk* comes with a simple CMakeLists.txt file, thus you can use CMake to generate a makefile. Currently, this procedure works only on Linux,
for other operating systems, you have to create appropriate build files by hand. Install all the dependencies and maybe CMake by using, for example:

    sudo apt-get install libfltk1.3-dev libboost-all-dev libopencv-dev portaudio19-dev cmake

[Download](https://github.com/cpaproth/sk/archive/master.zip) and unpack the *sk* source code. Finally, you can build and run *sk*:

    cd sk-master
    mkdir build
    cd build
    cmake -D CMAKE_BUILD_TYPE=Release ../src/
    make
    ./sk


## Usage

First of all, you need a webcam, a microphone, headphones and two other players.
Skat is a card game played by three people. One player has to be the server. If you want to be the one, start *sk*, go to the system tab, leave the *IP Address* field empty, and click *Connect*. Then tell the other two players your publicly reachable IP address or hostname, and UDP port per e-mail, telephone, or whatever.
Now, the other two can start *sk* and connect to this disclosed address and UDP port. If a connection can be established, the videoconferencing starts.
If two peers are connected to the server, the server initiates UDP hole punching between these two peers and if successful, the game starts.
The game consists of secure dealing out, bidding, choosing the Skat cards, announcing the game, playing the hand, and especially small talk: Hinten ist die Ente fett!
