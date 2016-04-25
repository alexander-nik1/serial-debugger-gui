 
cmake_minimum_required(VERSION 2.8)

#set(CROSS_COMPILE i586-mingw32msvc-)
#set(CROSS_COMPILE arm-linux-gnueabi-)
#set(CROSS_COMPILE /opt/nxp/gcc-4.3.2-glibc-2.7/bin/arm-vfp-linux-gnu-)

SET(CMAKE_C_COMPILER   ${CROSS_COMPILE}gcc)
SET(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

set(projname serial-debugger-gui)

project(${projname})

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
include_directories(${Qt5Network_INCLUDE_DIRS})

add_definitions(${Qt5Core_DEFINITIONS})
add_definitions(${Qt5Widgets_DEFINITIONS})
add_definitions(${Qt5Network_DEFINITIONS})

QT5_WRAP_CPP(HEADERS_MOC
    ./mainwindow.h
)

QT5_WRAP_UI(FORMS_HEADERS
    ./mainwindow.ui
)

set(SRC
    main.cpp
    mainwindow.cpp
)

# Executables fail to build with Qt 5 in the default configuration
# without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(CMAKE_CXX_FLAGS "-std=c++11")

#add_subdirectory(emodbus)

#include_directories(emodbus)

add_executable(${projname} ${SRC} ${FORMS_HEADERS})

qt5_use_modules(${projname} Core SerialPort Widgets)


###########################################################################################

INSTALL(PROGRAMS bin/serial-debugger-gui DESTINATION /usr/bin)

EXECUTE_PROCESS(COMMAND "date" "+%Y.%m.%d.%H.%M" OUTPUT_VARIABLE CURR_DATETIME)
string(REGEX MATCH "(....).(..).(..).(..).(..)" CURR_DATETIME ${CURR_DATETIME}) # remove newline
message(STATUS "Compilation date = ${CURR_DATETIME}")

set(VERSION_MAJOR "0")
set(VERSION_MINOR "5")
set(VERSION_PATCH "${CURR_DATETIME}")

set(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a,libqt5widgets5,libqt5serialport5")
SET(CPACK_PACKAGE_VENDOR "Alexander")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY " Serial ports debugger")
set(CPACK_PACKAGE_DESCRIPTION " Serial ports hardware debugger. This program allows you control all lines of the serial port.")
set(CPACK_PACKAGE_CONTACT "Alexander Kovylin alexander-nik1@yandex.ru")

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}-${VERSION_PATCH}")

set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}_${PROJECT_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

include(CPack)
