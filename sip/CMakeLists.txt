
if( SIP_VERSION VERSION_GREATER_EQUAL 5 )
  set( makhber_SIP_HDR ${CMAKE_CURRENT_BINARY_DIR}/sip.h )
  set_source_files_properties( ${makhber_SIP_HDR} PROPERTIES GENERATED ON )
  add_custom_command(
    OUTPUT ${makhber_SIP_HDR}
    COMMAND sip-module --sip-h ${CMAKE_CURRENT_SOURCE_DIR}/makhber.sip
    )
  add_custom_target( generate_SIP_HDR DEPENDS ${makhber_SIP_HDR} )
endif()

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
  COMMAND ${SIP_EXECUTABLE} -c . -I${PyQt_INCLUDE_DIRS} ${PyQt_FLAGS}
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

execute_process( COMMAND ${Python3_EXECUTABLE} -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))"
  OUTPUT_VARIABLE _Python3_EXT_SUFFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if( APPLE )
  set_target_properties( makhber_sip PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY $<TARGET_BUNDLE_CONTENT_DIR:makhber>/Resources
  )
else()
  set_target_properties( makhber_sip PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:makhber>
  )
endif()

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
  target_include_directories( makhber_sip PUBLIC "${CMAKE_BINARY_DIR}/libmakhber/libmakhber_autogen/include_$<CONFIG>" )
else()
  target_include_directories( makhber_sip PUBLIC "${CMAKE_BINARY_DIR}/libmakhber/libmakhber_autogen/include" )
endif()
