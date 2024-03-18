
set(LIBORIGIN_ROOT_DIR "${LIBORIGIN_ROOT_DIR}"  CACHE PATH "Directory to search for liborigin" )

find_package(PkgConfig QUIET)
if( PkgConfig_FOUND )
  pkg_search_module(PC_LIBORIGIN QUIET liborigin)
  if( PC_LIBORIGIN_FOUND )
    set( LIBORIGIN_VERSION ${PC_LIBORIGIN_VERSION} )
  endif()
endif()

find_path( LIBORIGIN_INCLUDE_DIR
  NAMES OriginFile.h
  PATHS "${LIBORIGIN_ROOT_DIR}"
  HINTS ${PC_LIBORIGIN_INCLUDEDIR} ${PC_LIBORIGIN_INCLUDE_DIRS}
  PATH_SUFFIXES liborigin
)
find_library( LIBORIGIN_LIBRARY
  NAMES origin
  PATHS "${LIBORIGIN_ROOT_DIR}"
  HINTS ${PC_LIBORIGIN_LIBDIR} ${PC_LIBORIGIN_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( liborigin
  FOUND_VAR LIBORIGIN_FOUND
  REQUIRED_VARS
    LIBORIGIN_LIBRARY
    LIBORIGIN_INCLUDE_DIR
  VERSION_VAR LIBORIGIN_VERSION
)

if(LIBORIGIN_FOUND)
  set(LIBORIGIN_INCLUDE_DIRS ${LIBORIGIN_INCLUDE_DIR})
  set(LIBORIGIN_LIBRARIES ${LIBORIGIN_LIBRARY})
  if(NOT TARGET liborigin::liborigin)
    add_library(liborigin::liborigin UNKNOWN IMPORTED)
    set_target_properties( liborigin::liborigin
      PROPERTIES
        IMPORTED_LOCATION "${LIBORIGIN_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBORIGIN_INCLUDE_DIR}" 
    )
  endif()
  mark_as_advanced(LIBORIGIN_ROOT_DIR)
endif()

mark_as_advanced(LIBORIGIN_INCLUDE_DIR LIBORIGIN_LIBRARY)
