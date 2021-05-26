###################### HEADERS ##############################################
set( HEADERS
  "src/ApplicationWindow.h"
  "src/ArrowMarker.h"
  "src/AssociationsDialog.h"
  "src/AxesDialog.h"
  "src/Bar.h"
  "src/BoxCurve.h"
  "src/CanvasPicker.h"
  "src/ColorBox.h"
  "src/ColorButton.h"
  "src/ColorMapEditor.h"
  "src/Cone3D.h"
  "src/ConfigDialog.h"
  "src/Convolution.h"
  "src/Correlation.h"
  "src/CurveRangeDialog.h"
  "src/CurvesDialog.h"
  "src/DataPickerTool.h"
  "src/DataPointPicker.h"
  "src/DataSetDialog.h"
  "src/Differentiation.h"
  "src/ErrDialog.h"
  "src/ExpDecayDialog.h"
  "src/ExponentialFit.h"
  "src/ExportDialog.h"
  "src/ExtensibleFileDialog.h"
  "src/FFT.h"
  "src/FFTDialog.h"
  "src/FFTFilter.h"
  "src/Filter.h"
  "src/FilterDialog.h"
  "src/FindDialog.h"
  "src/Fit.h"
  "src/FitDialog.h"
  "src/Folder.h"
  "src/FunctionCurve.h"
  "src/FunctionDialog.h"
  "src/Graph.h"
  "src/Graph3D.h"
  "src/Grid.h"
  "src/IconLoader.h"
  "src/ImageDialog.h"
  "src/ImageExportDialog.h"
  "src/ImageMarker.h"
  "src/ImportASCIIDialog.h"
  "src/IntDialog.h"
  "src/Integration.h"
  "src/Interpolation.h"
  "src/InterpolationDialog.h"
  "src/LayerDialog.h"
  "src/Legend.h"
  "src/LineDialog.h"
  "src/LineProfileTool.h"
  "src/MakhberObject.h"
  "src/Matrix.h"
  "src/MuParserScript.h"
  "src/MuParserScripting.h"
  "src/MultiLayer.h"
  "src/MultiPeakFit.h"
  "src/MultiPeakFitTool.h"
  "src/MyParser.h"
  "src/MyWidget.h"
  "src/NonLinearFit.h"
  "src/Note.h"
  "src/OpenProjectDialog.h"
  "src/PatternBox.h"
  "src/PenWidget.h"
  "src/Plot.h"
  "src/Plot3DDialog.h"
  "src/PlotCurve.h"
  "src/PlotDialog.h"
  "src/PlotEnrichement.h"
  "src/PlotToolInterface.h"
  "src/PlotWizard.h"
  "src/PluginFit.h"
  "src/PolynomFitDialog.h"
  "src/PolynomialFit.h"
  "src/QStringStdString.h"
  "src/QwtBarCurve.h"
  "src/QwtErrorPlotCurve.h"
  "src/QwtHistogram.h"
  "src/QwtPieCurve.h"
  "src/RangeSelectorTool.h"
  "src/RenameWindowDialog.h"
  "src/ScaleDraw.h"
  "src/ScalePicker.h"
  "src/ScreenPickerTool.h"
  "src/Script.h"
  "src/ScriptEdit.h"
  "src/ScriptingEnv.h"
  "src/ScriptingLangDialog.h"
  "src/SelectionMoveResizer.h"
  "src/SigmoidalFit.h"
  "src/SmoothCurveDialog.h"
  "src/SmoothFilter.h"
  "src/Spectrogram.h"
  "src/SurfaceDialog.h"
  "src/SymbolBox.h"
  "src/SymbolDialog.h"
  "src/Table.h"
  "src/TableStatistics.h"
  "src/TeXTableExportDialog.h"
  "src/TeXTableSettings.h"
  "src/TeXTableSettingsWidget.h"
  "src/TextDialog.h"
  "src/TextFormatButtons.h"
  "src/TitlePicker.h"
  "src/TranslateCurveTool.h"
  "src/VectorCurve.h"
  "src/customevents.h"
  "src/fit_gsl.h"
  "src/core/AbstractAspect.h"
  "src/core/AbstractColumn.h"
  "src/core/AbstractFilter.h"
  "src/core/AbstractImportFilter.h"
  "src/core/AbstractPart.h"
  "src/core/AbstractSimpleFilter.h"
  "src/core/AspectPrivate.h"
  "src/core/PartMdiView.h"
  "src/core/Project.h"
  "src/core/ProjectConfigPage.h"
  "src/core/aspectcommands.h"
  "src/core/column/Column.h"
  "src/core/column/ColumnPrivate.h"
  "src/core/column/columncommands.h"
  "src/core/datatypes/DateTime2DoubleFilter.h"
  "src/core/datatypes/DateTime2StringFilter.h"
  "src/core/datatypes/DayOfWeek2DoubleFilter.h"
  "src/core/datatypes/Double2DateTimeFilter.h"
  "src/core/datatypes/Double2DayOfWeekFilter.h"
  "src/core/datatypes/Double2MonthFilter.h"
  "src/core/datatypes/Double2StringFilter.h"
  "src/core/datatypes/Month2DoubleFilter.h"
  "src/core/datatypes/NumericDateTimeBaseFilter.h"
  "src/core/datatypes/SimpleCopyThroughFilter.h"
  "src/core/datatypes/String2DateTimeFilter.h"
  "src/core/datatypes/String2DayOfWeekFilter.h"
  "src/core/datatypes/String2DoubleFilter.h"
  "src/core/datatypes/String2MonthFilter.h"
  "src/core/future_Folder.h"
  "src/core/interfaces.h"
  "src/lib/ActionManager.h"
  "src/lib/ConfigPageWidget.h"
  "src/lib/Interval.h"
  "src/lib/IntervalAttribute.h"
  "src/lib/XmlStreamReader.h"
  "src/lib/macros.h"
  "src/matrix/MatrixModel.h"
  "src/matrix/MatrixView.h"
  "src/matrix/future_Matrix.h"
  "src/matrix/matrixcommands.h"
  "src/table/AsciiTableImportFilter.h"
  "src/table/TableCommentsHeaderModel.h"
  "src/table/TableDoubleHeaderView.h"
  "src/table/TableItemDelegate.h"
  "src/table/TableModel.h"
  "src/table/TableView.h"
  "src/table/future_SortDialog.h"
  "src/table/future_Table.h"
  "src/table/tablecommands.h"
  "src/globals.h"
  )
configure_file( "src/version.h.in" "version.h" )

###################### SOURCES ##############################################
set( SRCS
  "src/ApplicationWindow.cpp"
  "src/ArrowMarker.cpp"
  "src/AssociationsDialog.cpp"
  "src/AxesDialog.cpp"
  "src/Bar.cpp"
  "src/BoxCurve.cpp"
  "src/CanvasPicker.cpp"
  "src/ColorBox.cpp"
  "src/ColorButton.cpp"
  "src/ColorMapEditor.cpp"
  "src/Cone3D.cpp"
  "src/ConfigDialog.cpp"
  "src/Convolution.cpp"
  "src/Correlation.cpp"
  "src/CurveRangeDialog.cpp"
  "src/CurvesDialog.cpp"
  "src/DataPickerTool.cpp"
  "src/DataPointPicker.cpp"
  "src/DataSetDialog.cpp"
  "src/Differentiation.cpp"
  "src/ErrDialog.cpp"
  "src/ExpDecayDialog.cpp"
  "src/ExponentialFit.cpp"
  "src/ExportDialog.cpp"
  "src/ExtensibleFileDialog.cpp"
  "src/FFT.cpp"
  "src/FFTDialog.cpp"
  "src/FFTFilter.cpp"
  "src/Filter.cpp"
  "src/FilterDialog.cpp"
  "src/FindDialog.cpp"
  "src/Fit.cpp"
  "src/FitDialog.cpp"
  "src/Folder.cpp"
  "src/FunctionCurve.cpp"
  "src/FunctionDialog.cpp"
  "src/Graph.cpp"
  "src/Graph3D.cpp"
  "src/Grid.cpp"
  "src/IconLoader.cpp"
  "src/ImageDialog.cpp"
  "src/ImageExportDialog.cpp"
  "src/ImageMarker.cpp"
  "src/ImportASCIIDialog.cpp"
  "src/IntDialog.cpp"
  "src/Integration.cpp"
  "src/Interpolation.cpp"
  "src/InterpolationDialog.cpp"
  "src/LayerDialog.cpp"
  "src/Legend.cpp"
  "src/LineDialog.cpp"
  "src/LineProfileTool.cpp"
  "src/Matrix.cpp"
  "src/MuParserScript.cpp"
  "src/MuParserScripting.cpp"
  "src/MultiLayer.cpp"
  "src/MultiPeakFit.cpp"
  "src/MultiPeakFitTool.cpp"
  "src/MyParser.cpp"
  "src/MyWidget.cpp"
  "src/NonLinearFit.cpp"
  "src/Note.cpp"
  "src/OpenProjectDialog.cpp"
  "src/PatternBox.cpp"
  "src/PenWidget.cpp"
  "src/Plot.cpp"
  "src/Plot3DDialog.cpp"
  "src/PlotCurve.cpp"
  "src/PlotDialog.cpp"
  "src/PlotEnrichement.cpp"
  "src/PlotWizard.cpp"
  "src/PluginFit.cpp"
  "src/PolynomFitDialog.cpp"
  "src/PolynomialFit.cpp"
  "src/QwtBarCurve.cpp"
  "src/QwtErrorPlotCurve.cpp"
  "src/QwtHistogram.cpp"
  "src/QwtPieCurve.cpp"
  "src/RangeSelectorTool.cpp"
  "src/RenameWindowDialog.cpp"
  "src/ScaleDraw.cpp"
  "src/ScalePicker.cpp"
  "src/ScreenPickerTool.cpp"
  "src/Script.cpp"
  "src/ScriptEdit.cpp"
  "src/ScriptingEnv.cpp"
  "src/ScriptingLangDialog.cpp"
  "src/SelectionMoveResizer.cpp"
  "src/SigmoidalFit.cpp"
  "src/SmoothCurveDialog.cpp"
  "src/SmoothFilter.cpp"
  "src/Spectrogram.cpp"
  "src/SurfaceDialog.cpp"
  "src/SymbolBox.cpp"
  "src/SymbolDialog.cpp"
  "src/Table.cpp"
  "src/TableStatistics.cpp"
  "src/TeXTableExportDialog.cpp"
  "src/TeXTableSettings.cpp"
  "src/TeXTableSettingsWidget.cpp"
  "src/TextDialog.cpp"
  "src/TextFormatButtons.cpp"
  "src/TitlePicker.cpp"
  "src/TranslateCurveTool.cpp"
  "src/VectorCurve.cpp"
  "src/fit_gsl.cpp"
  "src/core/AbstractAspect.cpp"
  "src/core/AbstractFilter.cpp"
  "src/core/AbstractPart.cpp"
  "src/core/AbstractSimpleFilter.cpp"
  "src/core/AspectPrivate.cpp"
  "src/core/PartMdiView.cpp"
  "src/core/Project.cpp"
  "src/core/ProjectConfigPage.cpp"
  "src/core/column/Column.cpp"
  "src/core/column/ColumnPrivate.cpp"
  "src/core/column/columncommands.cpp"
  "src/core/datatypes/DateTime2StringFilter.cpp"
  "src/core/datatypes/Double2StringFilter.cpp"
  "src/core/datatypes/NumericDateTimeBaseFilter.cpp"
  "src/core/datatypes/String2DateTimeFilter.cpp"
  "src/core/future_Folder.cpp"
  "src/lib/ActionManager.cpp"
  "src/lib/ConfigPageWidget.cpp"
  "src/lib/XmlStreamReader.cpp"
  "src/matrix/MatrixModel.cpp"
  "src/matrix/MatrixView.cpp"
  "src/matrix/future_Matrix.cpp"
  "src/matrix/matrixcommands.cpp"
  "src/table/AsciiTableImportFilter.cpp"
  "src/table/TableCommentsHeaderModel.cpp"
  "src/table/TableDoubleHeaderView.cpp"
  "src/table/TableItemDelegate.cpp"
  "src/table/TableModel.cpp"
  "src/table/TableView.cpp"
  "src/table/future_SortDialog.cpp"
  "src/table/future_Table.cpp"
  "src/table/tablecommands.cpp"
  "src/globals.cpp"
  )

###################### FORMS ##############################################
set( FORMS
  "src/MakhberAbout.ui"
  "src/core/ProjectConfigPage.ui"
  "src/matrix/matrixcontroltabs.ui"
  "src/table/DimensionsDialog.ui"
  "src/table/controltabs.ui"
  )

add_library( libmakhber
  ${SRCS}
  ${HEADERS}
  ${FORMS}
  )

set_target_properties( libmakhber PROPERTIES OUTPUT_NAME makhber )

target_link_libraries( libmakhber
  Qt5::Core
  Qt5::Gui
  Qt5::PrintSupport
  Qt5::OpenGL
  Qt5::Svg
  OpenGL::GL
  OpenGL::GLU
  ZLIB::ZLIB
  muparser::muparser
  minigzip
  qwt5-qt5
  qwtplot3d
  )

if( BUILD_SHARED_LIBS AND WIN32 )
  target_compile_definitions( libmakhber PUBLIC QWT_DLL )
  set_target_properties( libmakhber
    PROPERTIES
      WINDOWS_EXPORT_ALL_SYMBOLS ON
  )
endif()

if( MINGW AND CMAKE_GENERATOR STREQUAL "Ninja")
  target_link_libraries( libmakhber ${GSL_LIBRARIES} )
  target_include_directories( libmakhber PUBLIC ${GSL_INCLUDE_DIRS} )
else()
  target_link_libraries( libmakhber
    GSL::gsl
    GSL::gslcblas
  )
endif()

target_include_directories( libmakhber PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  ${CMAKE_CURRENT_BINARY_DIR}
  )

if( MULTI_CONFIG )
  target_include_directories( libmakhber PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}/libmakhber_autogen/include_$<CONFIG>"
    )
else()
  target_include_directories( libmakhber PUBLIC
    "${CMAKE_CURRENT_BINARY_DIR}/libmakhber_autogen/include"
    )
endif()

target_compile_definitions( libmakhber PUBLIC DYNAMIC_PLUGIN_PATH )
# target_compile_definitions( libmakhber PUBLIC DYNAMIC_MANUAL_PATH )

if( WIN32 )
  target_compile_definitions( libmakhber PUBLIC
    TS_PATH="/../translations"
    PLUGIN_PATH="/../fitPlugins"
    )
elseif( APPLE )
  target_compile_definitions( libmakhber PUBLIC
    TS_PATH="/../Resources/translations"
    PLUGIN_PATH="/../PlugIns/fitPlugins"
    )
else()
  target_compile_definitions( libmakhber PUBLIC
    TS_PATH="/../${CMAKE_INSTALL_DATADIR}/makhber/translations"
    PLUGIN_PATH="/../${CMAKE_INSTALL_LIBDIR}/makhber/plugins"
    )
endif()

if( MAKHBER_SEARCH_FOR_UPDATES OR MAKHBER_DOWNLOAD_LINKS )
  target_link_libraries( libmakhber Qt5::Network )
endif()

if( MAKHBER_SEARCH_FOR_UPDATES )
  target_compile_definitions( libmakhber PUBLIC SEARCH_FOR_UPDATES )
endif()

if( MAKHBER_DOWNLOAD_LINKS )
  target_compile_definitions( libmakhber PUBLIC DOWNLOAD_LINKS )
endif()

if( MAKHBER_ORIGIN_IMPORT )
  target_sources( libmakhber PRIVATE "src/importOPJ.h" "src/importOPJ.cpp" )
  target_link_libraries( libmakhber $<IF:$<BOOL:${BUILD_SHARED_LIBS}>,origin_shared,origin_static> )
  target_include_directories( libmakhber PUBLIC "${CMAKE_SOURCE_DIR}/3rdparty/liborigin" )
  target_compile_definitions( libmakhber PUBLIC ORIGIN_IMPORT )
endif()

if( MAKHBER_SCRIPTING_MUPARSER AND MAKHBER_SCRIPTING_PYTHON )
  target_compile_definitions( libmakhber PUBLIC SCRIPTING_DIALOG )
endif()

if( MAKHBER_SCRIPTING_MUPARSER OR MAKHBER_SCRIPTING_PYTHON )
  target_compile_definitions( libmakhber PUBLIC SCRIPTING_CONSOLE )
endif()

if( MAKHBER_SCRIPTING_MUPARSER )
  target_compile_definitions( libmakhber PUBLIC SCRIPTING_MUPARSER )
endif()

if( MAKHBER_SCRIPTING_PYTHON )

  target_link_libraries( libmakhber Python3::Python )
  target_compile_definitions( libmakhber PUBLIC QT_NO_KEYWORDS )

  if( SIP_VERSION VERSION_GREATER_EQUAL 5 )
    add_dependencies( libmakhber generate_SIP_HDR )
    target_include_directories( libmakhber PRIVATE ${CMAKE_BINARY_DIR}/sip )
  endif()

  if( WIN32 )
    set( PYTHONPATH "." )
  elseif( APPLE )
    set( PYTHONPATH "/../Resources" )
  else()
    set( PYTHONPATH "/../${PYTHON_INSTALL_PATH}" )
  endif()

  set_property( SOURCE "src/PythonScripting.cpp"
    APPEND
    PROPERTY COMPILE_DEFINITIONS
      PYTHONPATH="${PYTHONPATH}"
  )

  set_property( SOURCE
    "src/PythonScripting.cpp"
    "src/Script.cpp"
    "src/ScriptingEnv.cpp"
    "src/globals.cpp"
    APPEND
    PROPERTY COMPILE_DEFINITIONS SCRIPTING_PYTHON
  )

  if( MINGW )
    set_property( SOURCE "src/PythonScripting.cpp"
      APPEND
      PROPERTY COMPILE_DEFINITIONS
      PYTHONHOME=${PYTHON_PREFIX}
    )
  endif()

  target_sources( libmakhber PRIVATE
    "src/PythonScript.h"
    "src/PythonScripting.h"
    "src/PythonScript.cpp"
    "src/PythonScripting.cpp"
  )

endif()

if( BUILD_SHARED_LIBS )
  install( TARGETS libmakhber
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()
