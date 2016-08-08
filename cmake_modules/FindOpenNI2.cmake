# Try to find OpenNI2 SDK
#
# Once done this will define
# OPENNI2_FOUND - System has OpenNI2 SDK installed
# OPENNI2_INCLUDE_DIRS - The OpenNI2 SDK include directories
# OPENNI2_LIBRARY_DIRS - The OpenNI2 SDK library directories
# OPENNI2_LIBRARIES - The libraries needed to use OpenNI2 SDK

if (WIN32)
  find_path(OPENNI2_INCLUDE_DIR OpenNI.h
            HINTS ENV OPENNI2_INCLUDE64
            PATHS "C:/Program Files/OpenNI2/Include" "C:/OpenNI2/Include")
  find_library(OPENNI2_LIBRARY NAMES OpenNI2
               HINTS ENV OPENNI2_LIB64
               PATHS "C:/Program Files/OpenNI2/Lib" "C:/OpenNI2/Lib")
elseif (UNIX AND NOT APPLE)
  find_path(OPENNI2_INCLUDE_DIR OpenNI.h
            PATHS "/usr/include" "/usr/local/include"
            PATH_SUFFIXES "openni2")
  find_library(OPENNI2_LIBRARY NAMES OpenNI2
               PATHS "/usr/lib" "/usr/local/lib"
	       PATH_SUFFIXES "openni2") 
endif (WIN32)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenNI2 DEFAULT_MSG
                                  OPENNI2_LIBRARY OPENNI2_INCLUDE_DIR)
mark_as_advanced(OPENNI2_INCLUDE_DIR OPENNI2_LIBRARY)

get_filename_component(OPENNI2_LIBRARIES ${OPENNI2_LIBRARY} NAME)
get_filename_component(OPENNI2_LIBRARY_DIRS ${OPENNI2_LIBRARY} DIRECTORY)
set(OPENNI2_INCLUDE_DIRS ${OPENNI2_INCLUDE_DIR})

