CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(PITCHDETECT)
SET(CMAKE_C_FLAGS "-std=gnu99 -ggdb -fPIC -pipe")


ADD_DEFINITIONS("-Wall -Wextra -Wpointer-arith -Wcast-qual
  -Wno-unused-parameter -fstrict-aliasing")

FIND_PACKAGE(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

INCLUDE_DIRECTORIES( ${GTK3_INCLUDE_DIRS})

SET(COMMON_LIBRARIES m)

ADD_EXECUTABLE(pitchdetector pitchdetector.c)
TARGET_LINK_LIBRARIES(pitchdetector ${GTK3_LIBRARIES} sndfile)
