
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

# QwtPlot3D
add_subdirectory( qwtplot3d )

# liborigin
if( MAKHBER_ORIGIN_IMPORT )
  add_subdirectory( liborigin )
endif()
