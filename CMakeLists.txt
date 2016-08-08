cmake_minimum_required (VERSION 2.8)

project(RGBDGrabber)
set(RGBDGRABBER_MAJOR_VERSION 0)
set(RGBDGRABBER_MINOR_VERSION 1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")
find_package(OpenNI2)
find_package(SoftKinetic)
find_package(PThread REQUIRED)
find_package(OpenCVHighGuiImgProc)

# Enable cmath macros in Visual Studio
if (WIN32)
  add_definitions(-D_USE_MATH_DEFINES)
endif (WIN32)

# Check if OpenNI or SoftKinetic SDKs have been found
if (NOT OPENNI2_FOUND AND NOT SOFTKINETIC_FOUND)
  message(FATAL_ERROR "No supported camera SDK found")
endif (NOT OPENNI2_FOUND AND NOT SOFTKINETIC_FOUND)

# Set output directories
# - binaries (e.g. tests) in /bin
# - libraries (static and shared) and debug info in /lib
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_BINARY_DIR}/lib")
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

include_directories(${PTHREAD_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/src")
set(SRC_FILES "")
set(HEADER_FILES "${CMAKE_SOURCE_DIR}/src/rgbd_grabber.hpp")
set(LIBRARY_DIRS ${PTHREAD_LIBRARY_DIRS} ${OPENCV_LIBRARY_DIRS})
set(LIBRARIES ${PTHREAD_LIBRARIES} ${OPENCV_LIBRARIES})
if (OPENNI2_FOUND)
  set(SRC_FILES ${SRC_FILES} "${CMAKE_SOURCE_DIR}/src/openni2_grabber.cpp")
  set(HEADER_FILES ${HEADER_FILES} "${CMAKE_SOURCE_DIR}/src/openni2_grabber.hpp")
  set(LIBRARY_DIRS ${OPENNI2_LIBRARY_DIRS} ${LIBRARY_DIRS})
  set(LIBRARIES ${OPENNI2_LIBRARIES} ${LIBRARIES})
  include_directories(${OPENNI2_INCLUDE_DIRS} ${CMAKE_INCLUDE_PATH})
endif (OPENNI2_FOUND)
if (SOFTKINETIC_FOUND)
  include_directories(${SOFTKINETIC_INCLUDE_DIRS} ${CMAKE_INCLUDE_PATH})
  set(SRC_FILES ${SRC_FILES} "${CMAKE_SOURCE_DIR}/src/softkinetic_grabber.cpp")
  set(HEADER_FILES ${HEADER_FILES} "${CMAKE_SOURCE_DIR}/src/softkinetic_grabber.hpp")
  set(LIBRARY_DIRS ${SOFTKINETIC_LIBRARY_DIRS} ${LIBRARY_DIRS})
  set(LIBRARIES ${SOFTKINETIC_LIBRARIES} ${LIBRARIES})
endif (SOFTKINETIC_FOUND)
link_directories(${LIBRARY_DIRS} "${CMAKE_BINARY_DIR}/lib" )

# Note: build a shared library for windows and a static labrary for linux
if (WIN32)
  add_library(RGBDGrabber STATIC ${SRC_FILES})
elseif (UNIX AND NOT APPLE)
  add_library(RGBDGrabber SHARED ${SRC_FILES})
  target_link_libraries(RGBDGrabber ${LIBRARIES})
endif (WIN32)

# Add test files
add_executable(test_openni2 "${CMAKE_SOURCE_DIR}/test/test_openni2.cpp")
target_link_libraries(test_openni2 ${LIBRARIES} RGBDGrabber)

add_executable(test_softkinetic "${CMAKE_SOURCE_DIR}/test/test_softkinetic.cpp")
target_link_libraries(test_softkinetic ${LIBRARIES} RGBDGrabber)

# Process cmake config file
set(RGBDGRABBER_INCLUDE_DIR ${PTHREAD_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${OPENNI2_INCLUDE_DIRS} ${SOFTKINETIC_INCLUDE_DIRS} "${CMAKE_INSTALL_PREFIX}/include")
set(RGBDGRABBER_LIBRARY_DIR ${LIBRARY_DIRS} "${CMAKE_INSTALL_PREFIX}/lib")
if (WIN32)
  set(RGBDGRABBER_LIBRARY ${LIBRARIES} "${CMAKE_STATIC_LIBRARY_PREFIX}RGBDGrabber${CMAKE_STATIC_LIBRARY_SUFFIX}")
elseif (UNIX AND NOT APPLE)
  set(RGBDGRABBER_LIBRARY ${LIBRARIES} "${CMAKE_SHARED_LIBRARY_PREFIX}RGBDGrabber${CMAKE_SHARED_LIBRARY_SUFFIX}")
endif (WIN32)
configure_file("${CMAKE_SOURCE_DIR}/RGBDGrabberConfig.cmake.in"
               "${CMAKE_BINARY_DIR}/RGBDGrabberConfig.cmake")

message(STATUS "" "${LIBRARIES}")
			   
# Installation directives
if (WIN32)
  install(FILES "${CMAKE_BINARY_DIR}/RGBDGrabberConfig.cmake" DESTINATION "/")
  if (OPENNI2_FOUND)
    install(TARGETS test_openni2 DESTINATION "test")
  endif (OPENNI2_FOUND)
  if (SOFTKINETIC_FOUND)
    install(TARGETS test_softkinetic DESTINATION "test")
  endif (SOFTKINETIC_FOUND)
elseif (UNIX AND NOT APPLE)
  install(FILES "${CMAKE_BINARY_DIR}/RGBDGrabberConfig.cmake" DESTINATION "share/rgbd_grabber")
  if (OPENNI2_FOUND)
    install(TARGETS test_openni2 DESTINATION "share/rgbd_grabber/test")
  endif (OPENNI2_FOUND)
  if (SOFTKINETIC_FOUND)
    install(TARGETS test_softkinetic DESTINATION "share/rgbd_grabber/test")
  endif (SOFTKINETIC_FOUND)
endif (WIN32)
install(FILES ${HEADER_FILES} DESTINATION "include")
install(TARGETS RGBDGrabber DESTINATION "lib")

