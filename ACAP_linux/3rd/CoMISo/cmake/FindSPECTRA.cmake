# - Try to find SPECTRA
# Once done this will define
#  SPECTRA_FOUND         - System has SPECTRA
#  SPECTRA_INCLUDE_DIRS  - The SPECTRA include directories

if (SPECTRA_INCLUDE_DIR)
  # in cache already
  set(SPECTRA_FOUND TRUE)
  set(SPECTRA_INCLUDE_DIRS "${SPECTRA_INCLUDE_DIR}" )
else (SPECTRA_INCLUDE_DIR)

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


find_path( SPECTRA_INCLUDE_DIR 
	   NAMES SymEigsSolver.h 
           PATHS $ENV{SPECTRA_DIR}
                 /usr/include/spectra
                 /usr/local/include
                 /usr/local/include/spectra/
                 /opt/local/include/spectra/
                 "${CMAKE_WINDOWS_LIBS_DIR}/general/spectra"
                 "${CMAKE_WINDOWS_LIBS_DIR}/spectra"
                 "${CMAKE_WINDOWS_LIBS_DIR}/spectra/include"
		 "${CMAKE_WINDOWS_LIBS_DIR}/eigen/include"
		  ${PROJECT_SOURCE_DIR}/MacOS/Libs/SPECTRA/include
                  ../../External/include
                  ${module_file_path}/../../../External/include
          )

set(SPECTRA_INCLUDE_DIRS "${SPECTRA_INCLUDE_DIR}" )


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SPECTRA_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SPECTRA  DEFAULT_MSG
                                  SPECTRA_INCLUDE_DIR)

mark_as_advanced(SPECTRA_INCLUDE_DIR)

endif(SPECTRA_INCLUDE_DIR)
