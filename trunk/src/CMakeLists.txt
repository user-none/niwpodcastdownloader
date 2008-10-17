# Remove this once cmake updates FindThreads.cmake
# from SET(CMAKE_THREAD_LIBS_INIT "-lpthread")
# to honor the new CMP0003 policy.
# This just hides the warning.
CMAKE_POLICY(SET CMP0003 OLD)

SET(QT_MIN_VERSION "4.4.0")
FIND_PACKAGE(Qt4 REQUIRED)

SET(QT_DONT_USE_QTGUI TRUE)
SET(QT_USE_QTNETWORK TRUE)
SET(QT_USE_QTSQL TRUE)
SET(QT_USE_QTXML TRUE)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(-Wall)

# Disable platform specific code if NO_PLATFORM is TRUE
IF(NO_PLATFORM)
    ADD_DEFINITIONS(-DNO_PLATFORM)
ENDIF(NO_PLATFORM)

SET(SRC_MOC_HEADERS
    client.h
    database.h
    downloaditem.h
    podcast.h
    podcastepisode.h
    podcastlistingsparser.h
)
SET(SRC_CPP
    client.cpp
    configure.cpp
    database.cpp
    downloaditem.cpp
    main.cpp
    opts.cpp
    platform.cpp
    podcast.cpp
    podcastepisode.cpp
    podcastlistingsparser.cpp
    settingsmanager.cpp
)

QT4_WRAP_CPP(SRC_MOC_CPP ${SRC_MOC_HEADERS})

ADD_EXECUTABLE(niw-podcast-downloader ${SRC_MOC_CPP} ${SRC_CPP})
TARGET_LINK_LIBRARIES(niw-podcast-downloader ${QT_LIBRARIES})

INSTALL(TARGETS niw-podcast-downloader
    RUNTIME DESTINATION bin
)
