# Find Qwt
# ~~~~~~~~
# Copyright (c) 2010, Tim Sutton <tim at linfiniti.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# Once run this will define:
#
# QWT_FOUND       = system has QWT lib
# QWT_LIBRARY     = full path to the QWT library
# QWT_INCLUDE_DIR = where to find headers
#

set(QWT_LIBRARY_NAMES qwt-qt5 qwt6-qt5 qwt qwt6)

find_library(QWT_LIBRARY
  NAMES ${QWT_LIBRARY_NAMES}
  PATHS
    /usr/lib
    /usr/local/lib
    /usr/local/lib/qwt.framework
    /usr/local/lib/qt5
  )

set(_qwt_fw)
if(QWT_LIBRARY MATCHES "/qwt.*\\.framework")
  string(REGEX REPLACE "^(.*/qwt.*\\.framework).*$" "\\1" _qwt_fw "${QWT_LIBRARY}")
  set ( QWT_LIBRARY "${QWT_LIBRARY}/qwt" )
endif()

FIND_PATH(QWT_INCLUDE_DIR NAMES qwt.h
  PATHS
    "${_qwt_fw}/Headers"
    /usr/include
    /usr/include/qt5
    /usr/local/include
    /usr/local/include/qt5
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
  PATH_SUFFIXES
    ${QWT_LIBRARY_NAMES}
  )

# version
set ( _VERSION_FILE ${QWT_INCLUDE_DIR}/qwt_global.h )
if ( EXISTS ${_VERSION_FILE} )
  file ( STRINGS ${_VERSION_FILE} _VERSION_LINE REGEX "define[ ]+QWT_VERSION_STR" )
  if ( _VERSION_LINE )
    string ( REGEX REPLACE ".*define[ ]+QWT_VERSION_STR[ ]+\"([^\"]*)\".*" "\\1" QWT_VERSION_STRING "${_VERSION_LINE}" )
  endif ()
endif ()
unset ( _VERSION_FILE )

include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Qwt
  FOUND_VAR Qwt_FOUND
  REQUIRED_VARS
    QWT_LIBRARY
    QWT_INCLUDE_DIR
  VERSION_VAR
    QWT_VERSION_STRING
  )

if( Qwt_FOUND )
  set ( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
  set ( QWT_LIBRARIES ${QWT_LIBRARY} )
  if ( NOT TARGET Qwt::Qwt)
    add_library( Qwt::Qwt UNKNOWN IMPORTED )
    set_target_properties( Qwt::Qwt PROPERTIES
      IMPORTED_LOCATION "${QWT_LIBRARIES}"
      INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIRS}"
      )
    if( WIN32 )
      set_target_properties( Qwt::Qwt PROPERTIES INTERFACE_COMPILE_DEFINITIONS QWT_DLL )
    endif()
  endif ()
endif()

mark_as_advanced (
  QWT_LIBRARY
  QWT_INCLUDE_DIR
  )
