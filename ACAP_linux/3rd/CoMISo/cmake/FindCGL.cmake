# - Try to find CGL
# Once done this will define
#  CGL_FOUND - System has CGL
#  CGL_INCLUDE_DIRS - The CGL include directories
#  CGL_LIBRARIES - The libraries needed to use CGL


# Check if the base path is set
if ( NOT CMAKE_WINDOWS_LIBS_DIR )
  # This is the base directory for windows library search used in the finders we shipp.
  set(CMAKE_WINDOWS_LIBS_DIR "c:/libs" CACHE STRING "Default Library search dir on windows." )
endif()

if ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2012/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 11.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2012/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2013/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2013/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 14.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2015/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 14.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2015/x32/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 15.*Win64" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2017/x64/")
elseif ( CMAKE_GENERATOR MATCHES "^Visual Studio 15.*" )
  SET(VS_SEARCH_PATH "${CMAKE_WINDOWS_LIBS_DIR}/vs2017/x32/")
endif()

if ( NOT CGL_FOUND)

find_path(CGL_INCLUDE_DIR 
          NAMES CglConfig.h
          PATHS "$ENV{CGL_DIR}/include/coin"
                "$ENV{CBC_DIR}/include/coin"
                 "/usr/include/coin"
                 "C:\\libs\\cgl\\include"
                 "C:\\libs\\cbc\\include"
                 "${VS_SEARCH_PATH}CBC-2.9.7/Cgl/include"
                 "${VS_SEARCH_PATH}CBC-2.9.4/Cgl/include"
              )

find_library( CGL_LIBRARY_DEBUG 
              NAMES Cgld libCgld
              PATHS "$ENV{CGL_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cgl\\lib"
                    "C:\\libs\\cbc\\lib"
                    "${VS_SEARCH_PATH}CBC-2.9.7/lib/${VS_SUBDIR}Debug"
                    "${VS_SEARCH_PATH}CBC-2.9.4/Cgl/lib"
              )
              
find_library( CGL_LIBRARY_RELEASE
              NAMES Cgl libCgl
              PATHS "$ENV{CGL_DIR}/lib"
                    "$ENV{CBC_DIR}/lib" 
                    "/usr/lib"
                    "/usr/lib/coin"
                    "C:\\libs\\cgl\\lib"
                    "C:\\libs\\cbc\\lib"
                    "${VS_SEARCH_PATH}CBC-2.9.7/lib/${VS_SUBDIR}Release"
                    "${VS_SEARCH_PATH}CBC-2.9.4/Cgl/lib"
              )              

include(SelectLibraryConfigurations)
select_library_configurations( CGL )
  
set(CGL_INCLUDE_DIRS "${CGL_INCLUDE_DIR}" )
set(CGL_LIBRARIES "${CGL_LIBRARY}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set CGL_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(CGL  DEFAULT_MSG
                                  CGL_LIBRARY CGL_INCLUDE_DIR)

mark_as_advanced(CGL_INCLUDE_DIR CGL_LIBRARY)

endif(NOT CGL_FOUND)
