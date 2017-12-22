include(CheckFunctionExists)

project(train_tracks)

cmake_minimum_required(VERSION 2.6)
set (CMAKE_CXX_STANDARD 11)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -Werror")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-long-long -pedantic -Werror")
set(CMAKE_BUILD_TYPE Debug)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/gresource.c
	COMMAND glib-compile-resources --sourcedir=${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/train_tracks.gresource.xml --target=${CMAKE_BINARY_DIR}/gresource.c --generate-source
	DEPENDS ${CMAKE_SOURCE_DIR}/train_tracks.gresource.xml
			${CMAKE_SOURCE_DIR}/train_tracks_app_window.ui
			${CMAKE_SOURCE_DIR}/train_tracks_app_menu.ui
			${CMAKE_SOURCE_DIR}/train_tracks_vertex.glsl
			${CMAKE_SOURCE_DIR}/train_tracks_fragment.glsl)

add_definitions(${GTK3_CFLAGS_OTHER})

add_executable(train_tracks main.c train_tracks_app.c train_tracks_app_window.c gresource.c color.c train_tracks_error.c prog_gl.cpp draw_gl.cpp
	c_cpp_bridge.cpp curves.cpp worker_thread.cpp permutation.cpp zero_handle.cpp matrix.cpp display_cmd.cpp track.cpp one_handle.cpp arrow.cpp io.cpp)

CHECK_FUNCTION_EXISTS(fmod RESULT)
if(NOT RESULT)
  unset(RESULT)
  list(APPEND CMAKE_REQUIRED_LIBRARIES m)
  CHECK_FUNCTION_EXISTS(fmod RESULT0)
  if(RESULT0)
    target_link_libraries(train_tracks m)
  else()
    message(FATAL_ERROR "No fmod() found")
  endif()
endif()
target_link_libraries(train_tracks ${GTK3_LIBRARIES} ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
