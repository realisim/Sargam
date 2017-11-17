macro( add_component iPath iDisplayedName)

# add current directory to includes paths
# add parent directory to includes paths for case like:
#   ADD_COMPONENT("../../Math" "Math")
#   ...
#   in code, we want to do: #include "Math/Matrix4.h"
#   
include_directories( ${iPath} )
include_directories( ${iPath}/.. )

#message( "Globbing: " ${CMAKE_CURRENT_BINARY_DIR}/${iPath} )

file( GLOB iPATH_SOURCE_FILES
  ${iPath}/*.cpp )

file( GLOB iPATH_INCLUDE_FILES
  ${iPath}/*.h )

# Replace / by \\ present in iDasplayedName so we can create a folder hierachy in the solution.
# For example: 
#   ${iDisplayedName} = 'Simulator/Palettes'
#
string(REPLACE "/" "\\" source_group_msvc ${iDisplayedName})
source_group ("${source_group_msvc}" FILES ${iPATH_INCLUDE_FILES} ${iPATH_SOURCE_FILES})

# add sources and include to global variable
set( SOURCE_FILES ${SOURCE_FILES} ${iPATH_SOURCE_FILES} )
set( INCLUDE_FILES ${INCLUDE_FILES} ${iPATH_INCLUDE_FILES} )

endmacro(add_component)