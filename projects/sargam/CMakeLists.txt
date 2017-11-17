
PROJECT( Sargam )

#output apps in bin/"BuildConfiguration"/${PROJECT_NAME}
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME})

#------------------------------------------------------------------------------
# ICON functions
#------------------------------------------------------------------------------
#PATH est le path vers le fichier qui fini par un /
#FILE est le nom du fichier... abcd.icns
MACRO(ADD_MAC_ICON PATH FILE)
  #concatene le path et le nom de fichier et sera utiliser par GENERATE_EXE
  SET( MAC_ICONS ${MAC_ICONS} ${PATH}${FILE} )
  #change le plist pour indiquer le nom de l'icone
  set(MACOSX_BUNDLE_ICON_FILE ${FILE} )
  #copie le fichier icone vers le repertoire ressources du bundle.
  set_source_files_properties( ${MAC_ICONS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
ENDMACRO(ADD_MAC_ICON)

MACRO(ADD_WIN_ICON RC_FILE)
  SET( SOURCES ${SOURCES} ${RC_FILE} )
ENDMACRO(ADD_WIN_ICON)
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# include modules
#------------------------------------------------------------------------------
INCLUDE( "../../CMakeModules/Qt5.cmake" )

#------------------------------------------------------------------------------
# add sources
#------------------------------------------------------------------------------
ADD_COMPONENT( "." "Application" )
ADD_COMPONENT( "./utils" "utils" )

# add cmakelist to sources
SET( SOURCES ${SOURCES}
  "./CMakeLists.txt" )

#sources from utils
#INCLUDE_DIRECTORIES(../../reusables)
#FILE( GLOB utils
#  ../../reusables/utils/Command.*
#  ../../reusables/utils/CommandStack.*
#  ../../reusables/utils/Log.*
#  ../../reusables/utils/utilities.*
#)
#SOURCE_GROUP ("utils" FILES ${utils})

#SET( SOURCES ${SOURCES}
#  ${utils}
#  "./CMakeLists.txt" )

#------------------------------------------------------------------------------
# define icons...
#------------------------------------------------------------------------------
IF(APPLE)
  ADD_MAC_ICON( "./artwork/" "dockIcon.icns" )
ENDIF(APPLE)

IF(WIN32)
  ADD_WIN_ICON( "sargam.rc" )
ENDIF(WIN32)

#
QT_AUTOMOC()

# 
# create executable
#------------------------------------------------------------------------------
IF(APPLE)
    ADD_EXECUTABLE( ${PROJECT_NAME} MACOSX_BUNDLE ${INCLUDE_FILES} ${SOURCE_FILES} ${MAC_ICONS})
ELSE(APPLE)
    ADD_EXECUTABLE( ${PROJECT_NAME} ${INCLUDES} ${SOURCES} )
ENDIF(APPLE)
    
target_link_libraries( ${PROJECT_NAME} 
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Network)