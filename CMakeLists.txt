# ROOT CMakeList
PROJECT( Realisim2 )

cmake_minimum_required (VERSION 2.8.11)

#------------------------------
# Compiler flags
#------------------------------
if(APPLE)
    # this is necessary for some c++11 features such as atomic.
    # MSVC does not require any special flags for c++11
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif(APPLE)

#------------------------------
# Global variables to hold source/include files
#------------------------------
set( SOURCE_FILES "" )
set( INCLUDE_FILES "" )

#------------------------------------------------------------------------------
# GLobal variable to hold the platform specific folder.
# this represent the name of the folder containing the platform
# specific code, libs etc...
#------------------------------------------------------------------------------
SET(PLATFORM_SPECIFIC_FOLDER "")
IF(WIN32)
    SET(PLATFORM_SPECIFIC_FOLDER "windows")
ELSEIF(APPLE)
    SET(PLATFORM_SPECIFIC_FOLDER "mac")
ENDIF(WIN32)

message("PLATFORM_SPECIFIC_FOLDER " ${PLATFORM_SPECIFIC_FOLDER})
#------------------------------------------------------------------------------
# add supplementary functions to cmake
#------------------------------------------------------------------------------
INCLUDE( "CMakeModules/AddComponent.cmake" )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#------------------------------------------------------------------------------
# Options and variables in CMake GUI
#------------------------------------------------------------------------------
option(BUILD_SARGAM "Builds Sargam" ON)

#------------------------------------------------------------------------------
# Compiler flags
#------------------------------------------------------------------------------

if(MSVC)
    # Force to always compile with W4 (Max Error.)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()

    # Force treatWarning as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")

    # add extra secure development lifecycle.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /sdl")
endif(MSVC)

if(APPLE)
    #enable most warning and treat them as error
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wno-long-long -pedantic")
endif(APPLE)

#------------------------------------------------------------------------------
# add subdirectories
#------------------------------------------------------------------------------
if( BUILD_SARGAM )
    ADD_SUBDIRECTORY(Projects/Sargam)
endif()

