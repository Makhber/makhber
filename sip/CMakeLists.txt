
execute_process( COMMAND ${Python3_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))"
  OUTPUT_VARIABLE _Python3_EXT_SUFFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if( SIP_VERSION VERSION_GREATER_EQUAL 5 )
  set( makhber_SIP_HDR ${CMAKE_CURRENT_BINARY_DIR}/sip.h )
  set_source_files_properties( ${makhber_SIP_HDR} PROPERTIES GENERATED ON )
  add_custom_command(
    OUTPUT ${makhber_SIP_HDR}
    COMMAND ${SIP_Module_EXECUTABLE} --sip-h --abi-version 12 ${CMAKE_CURRENT_SOURCE_DIR}/makhber.sip
    )
  add_custom_target( generate_SIP_HDR DEPENDS ${makhber_SIP_HDR} )
endif()

if( SIP_VERSION VERSION_LESS 5 )

  set( makhber_PyQt_HDR ${CMAKE_CURRENT_BINARY_DIR}/sipAPImakhber.h )

  set( makhber_PyQt_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhbercmodule.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberApplicationWindow.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberGraph.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberArrowMarker.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberImageMarker.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberLegend.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberMultiLayer.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberTable.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberMatrix.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberMyWidget.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberScriptEdit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberNote.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberPythonScript.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberPythonScripting.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberFolder.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberExponentialFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberTwoExpFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberThreeExpFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberSigmoidalFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberGaussAmpFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberLorentzFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberNonLinearFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberPluginFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberMultiPeakFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberPolynomialFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberLinearFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberGaussFit.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberFilter.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberDifferentiation.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberIntegration.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberInterpolation.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberSmoothFilter.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberFFTFilter.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberFFT.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberCorrelation.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberConvolution.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberDeconvolution.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberAbstractAspect.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberColumn.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQwtSymbol.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQwtPlotCurve.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQwtPlot.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberGrid.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQList0100QDateTime.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQList0101Folder.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQList0101MyWidget.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipmakhberQList0101QwtPlotCurve.cpp
  )

  set_source_files_properties( ${makhber_PyQt_SRCS} ${makhber_PyQt_HDR} PROPERTIES GENERATED ON )

  add_custom_command(
    OUTPUT ${makhber_PyQt_HDR} ${makhber_PyQt_SRCS}
    COMMAND ${SIP_Legacy_EXECUTABLE} -c . -I${PyQt_INCLUDE_DIRS} ${PyQt_FLAGS}
      ${CMAKE_CURRENT_SOURCE_DIR}/makhber.sip
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/makhber.sip
  )

  add_custom_target(generate_PyQt_source
    DEPENDS ${makhber_PyQt_HDR} ${makhber_PyQt_SRCS}
  )

  python3_add_library( makhber_sip
    ${makhber_PyQt_HDR}
    ${makhber_PyQt_SRCS}
  )

  set_target_properties( makhber_sip PROPERTIES
    OUTPUT_NAME "makhber"
    PREFIX ""
    SUFFIX ${_Python3_EXT_SUFFIX}
  )

  add_dependencies( makhber_sip generate_PyQt_source )

  if( SIP_VERSION VERSION_GREATER_EQUAL 5 )
    add_dependencies( makhber_sip generate_SIP_HDR )
  endif()

  target_link_libraries( makhber_sip PRIVATE libmakhber )

  if( MULTI_CONFIG )
    target_include_directories( makhber_sip PUBLIC "${CMAKE_BINARY_DIR}/src/libmakhber_autogen/include_@CONFIG@" )
  else()
    target_include_directories( makhber_sip PUBLIC "${CMAKE_BINARY_DIR}/src/libmakhber_autogen/include" )
  endif()

  install( TARGETS makhber_sip
    COMPONENT Python
    LIBRARY DESTINATION ${PYTHON_INSTALL_PATH}
  )

else()

  get_target_property( QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION )

  macro( get_lib_properties LIB LIB_NAME LIB_DIR LIB_INCLUDE_DIRS )
    get_target_property( IS_IMPORTED ${LIB} IMPORTED )
    if( WIN32 )
      get_target_property( _LIB_LOCATION ${LIB} IMPORTED_IMPLIB_RELEASE )
      if( NOT _LIB_LOCATION )
        get_target_property( _LIB_LOCATION ${LIB} IMPORTED_IMPLIB )
      endif()
    endif()
    if( NOT _LIB_LOCATION OR NOT WIN32 )
      get_target_property( _LIB_LOCATION ${LIB} LOCATION )
    endif()
    if( _LIB_LOCATION )
      get_filename_component( _LIB_NAME ${_LIB_LOCATION} NAME_WE )
      string( REGEX REPLACE "^lib" "" LIB_NAME ${_LIB_NAME} )
      get_filename_component( LIB_DIR ${_LIB_LOCATION} DIRECTORY )
    else()
      message( WARNING "Couldn't find location for ${LIB}" )
    endif()
    get_target_property( LIB_INCLUDE_DIRS ${LIB} INTERFACE_INCLUDE_DIRECTORIES )
    if( NOT LIB_INCLUDE_DIRS )
      message( WARNING "Couldn't find include directories for ${LIB}" )
      set( LIB_INCLUDE_DIRS "" )
    endif()
  endmacro()

  set( qt_libs Core Gui Widgets PrintSupport Svg )
  if( MAKHBER_SEARCH_FOR_UPDATES OR MAKHBER_DOWNLOAD_LINKS )
    list( APPEND qt_libs Network )
  endif()
  foreach( lib ${qt_libs} )
    get_lib_properties( Qt5::${lib} LIB_NAME LIB_DIR LIB_INCLUDE_DIRS )
    if( NOT APPLE )
      string( JOIN "\n," SIP_LINK_LIBS \"${LIB_NAME}\" ${SIP_LINK_LIBS} )
      set( SIP_LINK_DIRS ${SIP_LINK_DIRS} ${LIB_DIR} )
    endif()
    set( SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} ${LIB_INCLUDE_DIRS} )
  endforeach()

  set( libs muparser::muparser ZLIB::ZLIB Qwt::Qwt )
  foreach( lib ${libs} )
    get_lib_properties( ${lib} LIB_NAME LIB_DIR LIB_INCLUDE_DIRS )
    string( JOIN "\n," SIP_LINK_LIBS \"${LIB_NAME}\" ${SIP_LINK_LIBS} )
    set( SIP_LINK_DIRS ${SIP_LINK_DIRS} ${LIB_DIR} )
    set( SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} ${LIB_INCLUDE_DIRS} )
  endforeach()

  if( APPLE )
    set( libs ${GSL_CBLAS_LIBRARY} ${GSL_LIBRARY} )
  else()
    set( libs ${GSL_CBLAS_LIBRARY} ${GSL_LIBRARY} ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY} )
    set( SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} )
  endif()
  foreach( lib ${libs} )
    get_filename_component( _LIB_NAME ${lib} NAME_WE )
    string( REGEX REPLACE "^lib" "" LIB_NAME ${_LIB_NAME} )
    string( JOIN "\n," SIP_LINK_LIBS \"${LIB_NAME}\" ${SIP_LINK_LIBS} )
    get_filename_component( LIB_DIR ${lib} DIRECTORY )
    if( LIB_DIR )
      set( SIP_LINK_DIRS ${SIP_LINK_DIRS} ${LIB_DIR} )
    endif()
  endforeach()
  set( SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} )

  if( GL2PS_LIBRARY )
    get_filename_component( _LIB_NAME ${GL2PS_LIBRARY} NAME_WE )
    string( REGEX REPLACE "^lib" "" LIB_NAME ${_LIB_NAME} )
    string( JOIN "\n," SIP_LINK_LIBS \"${LIB_NAME}\" ${SIP_LINK_LIBS} )
    get_filename_component( LIB_DIR ${GL2PS_LIBRARY} DIRECTORY )
    if( LIB_DIR )
      set( SIP_LINK_DIRS ${SIP_LINK_DIRS} ${LIB_DIR} )
    endif()
    set( SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} ${GL2PS_INCLUDE_DIR} )
  endif()

  string( JOIN "\n," SIP_LINK_LIBS
    \"makhber\"
    \"minigzip\"
    \"origin\"
    \"qwtplot3d\"
    ${SIP_LINK_LIBS}
    )

  if( MULTI_CONFIG )
    set( SIP_LINK_DIRS
      ${SIP_LINK_DIRS}
      ${CMAKE_BINARY_DIR}/src/@CONFIG@
      ${CMAKE_BINARY_DIR}/3rdparty/@CONFIG@
      ${CMAKE_BINARY_DIR}/3rdparty/liborigin/@CONFIG@
      ${CMAKE_BINARY_DIR}/3rdparty/qwtplot3d/@CONFIG@
      )
  else()
    set( SIP_LINK_DIRS
      ${SIP_LINK_DIRS}
      ${CMAKE_BINARY_DIR}/src
      ${CMAKE_BINARY_DIR}/3rdparty
      ${CMAKE_BINARY_DIR}/3rdparty/liborigin
      ${CMAKE_BINARY_DIR}/3rdparty/qwtplot3d
      )
  endif()
  list( REMOVE_DUPLICATES SIP_LINK_DIRS )
  string( JOIN "\"\n,\"" SIP_LINK_DIRS ${SIP_LINK_DIRS} )

  foreach( lib libmakhber qwtplot3d )
    get_target_property( INCLUDE_DIRS ${lib} INTERFACE_INCLUDE_DIRECTORIES )
    set( SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} ${INCLUDE_DIRS} )
  endforeach()
  list( REMOVE_DUPLICATES SIP_INCLUDE_DIRS )
  string( JOIN "\"\n,\"" SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} )
  string( REPLACE "$<CONFIG>" "@CONFIG@" SIP_INCLUDE_DIRS ${SIP_INCLUDE_DIRS} )

  get_target_property( COMPILE_DEFINITIONS libmakhber INTERFACE_COMPILE_DEFINITIONS )
  set( SIP_COMPILE_DEFINITIONS ${SIP_COMPILE_DEFINITIONS} ${COMPILE_DEFINITIONS} )
  if( MSVC )
    set( SIP_COMPILE_DEFINITIONS ${SIP_COMPILE_DEFINITIONS}  "_USE_MATH_DEFINES" )
  endif()
  list( TRANSFORM SIP_COMPILE_DEFINITIONS REPLACE "\"" "\\\\\"" )
  string( JOIN "\"\n,\"" SIP_COMPILE_DEFINITIONS ${SIP_COMPILE_DEFINITIONS} )

  if( MULTI_CONFIG )
    configure_file( pyproject.toml.in pyproject_tmp.toml @ONLY )
    add_custom_target( pyproject_toml ALL
      BYPRODUCTS pyproject.toml
      COMMAND ${CMAKE_COMMAND} -DCONFIG=$<CONFIG> -P ${CMAKE_CURRENT_SOURCE_DIR}/configure_toml.cmake
      DEPENDS libmakhber
      COMMENT "Generating pyproject.toml file...\n"
    )
  else()
    configure_file( pyproject.toml.in pyproject.toml @ONLY )
  endif()

  if( MSVC )
    set( module_file ${CMAKE_CURRENT_BINARY_DIR}/build/makhber/makhber.pyd )
  else()
    set( module_file ${CMAKE_CURRENT_BINARY_DIR}/build/makhber/makhber${_Python3_EXT_SUFFIX} )
  endif()

  if( NOT MSVC )
    set( QMAKE_CXX "${CMAKE_CXX_COMPILER}" )
  elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
    set( QMAKE_CXX "clang-cl" )
  else()
    set( QMAKE_CXX "cl" )
  endif()

  include( ProcessorCount )
  ProcessorCount( ncpu )

  add_custom_command( OUTPUT ${module_file}
    COMMAND ${SIP_Build_EXECUTABLE} --verbose --jobs ${ncpu} --concatenate $<IF:$<BOOL:${WIN32}>,1,${ncpu}>
      --qmake-setting QMAKE_CXX=${QMAKE_CXX}
      --qmake-setting $<IF:$<CONFIG:Debug>,"CONFIG+=debug","CONFIG+=release">
    DEPENDS "makhber.sip" libmakhber ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml
    COMMAND_EXPAND_LISTS
    COMMENT "Building Makhber Python Module...\n"
  )

  add_custom_target( makhber_sip
    ALL
    DEPENDS ${module_file}
  )
  if( MULTI_CONFIG )
    add_dependencies( makhber_sip pyproject_toml )
  endif()

  install( FILES ${module_file}
    COMPONENT Python
    DESTINATION ${PYTHON_INSTALL_PATH}
  )

endif()
