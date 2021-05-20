
set( appid "com.github.makhber.Makhber" )

# Icons
install( FILES ${appid}.svg
  DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

foreach( res IN ITEMS 16 22 32 48 64 128 )
  install( FILES hicolor-${res}/${appid}.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${res}x${res}/apps
  )
endforeach()

foreach( res IN ITEMS 16 22 32 )
  install( FILES locolor-${res}/${appid}.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/locolor/${res}x${res}/apps
  )
endforeach()

# Application Files
install( FILES ${appid}.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
install( FILES ${appid}.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo )
install( FILES ${appid}.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages )
install( FILES makhber.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )
