
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

# minigzip: mini gzip implementation
add_library( minigzip "minigzip/minigzip.c" )
target_link_libraries( minigzip ZLIB::ZLIB )

# Qwt5
add_subdirectory( qwt5-qt5 )

# QwtPlot3D
add_subdirectory( qwtplot3d )

# liborigin
if( SCIDAVIS_ORIGIN_IMPORT )
  message( STATUS "Importing OriginLab files Enabled" )
  add_subdirectory( liborigin )
else()
  message( STATUS "Importing OriginLab files not Enabled" )
endif()
