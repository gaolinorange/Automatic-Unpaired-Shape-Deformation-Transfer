include (ACGCommon)

if ( NOT DEFINED BUILD_APPS )
  set( BUILD_APPS true CACHE BOOL "Enable or disable building of apps" )
endif()
 
if ( BUILD_APPS )

  # Only build and fixbundle apps, when we are building OpenMesh standalone
  if(${PROJECT_NAME} MATCHES "OpenMesh")

    if ( WIN32 AND OPENMESH_BUILD_SHARED )
	  add_definitions(-DOPENMESHDLL )
    endif()

    add_subdirectory (Dualizer)
    add_subdirectory (Decimating/commandlineDecimater)
    add_subdirectory (Smoothing)
    add_subdirectory (Subdivider/commandlineSubdivider)
    add_subdirectory (Subdivider/commandlineAdaptiveSubdivider)
    add_subdirectory (mconvert)
    add_subdirectory (VDProgMesh/mkbalancedpm)
    add_subdirectory (VDProgMesh/Analyzer)

    # Add non ui apps as dependency before fixbundle 
    if ( WIN32 )
      if ( NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles" )
	# let bundle generation depend on all targets
	add_dependencies (fixbundle commandlineDecimater Dualizer mconvert Smoothing commandlineAdaptiveSubdivider commandlineSubdivider mkbalancedpm Analyzer )
      endif()
    endif()

    # Add non ui apps as dependency before fixbundle
    if ( APPLE)
      # let bundle generation depend on all targets
      add_dependencies (fixbundle commandlineDecimater Dualizer mconvert Smoothing commandlineAdaptiveSubdivider commandlineSubdivider mkbalancedpm Analyzer )
    endif()


    # find needed packages for gui applications
    find_package (OpenGL)
    find_package (GLUT)

    # try to use QT5 if possible otherwise stick to QT4
    set (FORCE_QT4 OFF CACHE BOOL "Use Qt4 even if Qt5 was found")

    # For the apps, we need qt and opengl to build them
    if (NOT QT5_FOUND AND NOT FORCE_QT4)
      acg_qt5 ()
    endif()
    if (NOT QT5_FOUND AND NOT QT4_FOUND)
      find_package (Qt4 COMPONENTS QtCore QtGui )

      set (QT_USE_QTOPENGL 1)

      include (${QT_USE_FILE})
    endif ()

    if ("${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles")
      message(WARNING "GUI Apps are not build with mingw. (TODO)")
    endif()

    # check for OpenGL and GLUT as our required dependencies
    if ((QT5_FOUND OR QT4_FOUND) AND OPENGL_FOUND AND GLUT_FOUND AND NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles" )

      add_subdirectory (Decimating/DecimaterGui)
      add_subdirectory (QtViewer)
      add_subdirectory (Subdivider/SubdividerGui)
      add_subdirectory (ProgViewer)
      add_subdirectory (VDProgMesh/Synthesizer)
			    

      # Add ui apps as dependency before fixbundle 
      if ( WIN32 AND NOT "${CMAKE_GENERATOR}" MATCHES "MinGW Makefiles")
	# let bundle generation depend on all targets
	add_dependencies (fixbundle DecimaterGui ProgViewer QtViewer SubdividerGui Synthesizer)
      endif()
    
      # Add ui apps as dependency before fixbundle 
      if ( APPLE)
	# let bundle generation depend on all targets
	add_dependencies (fixbundle DecimaterGui ProgViewer QtViewer SubdividerGui Synthesizer)
      endif()

      if ( WIN32 )
	    FILE(GLOB files_install_app_dlls "${CMAKE_BINARY_DIR}/Build/*.dll" )
	    INSTALL(FILES ${files_install_app_dlls} DESTINATION . )

            # platform plugins for qt shoulb be installed along with the apps to run on the target systems
            if (EXISTS ${CMAKE_BINARY_DIR}/Build/plugins/platforms )
              FILE(GLOB files_install_app_platform_dlls "${CMAKE_BINARY_DIR}/Build/plugins/platforms/*.dll" )
              INSTALL(FILES ${files_install_app_platform_dlls} DESTINATION plugins/platforms/ )

              # Create a qt.conf file to find plugins on windows
              INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Qt/qt.conf DESTINATION . )
            endif()

      endif()

    else () # QT ,Opengl or glut not found
      
      if (NOT QT4_FOUND AND NOT QT5_FOUND)
	    message ("QT 4 and 5 not found! Skipping some apps.")
      endif ()

      if (NOT OPENGL_FOUND)
	    message ("OpengGL not found! Skipping some apps.")
      endif ()

      if (NOT GLUT_FOUND)
	    message ("GLUT not found! Skipping some apps.")
      endif ()
      
    endif ()

  endif() # Project is OpenMesh standalone

else (BUILD_APPS)

  message ("Building Apps disabled by user.")

endif (BUILD_APPS)
