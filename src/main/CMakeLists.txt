set( RESOURCES "appicons.qrc" "icons.qrc" )

if( WIN32 )
  list( APPEND RESOURCES "makhber.rc" )
endif()

add_executable( makhber
  WIN32 MACOSX_BUNDLE
  "main.cpp"
  "${RESOURCES}"
)

target_link_libraries( makhber libmakhber )

if( MAKHBER_BUILD_TRANSLATIONS )
  file( GLOB TS_FILES "${CMAKE_SOURCE_DIR}/translations/*.ts" )
  if( QT_VERSION_MAJOR VERSION_GREATER_EQUAL 6 )
    qt_add_translations( makhber
      TS_FILES ${TS_FILES}
      QM_FILES_OUTPUT_VARIABLE QM_FILES
      LUPDATE_OPTIONS "-no-obsolete"
    )
  else()
    qt5_create_translation( QM_FILES "${CMAKE_SOURCE_DIR}/src" ${TS_FILES} OPTIONS "-no-obsolete" )
    target_sources( makhber PRIVATE "${QM_FILES}" )
  endif()
  add_custom_command( TARGET makhber
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E
      make_directory "$<TARGET_FILE_DIR:makhber>/translations"
    COMMAND ${CMAKE_COMMAND} -E
      copy ${QM_FILES} "$<TARGET_FILE_DIR:makhber>/translations/"
    COMMENT "Copying Translation files..."
  )
endif()

if( APPLE )
  set( MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME} )
  set( MACOSX_BUNDLE_VERSION ${PROJECT_VERSION} )
  set( MACOSX_BUNDLE_ICON_FILE "makhber.icns" )
  set_target_properties( makhber PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
  )
endif()

if( MAKHBER_SCRIPTING_PYTHON )
  set( MAKHBER_PYTHON_FILES
    "${CMAKE_SOURCE_DIR}/src/scripting/makhberrc.py"
    "${CMAKE_SOURCE_DIR}/src/scripting/makhberUtil.py"
  )
  file( COPY ${MAKHBER_PYTHON_FILES} DESTINATION . )
  # Copy *.py files to run python tests, and compile them for installation
  add_custom_command( TARGET makhber
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${MAKHBER_PYTHON_FILES} "$<TARGET_FILE_DIR:makhber>/"
    COMMAND ${Python3_EXECUTABLE} -m compileall -l ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Copying and Compiling Python files..."
  )
endif()

install( TARGETS makhber
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION .
)

if( APPLE )
  install( FILES icons/makhber.icns DESTINATION makhber.app/Contents/Resources )
endif()

set( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR} )
include(DeployQt)
if( MSVC )
  windeployqt( makhber ${CMAKE_INSTALL_BINDIR} )
elseif( APPLE )
  install( CODE "
    execute_process(
      COMMAND ${MACDEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/makhber.app -always-overwrite
    )"
  )
endif()

if( WIN32 AND RUNTIME_DIRS )
  if( MAKHBER_SCRIPTING_PYTHON )
    list( APPEND RUNTIME_DIRS ${Python3_RUNTIME_LIBRARY_DIRS} )
  endif()
  foreach( dir ${RUNTIME_DIRS} )
    file( TO_CMAKE_PATH ${dir} _dir_cmake )
    set( RUNTIME_DIRS_CMAKE ${_dir_cmake} ${RUNTIME_DIRS_CMAKE} )
  endforeach()
  list( JOIN RUNTIME_DIRS_CMAKE "\" \"" _RUNTIME_DIRS_CMAKE )
  set( MAKHBER_EXECUTABLE "${CMAKE_INSTALL_BINDIR}/makhber.exe" )
  install( CODE "
    file( GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
      UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
      CONFLICTING_DEPENDENCIES_PREFIX CONFLICTING_DEPENDENCIES
      EXECUTABLES \${CMAKE_INSTALL_PREFIX}/${MAKHBER_EXECUTABLE}
      DIRECTORIES \"${_RUNTIME_DIRS_CMAKE}\"
      PRE_EXCLUDE_REGEXES api-ms-win-*
      POST_EXCLUDE_REGEXES system32*
    )
    message( \"RESOLVED_DEPENDENCIES :\" )
    foreach( dll \${RESOLVED_DEPENDENCIES} )
      message( \"    \${dll}\" )
      execute_process(
        COMMAND \"${CMAKE_COMMAND}\" -E
        copy \${dll} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
      )
    endforeach()
    message( \"UNRESOLVED_DEPENDENCIES :\" )
    foreach( dll \${UNRESOLVED_DEPENDENCIES} )
      message( \"    \${dll}\" )
    endforeach()
    "
  )
  find_package( OpenSSL 1.1 REQUIRED )
  if( OPENSSL_VERSION VERSION_LESS 3 )
    set( SSL_VERSION "1_1" )
  else()
    set( SSL_VERSION "3" )
  endif()
  find_file( OPENSSL_LIBSSL_DLL "libssl-${SSL_VERSION}-x64.dll" PATHS ${RUNTIME_DIRS} REQUIRED )
  find_file( OPENSSL_LIBCRYPTO_DLL "libcrypto-${SSL_VERSION}-x64.dll" PATHS ${RUNTIME_DIRS} REQUIRED )
  install( CODE "
    execute_process(
      COMMAND \"${CMAKE_COMMAND}\" -E
      copy ${OPENSSL_LIBSSL_DLL} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
      COMMAND \"${CMAKE_COMMAND}\" -E
      copy ${OPENSSL_LIBCRYPTO_DLL} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
    )"
  )
endif()

# Install Translations
if( MAKHBER_BUILD_TRANSLATIONS )
  if( MSVC )
    install( FILES ${QM_FILES}
      COMPONENT Translations
      DESTINATION translations
    )
  elseif( APPLE )
    install( FILES ${QM_FILES}
      COMPONENT Translations
      DESTINATION makhber.app/Contents/Resources/translations
    )
  else()
    install( FILES ${QM_FILES}
      COMPONENT Translations
      DESTINATION ${CMAKE_INSTALL_DATADIR}/makhber/translations
    )
  endif()
endif()
