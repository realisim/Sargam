
# Variable to hold the bin directory of Qt, it is dependent on Qt5Core_DIR
get_filename_component(Qt5_BIN_DIR ${Qt5Core_DIR}/../../../bin ABSOLUTE)

# Widgets finds its own dependencies (QtGui and QtCore).
FIND_PACKAGE(Qt5Widgets)

FIND_PACKAGE(Qt5OpenGL)

FIND_PACKAGE(Qt5PrintSupport REQUIRED)
FIND_PACKAGE(Qt5Network REQUIRED)

#------------------------------------------------------------------------------
# remove some anonying warning created by QT
#------------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4127")
endif(MSVC)

IF(APPLE)
    #-fvisibility=hidden sert a enlever les milliers de warnings de Qt depuis la version 4.5 
    ADD_DEFINITIONS(-fvisibility=hidden)
ENDIF(APPLE)

#--------------------------------
# Fonctions
#--------------------------------

MACRO(QT_AUTOMOC)
    # Instruct CMake to run moc automatically when needed.
    set(CMAKE_AUTOMOC ON)
ENDMACRO(QT_AUTOMOC)

# Copy all required Qt files to iInstallPath
MACRO(QT5_INSTALL iInstallPath)

    IF(WIN32)
        #MESSAGE("path to QtCore: " ${Qt5Core_DIR} )
        #MESSAGE("path to Qt Bin: " ${Qt5_BIN_DIR} )
        #MESSAGE("Qt5Widgets_LIBRARIES in use: " ${Qt5Widgets_LIBRARIES} )
        #MESSAGE("Qt5OpenGL_LIBRARIES in use: " ${Qt5OpenGL_LIBRARIES} )

        SET(ICU_FILES ${Qt5_BIN_DIR}/icudt54.dll
            ${Qt5_BIN_DIR}/icuin54.dll
            ${Qt5_BIN_DIR}/icuuc54.dll )

        #--- dll related to Qt5Widgets
        SET(QT5WIDGETS_FILES_DEBUG "")
        SET(QT5WIDGETS_FILES_RELEASE "")
        IF (NOT "${Qt5Widgets_LIBRARIES}" STREQUAL "")
            SET(QT5WIDGETS_FILES_DEBUG ${Qt5_BIN_DIR}/Qt5Cored.dll
                ${Qt5_BIN_DIR}/Qt5Cored.pdb
                ${Qt5_BIN_DIR}/Qt5Guid.dll
                ${Qt5_BIN_DIR}/Qt5Guid.pdb
                ${Qt5_BIN_DIR}/Qt5Widgetsd.dll
                ${Qt5_BIN_DIR}/Qt5Widgetsd.pdb )

            SET(QT5WIDGETS_FILES_RELEASE ${Qt5_BIN_DIR}/Qt5Core.dll
                ${Qt5_BIN_DIR}/Qt5Gui.dll
                ${Qt5_BIN_DIR}/Qt5Widgets.dll )
        ENDIF()

        #--- dll related to Qt5OpenGL
        SET(QT5OPENGL_FILES_DEBUG "")
        SET(QT5OPENGL_FILES_RELEASE "")
        IF (NOT "${Qt5OpenGL_LIBRARIES}" STREQUAL "")
            SET(QT5OPENGL_FILES_DEBUG ${Qt5_BIN_DIR}/Qt5OpenGLd.dll
                ${Qt5_BIN_DIR}/Qt5OpenGLd.pdb )

            SET(QT5OPENGL_FILES_RELEASE ${Qt5_BIN_DIR}/Qt5OpenGL.dll )
        ENDIF()

        # --- and now install...
        # --- in DEBUG
        INSTALL(FILES ${ICU_FILES} CONFIGURATIONS Debug DESTINATION ${iInstallPath}/Debug)
        INSTALL(FILES ${QT5WIDGETS_FILES_DEBUG} CONFIGURATIONS Debug DESTINATION ${iInstallPath}/Debug)
        INSTALL(FILES ${QT5OPENGL_FILES_DEBUG} CONFIGURATIONS Debug DESTINATION ${iInstallPath}/Debug)

        # ----- RELEASE
        INSTALL(FILES ${ICU_FILES} CONFIGURATIONS Release DESTINATION ${iInstallPath}/Release)
        INSTALL(FILES ${QT5WIDGETS_FILES_RELEASE} CONFIGURATIONS Release DESTINATION ${iInstallPath}/Release)
        INSTALL(FILES ${QT5OPENGL_FILES_RELEASE} CONFIGURATIONS Release DESTINATION ${iInstallPath}/Release)

    ENDIF(WIN32)

ENDMACRO(QT5_INSTALL)