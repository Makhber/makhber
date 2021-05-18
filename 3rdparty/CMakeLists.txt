
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

# minigzip: mini gzip implementation
add_library( minigzip "minigzip/minigzip.c" )
target_link_libraries( minigzip ZLIB::ZLIB )

if( BUILD_SHARED_LIBS )
  set_target_properties( minigzip
    PROPERTIES
      WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
  install( TARGETS minigzip
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
  )
endif()

# Qwt5
add_subdirectory( qwt5-qt5 )

# QwtPlot3D
add_subdirectory( qwtplot3d )

# liborigin
if( MAKHBER_ORIGIN_IMPORT )
  message( STATUS "Importing OriginLab files Enabled" )
  add_subdirectory( liborigin )
else()
  message( STATUS "Importing OriginLab files not Enabled" )
endif()
