PROJECT(niwpodcastdownloader)
ADD_SUBDIRECTORY(src)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)


###
# Write configure data.
###
FIND_PACKAGE(Subversion)
IF(Subversion_FOUND)
    IF(EXISTS .svn)
        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
        SET(VERSION trunk-r${PROJECT_WC_REVISION})
    ELSE(EXISTS .svn)
        SET(VERSION unknown)
    ENDIF(EXISTS .svn)
ENDIF(Subversion_FOUND)

CONFIGURE_FILE(src/configure.h src/configure.h)
CONFIGURE_FILE(src/configure.cpp.cmake src/configure.cpp)
