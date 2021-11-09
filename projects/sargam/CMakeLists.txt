
PROJECT( Sargam )

#output apps in bin/"BuildConfiguration"/${PROJECT_NAME}
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME})

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/MP)
endif()

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
  SET( SOURCE_FILES ${SOURCE_FILES} ${RC_FILE} )
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
SET( SOURCE_FILES ${SOURCE_FILES}
  "./CMakeLists.txt" )

# add release notes to sources
SET( SOURCE_FILES ${SOURCE_FILES}
  "./releaseNotes/sargamReleaseNotes.txt"
  "./releaseNotes/sargamReleaseNotes-preprod.txt")

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
    ADD_EXECUTABLE( ${PROJECT_NAME} WIN32 ${INCLUDE_FILES} ${SOURCE_FILES} )
ENDIF(APPLE)
    
target_link_libraries( ${PROJECT_NAME} 
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Network)

# install directives
QT5_INSTALL(${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME})

# install ThirdParties
set(QtOpenSslDlls 
  ${CMAKE_SOURCE_DIR}/ThirdParties/OpenSsl/libcrypto-1_1-x64.dll
  ${CMAKE_SOURCE_DIR}/ThirdParties/OpenSsl/libssl-1_1-x64.dll)
INSTALL(FILES ${QtOpenSslDlls} CONFIGURATIONS Debug DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}/Debug)
INSTALL(FILES ${QtOpenSslDlls} CONFIGURATIONS Release DESTINATION ${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}/Release)