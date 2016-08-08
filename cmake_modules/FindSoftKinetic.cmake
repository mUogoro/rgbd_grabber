# Try to find SoftKinetic SDK
#
# Once done this will define
# SOFTKINETIC_FOUND - System has SoftKinetic SDK installed
# SOFTKINETIC_INCLUDE_DIRS - The SoftKinetic SDK include directories
# SOFTKINETIC_LIBRARY_DIRS - The SoftKinetic SDK library directories
# SOFTKINETIC_LIBRARIES - The libraries needed to use SoftKinetic SDK

if (WIN32)
  find_path(SOFTKINETIC_INCLUDE_DIR DepthSense.hxx
            HINTS ENV DEPTHSENSESDK64
            PATHS "C:/Program Files/SoftKinetic" "C:/SoftKinetic"
			PATH_SUFFIXES "include" "DepthSenseSDK/include")
  find_library(SOFTKINETIC_LIBRARY NAMES DepthSense
               HINTS ENV DEPTHSENSESDK64
               PATHS "C:/Program Files/SoftKinetic" "C:/SoftKinetic"
			   PATH_SUFFIXES "lib" "DepthSenseSDK/lib")
elseif (UNIX AND NOT APPLE)
  find_path(SOFTKINETIC_INCLUDE_DIR DepthSense.hxx
            PATHS "/opt/softkinetic" "/usr" "/usr/local"
	    PATH_SUFFIXES "DepthSenseSDK/include")
  find_library(SOFTKINETIC_LIBRARY NAMES DepthSense
               PATHS "/opt/softkinetic" "/usr/lib" "/usr/local"
               PATH_SUFFIXES "DepthSenseSDK/lib")		   
endif (WIN32)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SoftKinetic DEFAULT_MSG
                                  SOFTKINETIC_LIBRARY SOFTKINETIC_INCLUDE_DIR)
mark_as_advanced(SOFTKINETIC_INCLUDE_DIR SOFTKINETIC_LIBRARY)

get_filename_component(SOFTKINETIC_LIBRARIES ${SOFTKINETIC_LIBRARY} NAME)
get_filename_component(SOFTKINETIC_LIBRARY_DIRS ${SOFTKINETIC_LIBRARY} DIRECTORY)
set(SOFTKINETIC_INCLUDE_DIRS ${SOFTKINETIC_INCLUDE_DIR})
