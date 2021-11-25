set( RESOURCES "appicons.qrc" "icons.qrc" )

if( WIN32 )
  list( APPEND RESOURCES "makhber.rc" )
endif()

file( GLOB TS_FILES "${CMAKE_SOURCE_DIR}/translations/*.ts" )
qt5_create_translation( QM_FILES "${CMAKE_SOURCE_DIR}/src" ${TS_FILES} OPTIONS "-no-obsolete" )

add_executable( makhber
  WIN32 MACOSX_BUNDLE
  "main.cpp"
  "${RESOURCES}"
  "${QM_FILES}"
)

target_link_libraries( makhber libmakhber )

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

  file( COPY
    "${CMAKE_SOURCE_DIR}/src/scripting/makhberrc.py"
    "${CMAKE_SOURCE_DIR}/src/scripting/makhberUtil.py"
    DESTINATION .
  )
  add_custom_command( TARGET makhber
    POST_BUILD
    COMMAND ${Python3_EXECUTABLE} -m compileall -l ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling Python files"
  )

endif()

if( APPLE )
  install( TARGETS makhber BUNDLE DESTINATION . )
  install( FILES icons/makhber.icns DESTINATION makhber.app/Contents/Resources )
else()
  install( TARGETS makhber RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
endif()

if( MSVC )
  set( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR} )
  include(DeployQt)
  windeployqt( makhber ${CMAKE_INSTALL_BINDIR} )
endif()

if( RUNTIME_DIRS )
  foreach( dir ${RUNTIME_DIRS} )
    file( TO_CMAKE_PATH ${dir} _dir_cmake )
    set( RUNTIME_DIRS_CMAKE ${_dir_cmake} ${RUNTIME_DIRS_CMAKE} )
  endforeach()
  list( JOIN RUNTIME_DIRS_CMAKE " " _RUNTIME_DIRS_CMAKE )
  set( EXCLUDE_DLLS "system32*" )
  set( MAKHBER_EXECUTABLE "${CMAKE_INSTALL_BINDIR}/makhber.exe" )
  install( CODE "
    file( GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
      UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
      CONFLICTING_DEPENDENCIES_PREFIX CONFLICTING_DEPENDENCIES
      EXECUTABLES \${CMAKE_INSTALL_PREFIX}/${MAKHBER_EXECUTABLE}
      DIRECTORIES ${_RUNTIME_DIRS_CMAKE} ${Python3_RUNTIME_LIBRARY_DIRS}
      PRE_EXCLUDE_REGEXES api-ms-win-*
      POST_EXCLUDE_REGEXES ${EXCLUDE_DLLS}
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
  find_package( OpenSSL REQUIRED )
  find_file( OPENSSL_LIBSSL_DLL "libssl-1_1-x64.dll" PATHS ${RUNTIME_DIRS} REQUIRED )
  find_file( OPENSSL_LIBCRYPTO_DLL "libcrypto-1_1-x64.dll" PATHS ${RUNTIME_DIRS} REQUIRED )
  install( CODE "
    execute_process(
      COMMAND \"${CMAKE_COMMAND}\" -E
      copy ${OPENSSL_LIBSSL_DLL} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
      COMMAND \"${CMAKE_COMMAND}\" -E
      copy ${OPENSSL_LIBCRYPTO_DLL} \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
    )"
  )
endif()

if( APPLE )
  include(DeployQt)
  install( CODE "
    execute_process(
      COMMAND ${MACDEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/makhber.app -always-overwrite
    )"
  )
endif()

# Install Translations
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
