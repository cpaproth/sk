cmake_minimum_required(VERSION 2.6)
project(sk)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED system thread)
find_package(FLTK REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0>=19)

add_executable(sk main ui GlImage GlTable Network Audio Video Game BidButton
	LogDisplay Convenience)

target_link_libraries(sk ${PORTAUDIO_LIBRARIES} ${FLTK_LIBRARIES}
	${OpenCV_LIBS} ${Boost_LIBRARIES} ${CMAKE_DL_LIBS})
