cmake_minimum_required( VERSION 3.16 )

cmake_policy(SET CMP0091 NEW)

project( makhber
  VERSION 0.9.0
  DESCRIPTION "Makhber is a free application for Analysis and Visualization of Scientific Data."
  HOMEPAGE_URL "https://github.com/Makhber/makhber"
  LANGUAGES CXX C
  )

math( EXPR PROJECT_VERSION_NO "${PROJECT_VERSION_MAJOR} * 65536 + \
  ${PROJECT_VERSION_MINOR} * 256 + ${PROJECT_VERSION_PATCH}" )

string( TIMESTAMP PROJECT_BUILD_DATE "%b, %d %Y" )

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

get_property( MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )

option( MAKHBER_SEARCH_FOR_UPDATES "Enable Searching for updates" ON )

option( MAKHBER_DOWNLOAD_LINKS "Enable Download links" ON )

option( MAKHBER_SCRIPTING_MUPARSER "Enable muParser Scripting" ON )

option( MAKHBER_ORIGIN_IMPORT "Enable importing OriginLab project files" ON )

option( MAKHBER_SCRIPTING_PYTHON "Enable Python Scripting" OFF )

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
endif()

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

add_subdirectory( 3rdparty )

add_subdirectory( libmakhber )

if( MAKHBER_SCRIPTING_PYTHON )
  add_subdirectory( sip )
endif()

add_subdirectory( makhber )

add_subdirectory( fitPlugins )

if( MAKHBER_BUILD_TESTS )
  enable_testing()
  add_subdirectory( test )
endif()

# Documentation
set( DOC_FILES
  ChangeLog.md
  README.md
  license.rtf
  )

option( MAKHBER_BUILD_CODE_DOCS "Build Code documentation" OFF )

if( MAKHBER_BUILD_CODE_DOCS )
  find_package( Doxygen )
  if( Doxygen_FOUND )
    set( DOXYGEN_FILE_PATTERNS "*.cpp" "*.h" )
    set( DOXYGEN_EXCLUDE_PATTERNS "*/build/*" "*/3rdparty/*" )
    doxygen_add_docs( code_documentation libmakhber makhber )
  endif()
endif()

if( WIN32 )
  install( FILES ${DOC_FILES} DESTINATION . )
elseif( APPLE )
  install( FILES ${DOC_FILES} DESTINATION makhber.app/Contents/Resources )
else()
  install( FILES ${DOC_FILES} DESTINATION share/doc/makhber )
endif()

set( CPACK_PACKAGE_NAME "Makhber" )
set( CPACK_PACKAGE_VENDOR "Mehdi Chinoune" )
set( CPACK_PACKAGE_DESCRIPTION "Makhber Installer" )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE" )
set( CPACK_PACKAGE_EXECUTABLES "makhber;Makhber" )
set( CPACK_CREATE_DESKTOP_LINKS "makhber" )
set( CPACK_STRIP_FILES ON )

# Source packaging
set( CPACK_SOURCE_IGNORE_FILES "/build/;CMakeLists\.txt\.user*;/\.git/" )

# WIX
set( CPACK_WIX_UPGRADE_GUID "58250F82-661E-451E-9191-4453EF1A5624" )
set( CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/license.rtf" )
set( CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/makhber/icons/makhber.ico" )

# NSIS
set( CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/makhber/icons/makhber.ico" )
set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
set( CPACK_NSIS_EXECUTABLES_DIRECTORY . )

include(CPack)
