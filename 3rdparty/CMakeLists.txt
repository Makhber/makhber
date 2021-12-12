
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

# minigzip: mini gzip implementation
add_library( minigzip STATIC "minigzip/minigzip.c" )
target_link_libraries( minigzip ZLIB::ZLIB )

# QwtPlot3D
add_subdirectory( qwtplot3d )

# liborigin
if( MAKHBER_ORIGIN_IMPORT )
  add_subdirectory( liborigin )
endif()
