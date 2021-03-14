#[[
FindSIP
-------

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SIP_FOUND``
  True if the system has the required SIP executables.
``SIP_Legacy_EXECUTABLE``
  sip or sip5 Executable.
``SIP_Build_EXECUTABLE``
  sip-build Executable.
``SIP_Module_EXECUTABLE``
  sip-module Executable.
``SIP_VERSION``
  The version of the SIP executable.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``SIP_DIR``
  The directory containing the SIP executables.
]]

set( SIP_COMPONENTS Legacy Build Module )

foreach( comp ${SIP_COMPONENTS} )

  if( ${comp} STREQUAL "Legacy" )
    set( sip_names sip sip5 )
  elseif( ${comp} STREQUAL "Build" )
    set( sip_names sip-build )
  elseif( ${comp} STREQUAL "Module" )
    set( sip_names sip-module )
  endif()

  if( SIP_DIR )
    find_program( SIP_${comp}_EXECUTABLE
      NAMES ${sip_names}
      PATHS SIP_DIR
    )
  else()
    find_program( SIP_${comp}_EXECUTABLE NAMES ${sip_names} )
    get_filename_component( SIP_DIR ${SIP_${comp}_EXECUTABLE} DIRECTORY CACHE )
  endif()

  if( SIP_${comp}_EXECUTABLE )
    set( SIP_${comp}_FOUND TRUE )
  endif()

  execute_process(
    COMMAND ${SIP_${comp}_EXECUTABLE} -V
    OUTPUT_VARIABLE SIP_${comp}_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

endforeach()

if( SIP_Build_FOUND AND SIP_Module_FOUND )
  set( SIP_EXECUTABLES ${SIP_Build_EXECUTABLE} ${SIP_Module_EXECUTABLE} )
  set( SIP_VERSION ${SIP_Build_VERSION} )
elseif( SIP_Legacy_FOUND AND SIP_Module_FOUND )
  set( SIP_EXECUTABLES ${SIP_Legacy_EXECUTABLE} ${SIP_Module_EXECUTABLE} )
  set( SIP_VERSION ${SIP_Legacy_VERSION} )
elseif( SIP_Legacy_FOUND AND SIP_VERSION VERSION_LESS 5 )
  set( SIP_EXECUTABLES ${SIP_Legacy_EXECUTABLE} )
  set( SIP_VERSION ${SIP_Legacy_VERSION} )
endif()

include( FindPackageHandleStandardArgs )

find_package_handle_standard_args( SIP
  REQUIRED_VARS SIP_EXECUTABLES
  VERSION_VAR SIP_VERSION
)

mark_as_advanced( SIP_DIR )
