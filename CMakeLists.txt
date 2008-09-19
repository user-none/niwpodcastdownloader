PROJECT(niwpodcastdownloader)
ADD_SUBDIRECTORY(src)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)


###
# Write configure data.
###
SET(VERSION "1.0.2")

CONFIGURE_FILE(src/configure.h src/configure.h)
CONFIGURE_FILE(src/configure.cpp.cmake src/configure.cpp)
