# Once done this will define
#  GUROBI_FOUND - System has Gurobi
#  GUROBI_INCLUDE_DIRS - The Gurobi include directories
#  GUROBI_LIBRARIES - The libraries needed to use Gurobi

set(GUROBI_ENABLE ON CACHE BOOL "Enable gurobi?")

if (GUROBI_ENABLE)

set(GUROBI_BASE $ENV{GUROBI_HOME} CACHE PATH "GUROBI root directory.")

find_path(GUROBI_INCLUDE_DIR
          NAMES gurobi_c++.h
          PATHS
          "${GUROBI_BASE}/include"
          "$ENV{GUROBI_HOME}/include"
          )

get_filename_component(GUROBI_LIB_DIR "${GUROBI_INCLUDE_DIR}/../lib" ABSOLUTE)
# GUROBI_BIN_DIR is needed on windows, where it contains the .dll
get_filename_component(GUROBI_BIN_DIR "${GUROBI_INCLUDE_DIR}/../bin" ABSOLUTE)
get_filename_component(GUROBI_SRC_DIR "${GUROBI_INCLUDE_DIR}/../src" ABSOLUTE)


file(GLOB GUROBI_LIBRARY_LIST
    RELATIVE ${GUROBI_LIB_DIR}
    ${GUROBI_LIB_DIR}/libgurobi*.so
    ${GUROBI_LIB_DIR}/libgurobi*.dll)


# Ignore libgurobiXY_light.so, libgurobi.so (without version):
string(REGEX MATCHALL
    "libgurobi([0-9]+)\\..*"
    GUROBI_LIBRARY_LIST
    "${GUROBI_LIBRARY_LIST}"
    )

string(REGEX REPLACE
    "libgurobi([0-9]+)\\..*"
    "\\1"
    GUROBI_LIBRARY_VERSIONS
    "${GUROBI_LIBRARY_LIST}")
list(LENGTH GUROBI_LIBRARY_VERSIONS GUROBI_NUMVER)

#message("GUROBI LIB VERSIONS: ${GUROBI_LIBRARY_VERSIONS}")

if (GUROBI_NUMVER EQUAL 1)
    list(GET GUROBI_LIBRARY_VERSIONS 0 GUROBI_LIBRARY_VERSION)
    set(GUROBI_LIBRARY_NAME "gurobi${GUROBI_LIBRARY_VERSION}")
else()
    # none or more than one versioned library -let's try without suffix,
    # maybe the user added a symlink to the desired library
    set(GUROBI_LIBRARY_NAME "gurobi")
endif()

#message("GUROBI LIB NAME: ${GUROBI_LIBRARY_NAME}")

find_library(GUROBI_LIBRARY
    NAMES ${GUROBI_LIBRARY_NAME}
    PATHS
    "${GUROBI_BASE}/lib"
    "$ENV{GUROBI_HOME}/lib"
)

# Gurobi ships with some compiled versions of its C++ library for specific
# compilers, however it also comes with the source code. We will compile
# the source code outselves -- this is much safer, as it guarantees the same
# compiler version and flags.
# (Note: doing this is motivated by actual sometimes-subtle ABI compatibility bugs)
# The old behaviour can be enabled with GUROBI_USE_PRECOMPILED_CXX)

option(GUROBI_USE_PRECOMPILED_CXX "Use precompiled C++ libraries instead of building it ourselves. Not recommended." OFF)
mark_as_advanced(GUROBI_USE_PRECOMPILED_CXX)

if(GUROBI_USE_PRECOMPILED_CXX)
  if ( CMAKE_GENERATOR MATCHES "^Visual Studio 12.*Win64" )
    SET(GUROBI_LIB_NAME "gurobi_c++md2013")
  endif()
  
find_library(GUROBI_CXX_LIBRARY 
             NAMES gurobi_c++
                   ${GUROBI_LIB_NAME}
             PATHS "$ENV{GUROBI_HOME}/lib" 
              "${GUROBI_BASE}/lib"
             )
else()
    file(GLOB GUROBI_CXX_SRC CONFIGURE_DEPENDS ${GUROBI_SRC_DIR}/cpp/*.cpp)
    if(NOT GUROBI_CXX_SRC)
        message(FATAL_ERROR "could not find gurobi c++ sources in GUROBI_SRC_DIR=${GUROBI_SRC_DIR}/cpp/.")
    endif()
    add_library(GurobiCXX STATIC ${GUROBI_CXX_SRC})
    target_include_directories(GurobiCXX PUBLIC ${GUROBI_INCLUDE_DIR})
    # We need to be able to link this into a shared library:
    set_target_properties(GurobiCXX PROPERTIES POSITION_INDEPENDENT_CODE ON)
    set(GUROBI_CXX_LIBRARY $<TARGET_FILE:GurobiCXX>)
endif(GUROBI_USE_PRECOMPILED_CXX)

set(GUROBI_INCLUDE_DIRS "${GUROBI_INCLUDE_DIR}" )
set(GUROBI_LIBRARIES "${GUROBI_CXX_LIBRARY};${GUROBI_LIBRARY}" )

# use c++ headers as default
# set(GUROBI_COMPILER_FLAGS "-DIL_STD" CACHE STRING "Gurobi Compiler Flags")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GUROBI  DEFAULT_MSG
                                  GUROBI_CXX_LIBRARY GUROBI_LIBRARY GUROBI_INCLUDE_DIR)

mark_as_advanced(GUROBI_INCLUDE_DIR GUROBI_LIBRARY GUROBI_CXX_LIBRARY GUROBI_BIN_DIR )

endif(GUROBI_ENABLE)
