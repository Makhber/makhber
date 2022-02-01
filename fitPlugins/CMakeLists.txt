
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

add_subdirectory( exp_saturation )

add_subdirectory( explin )

add_subdirectory( fitRational0 )

add_subdirectory( fitRational1 )

add_subdirectory( planck_wavelength )

if( MSVC )
  install( TARGETS exp_saturation explin fitRational0 fitRational1 planck_wavelength
    COMPONENT FitPlugins
    RUNTIME DESTINATION fitPlugins
    )
elseif( APPLE )
  install( TARGETS exp_saturation explin fitRational0 fitRational1 planck_wavelength
    COMPONENT FitPlugins
    LIBRARY DESTINATION makhber.app/Contents/PlugIns/fitPlugins
    )
else()
  install( TARGETS exp_saturation explin fitRational0 fitRational1 planck_wavelength
    COMPONENT FitPlugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}/makhber/plugins
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/makhber/plugins
    )
endif()
