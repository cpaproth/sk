cmake_minimum_required(VERSION 3.0)
project(sk)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED system thread)
find_package(FLTK REQUIRED)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
	pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0>=19)
else()
	find_package(portaudio)
	if(NOT PORTAUDIO_FOUND)
		set(PORTAUDIO_LIBRARIES "PORTAUDIO_LIB-NOTFOUND" CACHE FILEPATH "portaudio library")
		set(PORTAUDIO_INCLUDE_DIRS "PORTAUDIO_INCLUDE_DIR-NOTFOUND" CACHE PATH "portaudio include directory")
	endif()
endif()

if(MSVC)
	add_compile_definitions(_WIN32_WINNT=0x0501)
	set(Boost_LIBRARIES "")
	link_directories(${Boost_LIBRARY_DIRS})
	set(LINK_WITH_STATIC 1 CACHE BOOL "all libraries are statically linked")
	if(LINK_WITH_STATIC)
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	endif()
endif()

add_executable(sk WIN32 main.cpp
	ui.cpp ui.h
	GlImage.cpp GlImage.h
	GlTable.cpp GlTable.h
	Network.cpp Network.h
	Audio.cpp Audio.h
	Video.cpp Audio.h
	Game.cpp Game.h
	BidButton.cpp BidButton.h
	LogDisplay.cpp LogDisplay.h
	Convenience.cpp Convenience.h)

target_include_directories(sk PUBLIC ${PORTAUDIO_INCLUDE_DIRS} ${FLTK_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_link_libraries(sk ${PORTAUDIO_LIBRARIES} ${FLTK_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
