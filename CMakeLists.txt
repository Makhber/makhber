cmake_minimum_required( VERSION 3.16 )

cmake_policy(SET CMP0069 NEW) # INTERPROCEDURAL_OPTIMIZATION
cmake_policy(SET CMP0083 NEW) # POSITION_INDEPENDENT_CODE
cmake_policy(SET CMP0091 NEW) # MSVC_RUNTIME_LIBRARY

set( CMAKE_FIND_PACKAGE_PREFER_CONFIG ON )

project( makhber
  VERSION 0.10.0
  DESCRIPTION "Free application for Visualization and Analysis of Scientific Data"
  HOMEPAGE_URL "https://github.com/Makhber/makhber"
  LANGUAGES CXX C
  )

set( PROJECT_VERSION_SUFFIX "alpha" )

set( PROJECT_RELEASE_DATE "May, 14 2022" )

set( CMAKE_CXX_EXTENSIONS OFF )

set( CMAKE_C_STANDARD 11 )
set( CMAKE_C_STANDARD_REQUIRED TRUE )
set( CMAKE_C_EXTENSIONS OFF )

include( CheckIPOSupported )
check_ipo_supported( RESULT IPO_SUPPORTED OUTPUT IPO_MESSAGE LANGUAGES CXX )
# LTO is broken with MINGW/binutils https://sourceware.org/bugzilla/show_bug.cgi?id=12762
if( MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" )
  set( IPO_SUPPORTED OFF )
endif()

option( MAKHBER_ENABLE_LTO "Enable LTO" ${IPO_SUPPORTED} )

if(MAKHBER_ENABLE_LTO AND NOT MSVC AND NOT APPLE)
  set(BUILD_SHARED_LIBS ON)
endif()

if( MAKHBER_ENABLE_LTO )
  set( CMAKE_INTERPROCEDURAL_OPTIMIZATION ON )
else()
  set( CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF )
endif()

include( CheckCXXCompilerFlag )

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
  check_cxx_compiler_flag("-ftrivial-auto-var-init=pattern" CXX_AUTO_INITIALIZE_PATTERN)
  if( CXX_AUTO_INITIALIZE_PATTERN )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrivial-auto-var-init=pattern" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ftrivial-auto-var-init=pattern" )
  endif()
endif()

if( APPLE )
  add_compile_definitions( GL_SILENCE_DEPRECATION )
endif()

include( CheckPIESupported )
check_pie_supported( OUTPUT_VARIABLE PIE_MESSAGE LANGUAGES CXX )
if( CMAKE_CXX_LINK_PIE_SUPPORTED )
  set( CMAKE_POSITION_INDEPENDENT_CODE TRUE )
endif()

set( CMAKE_INSTALL_DEFAULT_COMPONENT_NAME Makhber )
set( CPACK_COMPONENT_MAKHBER_REQUIRED TRUE )

get_property( MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH} )

include( GNUInstallDirs )

option( MAKHBER_SEARCH_FOR_UPDATES "Enable Searching for updates" ON )

option( MAKHBER_DOWNLOAD_LINKS "Enable Download links" OFF )

option( MAKHBER_SCRIPTING_MUPARSER "Enable muParser Scripting" ON )

option( MAKHBER_ORIGIN_IMPORT "Enable importing OriginLab project files" ON )

option( MAKHBER_SCRIPTING_PYTHON "Enable Python Scripting" OFF )

option( MAKHBER_BUILD_TRANSLATIONS "Build Translations" ON )

option( MAKHBER_BUILD_TESTS "Build tests" OFF )

option( MAKHBER_FORCE_QT5 "Force to build with Qt5" OFF )

if( NOT MAKHBER_FORCE_QT5 )
  find_package( Qt6 6.2 COMPONENTS Core QUIET )
endif()
if( Qt6_FOUND )
  set( QT_VERSION_MAJOR 6 )
  set( MINIMUM_QWT 6.2 )
else()
  find_package( Qt5 5.12 COMPONENTS Core REQUIRED )
  set( QT_VERSION_MAJOR 5 )
  set( MINIMUM_QWT 6.1 )
endif()

find_package( Qt${QT_VERSION_MAJOR}
  COMPONENTS
    Gui
    Widgets
    PrintSupport
    OpenGL
    Svg
  REQUIRED
)

find_package( GSL REQUIRED )

find_package( OpenGL COMPONENTS OpenGL REQUIRED )

find_package( muparser REQUIRED )

find_package( Qwt ${MINIMUM_QWT} REQUIRED )
# Qwt<6.2 couldn't be used with c++20
if(Qwt_VERSION VERSION_LESS 6.2)
  set( CMAKE_CXX_STANDARD 17 )
  set( CMAKE_CXX_STANDARD_REQUIRED TRUE )
else()
  set( CMAKE_CXX_STANDARD 20 )
  set( CMAKE_CXX_STANDARD_REQUIRED FALSE )
endif()

if( MAKHBER_SEARCH_FOR_UPDATES OR MAKHBER_DOWNLOAD_LINKS )
  find_package( Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED )
endif()

if( MAKHBER_BUILD_TRANSLATIONS )
  find_package( Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS LinguistTools )
  if( NOT Qt${QT_VERSION_MAJOR}LinguistTools_FOUND )
    set( MAKHBER_BUILD_TRANSLATIONS OFF CACHE BOOL "Build Translations" FORCE )
  endif()
endif()

if( MAKHBER_SCRIPTING_PYTHON )
  # Python tests segfault with static lib
  if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(BUILD_SHARED_LIBS ON)
  endif()

  if( MSVC OR APPLE )
    option( MAKHBER_BUNDLE_PYQT "Bundle PyQt Python module" ON )
  else()
    option( MAKHBER_BUNDLE_PYQT "Bundle PyQt Python module" OFF )
  endif()

  if( MINGW )
    set( Python3_FIND_REGISTRY LAST )
  endif()

  find_package( Python3 COMPONENTS Interpreter Development REQUIRED )

  if( QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6 )
    set( SIP_VERSION_MIN 5 )
  else()
    set( SIP_VERSION_MIN 4.19 )
  endif()
  find_package( SIP ${SIP_VERSION_MIN} REQUIRED )

  find_package( PyQt${QT_VERSION_MAJOR} REQUIRED )

  if( SIP_VERSION VERSION_GREATER_EQUAL 5 )
    find_package( PyQtBuild REQUIRED )
    if( MULTI_CONFIG )
      set( SIP_BUILD_PATH "${CMAKE_BINARY_DIR}/sip/build" )
    else()
      set( SIP_BUILD_PATH "${CMAKE_BINARY_DIR}/sip/build/makhber" )
    endif()
  else()
    if( BUILD_SHARED_LIBS )
      set( SIP_BUILD_PATH "${CMAKE_BINARY_DIR}/bin" )
    else()
      set( SIP_BUILD_PATH "${CMAKE_BINARY_DIR}/sip" )
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

if( BUILD_SHARED_LIBS )
  set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
  set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
  set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" )
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
set( DOC_FILES ChangeLog.md README.md )

if( CMAKE_VERSION VERSION_LESS "3.22" )
  macro(get_distribution_property RESULT PROPERTY)
    set ( OS_RELEASE_FILE /etc/os-release )
    if ( EXISTS ${OS_RELEASE_FILE} )
      file ( STRINGS ${OS_RELEASE_FILE} _ID_LINE REGEX "^${PROPERTY}=.+" )
      if ( _ID_LINE )
        string ( REGEX REPLACE "^${PROPERTY}=\"?([^\"]+)\"?" "\\1" ${RESULT} "${_ID_LINE}" )
      endif ()
    endif ()
    unset ( OS_RELEASE_FILE )
  endmacro()
endif()

if( MSVC )
  list( APPEND DOC_FILES license.rtf )
  install( FILES ${DOC_FILES} DESTINATION . )
elseif( APPLE )
  install( FILES ${DOC_FILES} DESTINATION makhber.app/Contents/Resources )
else()
  install( FILES ${DOC_FILES} DESTINATION ${CMAKE_INSTALL_DOCDIR} )
  set( LICENSE_FILE LICENSE )
  if( CMAKE_VERSION VERSION_GREATER "3.22" )
    cmake_host_system_information(RESULT DISTRO_ID QUERY DISTRIB_ID)
  else()
    get_distribution_property(DISTRO_ID ID)
  endif()
  if("${DISTRO_ID}" STREQUAL "debian" OR "${DISTRO_ID}" STREQUAL "ubuntu")
    install( FILES ${LICENSE_FILE} DESTINATION ${CMAKE_INSTALL_DOCDIR} )
  else()
    install( FILES ${LICENSE_FILE} DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/makhber )
  endif()
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
set( CPACK_PACKAGE_VERSION ${PROJECT_VERSION}-${PROJECT_VERSION_SUFFIX} )
set( CPACK_SOURCE_IGNORE_FILES "/build/;CMakeLists\.txt\.user*;/\.git/" )

# WIX
set( CPACK_WIX_UPGRADE_GUID "58250F82-661E-451E-9191-4453EF1A5624" )
set( CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/license.rtf" )
set( CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/src/main/icons/makhber.ico" )

# NSIS
set( CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/src/main/icons/makhber.ico" )
set( CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON )
set( CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR} )
set( CPACK_NSIS_MANIFEST_DPI_AWARE ON )

# Default permissions
set( CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
  OWNER_READ OWNER_WRITE OWNER_EXECUTE # 7
  GROUP_READ GROUP_EXECUTE # 5
  WORLD_READ WORLD_EXECUTE # 5
)

# DEB
set( CPACK_DEBIAN_PACKAGE_MAINTAINER "Mehdi Chinoune <mehdi.chinoune@hotmail.com>" )
set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON )
set( MAKHBER_EXTRA_DEB_DEPS "" )
if( MAKHBER_SCRIPTING_PYTHON )
  set( MAKHBER_EXTRA_DEB_DEPS "python3-pyqt${QT_VERSION_MAJOR} (>= ${PyQt${QT_VERSION_MAJOR}_VERSION})" )
endif()
if( MAKHBER_BUILD_TRANSLATIONS )
  if( QT_VERSION_MAJOR VERSION_EQUAL 6 )
    list( APPEND MAKHBER_EXTRA_DEB_DEPS "qt6-translations-l10n (>= ${Qt6_VERSION})")
  else()
    list( APPEND MAKHBER_EXTRA_DEB_DEPS "qttranslations5-l10n (>= ${Qt5_VERSION})")
  endif()
endif()
list( JOIN MAKHBER_EXTRA_DEB_DEPS ", " CPACK_DEBIAN_PACKAGE_DEPENDS )

# RPM
set( CPACK_RPM_PACKAGE_LICENSE "GPLv2+" )
set( CPACK_RPM_PACKAGE_DESCRIPTION "Makhber RPM Package" )
set( CPACK_RPM_PACKAGE_AUTOREQ ON )
set( MAKHBER_EXTRA_RPM_DEPS "" )
if( MAKHBER_SCRIPTING_PYTHON )
  set( MAKHBER_EXTRA_RPM_DEPS "python3-qt${QT_VERSION_MAJOR} >= ${PyQt${QT_VERSION_MAJOR}_VERSION}" )
endif()
if( MAKHBER_BUILD_TRANSLATIONS )
  if("${DISTRO_ID}" STREQUAL "fedora")
    list( APPEND MAKHBER_EXTRA_RPM_DEPS "qt${QT_VERSION_MAJOR}-qttranslations >= ${Qt${QT_VERSION_MAJOR}_VERSION}")
  elseif("${DISTRO_ID}" MATCHES "opensuse")
    if( QT_VERSION_MAJOR VERSION_EQUAL 6 )
      list( APPEND MAKHBER_EXTRA_RPM_DEPS "qt6-translations >= ${Qt6_VERSION}")
    else()
      list( APPEND MAKHBER_EXTRA_RPM_DEPS "libqt5-qttranslations >= ${Qt5_VERSION}")
    endif()
  endif()
endif()
list( JOIN MAKHBER_EXTRA_RPM_DEPS ", " CPACK_RPM_PACKAGE_REQUIRES )

# DMG
set( CPACK_DMG_FILESYSTEM "APFS" )

include(CPack)
