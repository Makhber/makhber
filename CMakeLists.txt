cmake_minimum_required( VERSION 3.16 )

cmake_policy(SET CMP0083 NEW)
cmake_policy(SET CMP0091 NEW)

project( makhber
  VERSION 0.10.0
  DESCRIPTION "Free application for Visualization and Analysis of Scientific Data"
  HOMEPAGE_URL "https://github.com/Makhber/makhber"
  LANGUAGES CXX C
  )

math( EXPR PROJECT_VERSION_NO "${PROJECT_VERSION_MAJOR} * 65536 + \
  ${PROJECT_VERSION_MINOR} * 256 + ${PROJECT_VERSION_PATCH}" )

set( PROJECT_RELEASE_DATE "MMM, DD YYYY" )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED TRUE )
set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_C_STANDARD 11 )
set( CMAKE_C_STANDARD_REQUIRED TRUE )
set( CMAKE_C_EXTENSIONS OFF )

if( MSVC )
  # /wd4456 /wd4457 /wd4458 Silent "declaration of %1 hides %2 ..."
  # /wd4251 Silent dll-related warnings
  # /wd4127 Silent conditional expression is constant (Qt headers)
  # /wd4310 Silent cast truncates constant value (muParser headers)
  # /wd4996 strcpy is unsafe warnings, equiv to #define _SCL_SECURE_NO_WARNINGS
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W4 /permissive- \
	 /wd4456 /wd4457 /wd4458 /wd4251 /wd4127 /wd4310 /wd4996" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /permissive-" )
  if( CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-nonportable-include-path" )
  else()
    add_compile_options( "/MP" )
  endif()
else()
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions -Wall -Wextra -pedantic" )
  set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" )
endif()

if( APPLE )
  add_compile_definitions( GL_SILENCE_DEPRECATION )
endif()

set( CMAKE_POSITION_INDEPENDENT_CODE TRUE )

set( CMAKE_INSTALL_DEFAULT_COMPONENT_NAME Makhber )
set( CPACK_COMPONENT_MAKHBER_REQUIRED TRUE )

get_property( MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )

include( GNUInstallDirs )

if( BUILD_SHARED_LIBS )
  if( MSVC )
    message( FATAL_ERROR "Building Shared Libararies is not supported with MSVC" )
  endif()
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
endif()

option( MAKHBER_SEARCH_FOR_UPDATES "Enable Searching for updates" ON )

option( MAKHBER_DOWNLOAD_LINKS "Enable Download links" OFF )

option( MAKHBER_SCRIPTING_MUPARSER "Enable muParser Scripting" ON )

option( MAKHBER_ORIGIN_IMPORT "Enable importing OriginLab project files" ON )

option( MAKHBER_SCRIPTING_PYTHON "Enable Python Scripting" OFF )

if( MSVC OR APPLE )
  option( MAKHBER_BUNDLE_PYQT5 "Bundle PyQt5 Python module" ON )
else()
  option( MAKHBER_BUNDLE_PYQT5 "Bundle PyQt5 Python module" OFF )
endif()

option( MAKHBER_BUILD_TESTS "Build tests" OFF )

find_package( Qt5
  COMPONENTS
    Core
    Gui
    Widgets
    Svg
    PrintSupport
    Xml
    OpenGL
    LinguistTools
  REQUIRED
  )

find_package( GSL REQUIRED )

find_package( ZLIB "1.2.11" REQUIRED )

find_package( OpenGL COMPONENTS OpenGL REQUIRED )

find_package( muparser REQUIRED )

find_package( Qwt REQUIRED )

if( MAKHBER_SEARCH_FOR_UPDATES OR MAKHBER_DOWNLOAD_LINKS )
  find_package( Qt5 COMPONENTS Network REQUIRED )
endif()

if( MAKHBER_SCRIPTING_PYTHON )

  if( MINGW )
    set( Python3_FIND_REGISTRY LAST )
  endif()

  find_package( Python3 COMPONENTS Interpreter Development REQUIRED )
  find_package( SIP REQUIRED )
  find_package( PyQt REQUIRED )

  if( SIP_VERSION VERSION_GREATER_EQUAL 5 )
    execute_process( COMMAND ${Python3_EXECUTABLE} -c "import pyqtbuild"
      ERROR_VARIABLE PYQT_BUILDER_NOTFOUND
    )
    if( PYQT_BUILDER_NOTFOUND )
      message( FATAL_ERROR "pyqt-builder not found" )
    endif()
  endif()

  if( MSVC )
    set( PYTHON_INSTALL_PATH ${CMAKE_INSTALL_BINDIR} )
  elseif( APPLE )
    set( PYTHON_INSTALL_PATH  "makhber.app/Contents/Resources" )
  else() # MinGW and Linux
    get_filename_component( PYTHON_DIR ${Python3_EXECUTABLE} DIRECTORY )
    get_filename_component( PYTHON_PREFIX "${PYTHON_DIR}/.." REALPATH )
    string( REPLACE "${PYTHON_PREFIX}/" "" PYTHON_INSTALL_PATH ${Python3_SITEARCH} )
  endif()

endif()

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

add_subdirectory( 3rdparty )

add_subdirectory( src )

if( MAKHBER_SCRIPTING_PYTHON )
  add_subdirectory( sip )
endif()

add_subdirectory( fitPlugins )

if( MAKHBER_BUILD_TESTS )
  enable_testing()
  add_subdirectory( test )
endif()

# Documentation
# Developer Documentation
option( MAKHBER_BUILD_CODE_DOCS "Build Code documentation" OFF )

if( MAKHBER_BUILD_CODE_DOCS )
  find_package( Doxygen )
  if( Doxygen_FOUND )
    set( DOXYGEN_FILE_PATTERNS "*.cpp" "*.h" )
    set( DOXYGEN_EXCLUDE_PATTERNS "*/build/*" "*/3rdparty/*" )
    doxygen_add_docs( code_documentation libmakhber makhber )
  endif()
endif()

# User Documentation
set( DOC_FILES
  ChangeLog.md
  README.md
  license.rtf
)

if( WIN32 )
  install( FILES ${DOC_FILES} DESTINATION . )
elseif( APPLE )
  install( FILES ${DOC_FILES} DESTINATION makhber.app/Contents/Resources )
else()
  install( FILES ${DOC_FILES} DESTINATION ${CMAKE_INSTALL_DOCDIR} )
endif()

if( UNIX AND NOT APPLE )
  add_subdirectory( packaging/freedesktop )
endif()

set( CPACK_PACKAGE_NAME "Makhber" )
set( CPACK_PACKAGE_VENDOR "Mehdi Chinoune" )
set( CPACK_PACKAGE_DESCRIPTION "Makhber Installer" )
set( CPACK_PACKAGE_INSTALL_DIRECTORY "Makhber")
set( CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Makhber")
set( CPACK_PACKAGE_EXECUTABLES "makhber;Makhber" )
set( CPACK_CREATE_DESKTOP_LINKS "makhber" )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE" )
set( CPACK_STRIP_FILES ON )

# Source packaging
set( CPACK_SOURCE_IGNORE_FILES "/build/;CMakeLists\.txt\.user*;/\.git/" )

# WIX
set( CPACK_WIX_UPGRADE_GUID "58250F82-661E-451E-9191-4453EF1A5624" )
set( CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/license.rtf" )
set( CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/src/main/icons/makhber.ico" )

# NSIS
set( CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/src/main/icons/makhber.ico" )
set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
set( CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR} )

# Default permissions
set( CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE # 7
  GROUP_READ GROUP_EXECUTE # 5
  WORLD_READ WORLD_EXECUTE # 5
)

# DEB
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "Mehdi Chinoune <mehdi.chinoune@hotmail.com>" )
set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
set( CPACK_DEBIAN_PACKAGE_DEPENDS "python3-pyqt5" "python3" )

# RPM
set( CPACK_RPM_PACKAGE_LICENSE "GPLv2+" )
set( CPACK_RPM_PACKAGE_DESCRIPTION "Makhber RPM Package" )
set( CPACK_RPM_PACKAGE_AUTOREQ ON )

include(CPack)
