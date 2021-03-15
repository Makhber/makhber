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
  "src/future/core/AbstractAspect.h"
  "src/future/core/AbstractColumn.h"
  "src/future/core/AbstractFilter.h"
  "src/future/core/AbstractImportFilter.h"
  "src/future/core/AbstractPart.h"
  "src/future/core/AbstractSimpleFilter.h"
  "src/future/core/AspectPrivate.h"
  "src/future/core/PartMdiView.h"
  "src/future/core/Project.h"
  "src/future/core/ProjectConfigPage.h"
  "src/future/core/aspectcommands.h"
  "src/future/core/column/Column.h"
  "src/future/core/column/ColumnPrivate.h"
  "src/future/core/column/columncommands.h"
  "src/future/core/datatypes/DateTime2DoubleFilter.h"
  "src/future/core/datatypes/DateTime2StringFilter.h"
  "src/future/core/datatypes/DayOfWeek2DoubleFilter.h"
  "src/future/core/datatypes/Double2DateTimeFilter.h"
  "src/future/core/datatypes/Double2DayOfWeekFilter.h"
  "src/future/core/datatypes/Double2MonthFilter.h"
  "src/future/core/datatypes/Double2StringFilter.h"
  "src/future/core/datatypes/Month2DoubleFilter.h"
  "src/future/core/datatypes/NumericDateTimeBaseFilter.h"
  "src/future/core/datatypes/SimpleCopyThroughFilter.h"
  "src/future/core/datatypes/String2DateTimeFilter.h"
  "src/future/core/datatypes/String2DayOfWeekFilter.h"
  "src/future/core/datatypes/String2DoubleFilter.h"
  "src/future/core/datatypes/String2MonthFilter.h"
  "src/future/core/future_Folder.h"
  "src/future/core/interfaces.h"
  "src/future/lib/ActionManager.h"
  "src/future/lib/ConfigPageWidget.h"
  "src/future/lib/Interval.h"
  "src/future/lib/IntervalAttribute.h"
  "src/future/lib/XmlStreamReader.h"
  "src/future/lib/macros.h"
  "src/future/matrix/MatrixModel.h"
  "src/future/matrix/MatrixView.h"
  "src/future/matrix/future_Matrix.h"
  "src/future/matrix/matrixcommands.h"
  "src/future/table/AsciiTableImportFilter.h"
  "src/future/table/TableCommentsHeaderModel.h"
  "src/future/table/TableDoubleHeaderView.h"
  "src/future/table/TableItemDelegate.h"
  "src/future/table/TableModel.h"
  "src/future/table/TableView.h"
  "src/future/table/future_SortDialog.h"
  "src/future/table/future_Table.h"
  "src/future/table/tablecommands.h"
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
  "src/future/core/AbstractAspect.cpp"
  "src/future/core/AbstractFilter.cpp"
  "src/future/core/AbstractPart.cpp"
  "src/future/core/AbstractSimpleFilter.cpp"
  "src/future/core/AspectPrivate.cpp"
  "src/future/core/PartMdiView.cpp"
  "src/future/core/Project.cpp"
  "src/future/core/ProjectConfigPage.cpp"
  "src/future/core/column/Column.cpp"
  "src/future/core/column/ColumnPrivate.cpp"
  "src/future/core/column/columncommands.cpp"
  "src/future/core/datatypes/DateTime2StringFilter.cpp"
  "src/future/core/datatypes/Double2StringFilter.cpp"
  "src/future/core/datatypes/NumericDateTimeBaseFilter.cpp"
  "src/future/core/datatypes/String2DateTimeFilter.cpp"
  "src/future/core/future_Folder.cpp"
  "src/future/lib/ActionManager.cpp"
  "src/future/lib/ConfigPageWidget.cpp"
  "src/future/lib/XmlStreamReader.cpp"
  "src/future/matrix/MatrixModel.cpp"
  "src/future/matrix/MatrixView.cpp"
  "src/future/matrix/future_Matrix.cpp"
  "src/future/matrix/matrixcommands.cpp"
  "src/future/table/AsciiTableImportFilter.cpp"
  "src/future/table/TableCommentsHeaderModel.cpp"
  "src/future/table/TableDoubleHeaderView.cpp"
  "src/future/table/TableItemDelegate.cpp"
  "src/future/table/TableModel.cpp"
  "src/future/table/TableView.cpp"
  "src/future/table/future_SortDialog.cpp"
  "src/future/table/future_Table.cpp"
  "src/future/table/tablecommands.cpp"
  "src/globals.cpp"
  )

###################### FORMS ##############################################
set( FORMS
  "src/MakhberAbout.ui"
  "src/future/core/ProjectConfigPage.ui"
  "src/future/matrix/matrixcontroltabs.ui"
  "src/future/table/DimensionsDialog.ui"
  "src/future/table/controltabs.ui"
  )

add_library( libmakhber STATIC
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
  GSL::gsl
  GSL::gslcblas
  muparser::muparser
  minigzip
  qwt5-qt5
  qwtplot3d
  )

target_include_directories( libmakhber PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/future"
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

target_compile_definitions( libmakhber PUBLIC
  LEGACY_CODE_0_2_x
  SCRIPTING_CONSOLE
  SCRIPTING_DIALOG
  )

if( WIN32 )
  target_compile_definitions( libmakhber PUBLIC
    TS_PATH="/translations"
    PLUGIN_PATH="/fitPlugins"
    DYNAMIC_MANUAL_PATH
    DYNAMIC_PLUGIN_PATH
    )
elseif( APPLE )
  target_compile_definitions( libmakhber PUBLIC
    TS_PATH="/../Resources/translations"
    PLUGIN_PATH="/../PlugIns/fitPlugins"
    )
else()
  target_compile_definitions( libmakhber PUBLIC
    TS_PATH="/../share/makhber/translations"
    PLUGIN_PATH="/../lib/makhber/plugins"
    )
endif()

if( MAKHBER_SEARCH_FOR_UPDATE OR MAKHBER_DOWNLOAD_LINKS )
  target_link_libraries( libmakhber Qt5::Network )
endif()

if( MAKHBER_SEARCH_FOR_UPDATES )
  target_compile_definitions( libmakhber PUBLIC SEARCH_FOR_UPDATES )
endif()

if( MAKHBER_DOWNLOAD_LINKS )
  target_compile_definitions( libmakhber PUBLIC DOWNLOAD_LINKS )
endif()

if( MAKHBER_SCRIPTING_MUPARSER )
  target_compile_definitions( libmakhber PUBLIC SCRIPTING_MUPARSER )
endif()

if( MAKHBER_ORIGIN_IMPORT )
  target_sources( libmakhber PRIVATE "src/importOPJ.h" "src/importOPJ.cpp" )
  target_link_libraries( libmakhber origin_static )
  target_include_directories( libmakhber PUBLIC "${CMAKE_SOURCE_DIR}/3rdparty/liborigin" )
  target_compile_definitions( libmakhber PUBLIC ORIGIN_IMPORT )
endif()

if( MAKHBER_SCRIPTING_PYTHON )

  target_link_libraries( libmakhber Python3::Python )
  add_dependencies( libmakhber generate_PyQt_source )
  target_include_directories( libmakhber PRIVATE ${CMAKE_BINARY_DIR}/sip )

  if( WIN32 )
    set( PYTHONPATH "." )
  elseif( APPLE )
    set( PYTHONPATH "/../Resources" )
  else()
    set( PYTHONPATH "/../share/makhber" )
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
    get_filename_component( PYTHON_DIR ${Python3_EXECUTABLE} DIRECTORY )
    file( REAL_PATH "${PYTHON_DIR}/.." PYTHON_ROOT_DIR )
    set( PYTHONHOME "${PYTHON_ROOT_DIR}" CACHE PATH "Python Home" )
    message( STATUS "PYTHONHOME = ${PYTHONHOME}" )
    set_property( SOURCE "src/PythonScripting.cpp"
      APPEND
      PROPERTY COMPILE_DEFINITIONS
      PYTHONHOME=${PYTHONHOME}
      )
  endif()

  target_sources( libmakhber PRIVATE
    "src/PythonScript.h"
    "src/PythonScripting.h"
    "src/PythonScript.cpp"
    "src/PythonScripting.cpp"
    )

endif()
