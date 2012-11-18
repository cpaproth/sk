cmake_minimum_required(VERSION 2.6)
project(sk)

find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED system thread)
find_package(FLTK2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSL REQUIRED gsl>=1.15)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0>=19)

add_executable(sk main ui GlImage GlTable Network Audio Video Game
	MathLib Convenience)

target_link_libraries(sk ${GSL_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${FLTK2_LIBRARIES}
	${X11_Xft_LIB} ${OPENGL_LIBRARIES} ${OpenCV_LIBS} ${Boost_LIBRARIES})
