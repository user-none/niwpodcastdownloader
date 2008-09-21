PROJECT(niwpodcastdownloader)
ADD_SUBDIRECTORY(src)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)


###
# Write configure data.
###
FIND_PACKAGE(Subversion)
IF(Subversion_FOUND)
    IF(EXISTS .svn)
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} PROJECTVERSION)
        SET(VERSION trunk-r${PROJECTVERSION_WC_REVISION})
    ELSE(EXISTS .svn)
        SET(VERSION unknown)
    ENDIF(EXISTS .svn)
ENDIF(Subversion_FOUND)

CONFIGURE_FILE(src/configure.h src/configure.h)
CONFIGURE_FILE(src/configure.cpp.cmake src/configure.cpp)


###
# Set build type.
###
IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "Release")
ENDIF(NOT CMAKE_BUILD_TYPE)

IF(NOT CMAKE_INSTALL_CONFIG_NAME)
    SET(CMAKE_INSTALL_CONFIG_NAME ${CMAKE_BUILD_TYPE})
ENDIF(NOT CMAKE_INSTALL_CONFIG_NAME)


###
# Show the build type and the install prefix.
###
MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
MESSAGE(STATUS "Install prefix: \"${CMAKE_INSTALL_PREFIX}\"")
