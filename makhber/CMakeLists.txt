set( RESOURCES
  "appicons.qrc"
  "icons.qrc"
  )

if( WIN32 )
  list( APPEND RESOURCES "makhber.rc" )
endif()

# Translation Files
set( TS_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_cs.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_de.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_es.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_ja.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_pl.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_pt-br.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_ru.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_sv.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_zh-cn.ts"
  "${CMAKE_CURRENT_SOURCE_DIR}/translations/makhber_zh-tw.ts"
  )
qt5_create_translation( QM_FILES "${CMAKE_SOURCE_DIR}/libmakhber" ${TS_FILES} )

add_executable( makhber
  WIN32 MACOSX_BUNDLE
  "src/main.cpp"
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

  file( COPY "makhberrc.py" "makhberUtil.py" DESTINATION . )
  add_custom_command( TARGET makhber
    POST_BUILD
    COMMAND ${Python3_EXECUTABLE} -m compileall -l ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Compiling Python files"
  )

endif()

if( WIN32 )
  install( TARGETS makhber RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
  install( FILES ${QM_FILES}
    COMPONENT Translations
    DESTINATION translations
  )
elseif( APPLE )
  install( TARGETS makhber BUNDLE DESTINATION . )
  install( FILES ${QM_FILES}
    COMPONENT Translations
    DESTINATION makhber.app/Contents/Resources/translations
  )
  install( FILES icons/makhber.icns DESTINATION makhber.app/Contents/Resources )
else()
  install( TARGETS makhber RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )
  install( FILES ${QM_FILES}
    COMPONENT Translations
    DESTINATION ${CMAKE_INSTALL_DATADIR}/makhber/translations
  )
endif()

if( MSVC )
  set( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${CMAKE_INSTALL_BINDIR} )
  include(DeployQt)
  windeployqt( makhber ${CMAKE_INSTALL_BINDIR} )
endif()

if( RUNTIME_DIR )
  file( TO_CMAKE_PATH ${RUNTIME_DIR} RUNTIME_DIR_CMAKE )
  set( EXCLUDE_DLLS "system32*" )
  set( MAKHBER_EXECUTABLE "${CMAKE_INSTALL_BINDIR}/makhber.exe" )
  install( CODE "
    file( GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
      UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
      CONFLICTING_DEPENDENCIES_PREFIX CONFLICTING_DEPENDENCIES
      EXECUTABLES \"\${CMAKE_INSTALL_PREFIX}/${MAKHBER_EXECUTABLE}\"
      DIRECTORIES \"${RUNTIME_DIR_CMAKE}\" \"${Python3_RUNTIME_LIBRARY_DIRS}\"
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
    ")
endif()

if( APPLE )
  include(DeployQt)
  install( CODE "
    execute_process(
      COMMAND ${MACDEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/makhber.app -always-overwrite
    )
  ")
endif()

# Icons

if( UNIX AND NOT APPLE )
  install( FILES icons/com.github.makhber.Makhber.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps )

  foreach( res IN ITEMS 16 22 32 48 64 128 )
    install( FILES icons/hicolor-${res}/com.github.makhber.Makhber.png
      DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${res}x${res}/apps )
  endforeach()

  foreach( res IN ITEMS 16 22 32 )
    install( FILES icons/locolor-${res}/com.github.makhber.Makhber.png
      DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/locolor/${res}x${res}/apps )
  endforeach()
endif()

# Application Files
if( UNIX AND NOT APPLE )
  install( FILES com.github.makhber.Makhber.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
  install( FILES makhber.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )
  install( FILES com.github.makhber.Makhber.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/appdata )
  install( FILES x-sciprj.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/mimelnk/application )
endif()

if( MAKHBER_SCRIPTING_PYTHON )

  install( FILES
    makhberrc.py
    makhberUtil.py
    COMPONENT Python
    DESTINATION ${PYTHON_INSTALL_PATH}
  )
  install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/__pycache__"
    COMPONENT Python
    DESTINATION "${PYTHON_INSTALL_PATH}"
  )

  if( MAKHBER_BUNDLE_PYQT5 )
    install( CODE
      COMPONENT Python
      "execute_process(   
        COMMAND ${Python3_EXECUTABLE} -m pip install pyqt5 pyqt5-sip --no-deps
        --target=\${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_PATH}
      )")
  endif()

  if( MSVC )
    install( CODE 
      COMPONENT Python
      "file( DOWNLOAD
        https://www.python.org/ftp/python/${Python3_VERSION}/python-${Python3_VERSION}-embed-amd64.zip
        python_embed.zip
      )
      file( ARCHIVE_EXTRACT
        INPUT python_embed.zip
        DESTINATION \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
        VERBOSE
      )
      ")
  endif()

endif()
