
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

# QwtPlot3D
add_subdirectory( qwtplot3d )

# liborigin
if( MAKHBER_ORIGIN_IMPORT )
  if(NOT LIBORIGIN_FOUND)
    add_subdirectory( liborigin )
    if(BUILD_SHARED_LIBS)
      add_library(liborigin::liborigin ALIAS origin_shared)
    else()
      add_library(liborigin::liborigin ALIAS origin_static)
    endif()
  endif()
endif()
