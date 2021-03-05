set( RESOURCES
  "appicons.qrc"
  "icons.qrc"
  )

if( WIN32 )
  list( APPEND RESOURCES "scidavis.rc" )
endif()

# Translation Files
file( COPY translations DESTINATION . )
set( TS_FILES
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_de.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_es.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_fr.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_ru.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_ja.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_sv.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_pt-br.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_cs.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_cs-alt.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_pl.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_zh-cn.ts"
  "${CMAKE_CURRENT_BINARY_DIR}/translations/makhber_zh-tw.ts"
  )
qt5_create_translation( QM_FILES "${CMAKE_SOURCE_DIR}/libscidavis" ${TS_FILES} )

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
  set( MACOSX_BUNDLE_ICON_FILE "scidavis.icns" )
  set_target_properties( makhber PROPERTIES
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
endif()

if( MULTI_CONFIG )
  target_include_directories( makhber PRIVATE
    "${CMAKE_BINARY_DIR}/libscidavis/libmakhber_autogen/include_$<CONFIG>"
    )
else()
  target_include_directories( makhber PRIVATE
    "${CMAKE_BINARY_DIR}/libscidavis/libmakhber_autogen/include"
    )
endif()

if( MAKHBER_SCRIPTING_PYTHON )

  if( MULTI_CONFIG )
    file( GENERATE OUTPUT $<CONFIG>/makhberrc.py INPUT makhberrc.py )
    add_custom_command( OUTPUT makhberrc.pyc
      COMMAND ${Python3_EXECUTABLE} -m compileall -b makhberrc.py
      WORKING_DIRECTORY $<CONFIG>
      COMMENT "Compiling python files"
      )
    add_custom_target( generate_pyc_files
      DEPENDS $<CONFIG>/makhberrc.pyc
      )
  else()
    file( COPY makhberrc.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
    add_custom_command( OUTPUT makhberrc.pyc
      COMMAND ${Python3_EXECUTABLE} -m compileall -b makhberrc.py
      COMMENT "Compiling python files"
      )
    add_custom_target( generate_pyc_files
      DEPENDS makhberrc.pyc
      )
  endif()

  add_dependencies( makhber generate_pyc_files )

endif()

if( WIN32 )
  install( TARGETS makhber RUNTIME DESTINATION . )
  install( FILES ${QM_FILES} DESTINATION translations )
elseif( APPLE )
  install( TARGETS makhber BUNDLE DESTINATION . )
  install( FILES ${QM_FILES} DESTINATION makhber.app/Contents/Resources/translations )
  install( FILES icons/makhber.icns DESTINATION makhber.app/Contents/Resources )
else()
  install( TARGETS makhber RUNTIME DESTINATION bin )
  install( FILES ${QM_FILES} DESTINATION share/makhber/translations )
endif()

if( RUNTIME_DIR )
  set( EXCLUDE_DLL "system32/*" "shell32*" "kernel32*" )
  set( MAKHBER_DIR "." )
  set( MAKHBER_EXECUTABLE "${MAKHBER_DIR}/makhber.exe" )
  install( CODE "
    file( GET_RUNTIME_DEPENDENCIES
      RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
      UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
      CONFLICTING_DEPENDENCIES_PREFIX CONFLICTING_DEPENDENCIES
      EXECUTABLES \"\${CMAKE_INSTALL_PREFIX}/${MAKHBER_EXECUTABLE}\"
      DIRECTORIES \"${RUNTIME_DIR}\"
      POST_EXCLUDE_REGEXES ${EXCLUDE_DLL}
      )
    foreach( lib \${RESOLVED_DEPENDENCIES} )
      execute_process(
        COMMAND \"${CMAKE_COMMAND}\" -E
        copy \${lib} \"\${CMAKE_INSTALL_PREFIX}/${MAKHBER_DIR}\"
        )
    endforeach()
    ")
endif()

if( APPLE )
  include(DeployQt)
  macdeployqt(makhber)
endif()

if( WIN32 )
  set( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION . )
  include(DeployQt)
  windeployqt( makhber . )
endif()

# Icons

if( UNIX AND NOT APPLE )
  install( FILES icons/makhber.svg DESTINATION share/icons/hicolor/scalable/apps )

  foreach( res IN ITEMS 16 22 32 48 64 128 )
    install( FILES icons/hicolor-${res}/scidavis.png
      DESTINATION share/icons/hicolor/${res}x${res}/apps )
  endforeach()

  foreach( res IN ITEMS 16 22 32 )
    install( FILES icons/locolor-${res}/scidavis.png
      DESTINATION share/icons/locolor/${res}x${res}/apps )
  endforeach()
endif()

# Application Files
if( UNIX AND NOT APPLE )
  install( FILES makhber.desktop DESTINATION share/applications )
  install( FILES makhber.xml DESTINATION share/mime/packages )
  install( FILES makhber.1 DESTINATION share/man/man1 )
  install( FILES makhber.appdata.xml DESTINATION share/appdata )
  install( FILES x-sciprj.desktop DESTINATION share/mimelnk/application )
endif()

if( MAKHBER_SCRIPTING_PYTHON )
  if( MULTI_CONFIG )
    install( FILES makhberrc.py ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/makhberrc.pyc DESTINATION etc )
  else()
    install( FILES makhberrc.py ${CMAKE_CURRENT_BINARY_DIR}/makhberrc.pyc DESTINATION etc )
  endif()
  install( FILES makhberUtil.py DESTINATION share/makhber )
endif()
