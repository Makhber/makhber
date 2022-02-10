###################### HEADERS ##############################################
set( HEADERS
  "analysis/Convolution.h"
  "analysis/Correlation.h"
  "analysis/Differentiation.h"
  "analysis/ExponentialFit.h"
  "analysis/FFT.h"
  "analysis/FFTFilter.h"
  "analysis/Filter.h"
  "analysis/Fit.h"
  "analysis/Integration.h"
  "analysis/Interpolation.h"
  "analysis/MultiPeakFit.h"
  "analysis/NonLinearFit.h"
  "analysis/PluginFit.h"
  "analysis/PolynomialFit.h"
  "analysis/SigmoidalFit.h"
  "analysis/SmoothFilter.h"
  "analysis/dialogs/ExpDecayDialog.h"
  "analysis/dialogs/FFTDialog.h"
  "analysis/dialogs/FilterDialog.h"
  "analysis/dialogs/FitDialog.h"
  "analysis/dialogs/IntDialog.h"
  "analysis/dialogs/InterpolationDialog.h"
  "analysis/dialogs/PolynomFitDialog.h"
  "analysis/dialogs/SmoothCurveDialog.h"
  "analysis/fit_gsl.h"
  "aspects/AbstractAspect.h"
  "aspects/AbstractColumn.h"
  "aspects/AbstractFilter.h"
  "aspects/AbstractImportFilter.h"
  "aspects/AbstractPart.h"
  "aspects/AbstractSimpleFilter.h"
  "aspects/AspectPrivate.h"
  "aspects/PartMdiView.h"
  "aspects/Project.h"
  "aspects/ProjectConfigPage.h"
  "aspects/aspectcommands.h"
  "aspects/column/Column.h"
  "aspects/column/ColumnPrivate.h"
  "aspects/column/columncommands.h"
  "aspects/datatypes/DateTime2DoubleFilter.h"
  "aspects/datatypes/DateTime2StringFilter.h"
  "aspects/datatypes/DayOfWeek2DoubleFilter.h"
  "aspects/datatypes/Double2DateTimeFilter.h"
  "aspects/datatypes/Double2DayOfWeekFilter.h"
  "aspects/datatypes/Double2MonthFilter.h"
  "aspects/datatypes/Double2StringFilter.h"
  "aspects/datatypes/Month2DoubleFilter.h"
  "aspects/datatypes/NumericDateTimeBaseFilter.h"
  "aspects/datatypes/SimpleCopyThroughFilter.h"
  "aspects/datatypes/String2DateTimeFilter.h"
  "aspects/datatypes/String2DayOfWeekFilter.h"
  "aspects/datatypes/String2DoubleFilter.h"
  "aspects/datatypes/String2MonthFilter.h"
  "aspects/future_Folder.h"
  "aspects/interfaces.h"
  "core/ApplicationWindow.h"
  "core/ColorBox.h"
  "core/ColorButton.h"
  "core/Folder.h"
  "core/IconLoader.h"
  "core/MakhberDefs.h"
  "core/MakhberObject.h"
  "core/MyWidget.h"
  "core/PatternBox.h"
  "core/PenWidget.h"
  "core/PlotWizard.h"
  "core/TeXTableSettings.h"
  "core/TeXTableSettingsWidget.h"
  "core/TextFormatButtons.h"
  "core/dialogs/ConfigDialog.h"
  "core/dialogs/ExtensibleFileDialog.h"
  "core/dialogs/FindDialog.h"
  "core/dialogs/ImageDialog.h"
  "core/dialogs/ImportASCIIDialog.h"
  "core/dialogs/OpenProjectDialog.h"
  "core/dialogs/RenameWindowDialog.h"
  "core/dialogs/SymbolDialog.h"
  "core/dialogs/TeXTableExportDialog.h"
  "core/globals.h"
  "lib/ActionManager.h"
  "lib/ConfigPageWidget.h"
  "lib/Interval.h"
  "lib/IntervalAttribute.h"
  "lib/QStringStdString.h"
  "lib/macros.h"
  "matrix/Matrix.h"
  "matrix/MatrixModel.h"
  "matrix/MatrixView.h"
  "matrix/future_Matrix.h"
  "matrix/matrixcommands.h"
  "plot2D/ArrowMarker.h"
  "plot2D/BoxCurve.h"
  "plot2D/CanvasPicker.h"
  "plot2D/ColorMapEditor.h"
  "plot2D/DataPickerTool.h"
  "plot2D/DataPointPicker.h"
  "plot2D/FunctionCurve.h"
  "plot2D/Graph.h"
  "plot2D/Grid.h"
  "plot2D/ImageMarker.h"
  "plot2D/Legend.h"
  "plot2D/LineProfileTool.h"
  "plot2D/MultiLayer.h"
  "plot2D/MultiPeakFitTool.h"
  "plot2D/Plot.h"
  "plot2D/PlotCurve.h"
  "plot2D/PlotEnrichement.h"
  "plot2D/PlotToolInterface.h"
  "plot2D/QwtBarCurve.h"
  "plot2D/QwtErrorPlotCurve.h"
  "plot2D/QwtHistogram.h"
  "plot2D/QwtPieCurve.h"
  "plot2D/RangeSelectorTool.h"
  "plot2D/ScaleDraw.h"
  "plot2D/ScalePicker.h"
  "plot2D/ScreenPickerTool.h"
  "plot2D/SelectionMoveResizer.h"
  "plot2D/Spectrogram.h"
  "plot2D/SymbolBox.h"
  "plot2D/TitlePicker.h"
  "plot2D/TranslateCurveTool.h"
  "plot2D/VectorCurve.h"
  "plot2D/dialogs/AssociationsDialog.h"
  "plot2D/dialogs/AxesDialog.h"
  "plot2D/dialogs/CurveRangeDialog.h"
  "plot2D/dialogs/CurvesDialog.h"
  "plot2D/dialogs/DataSetDialog.h"
  "plot2D/dialogs/ErrDialog.h"
  "plot2D/dialogs/FunctionDialog.h"
  "plot2D/dialogs/ImageExportDialog.h"
  "plot2D/dialogs/LayerDialog.h"
  "plot2D/dialogs/LineDialog.h"
  "plot2D/dialogs/PlotDialog.h"
  "plot2D/dialogs/TextDialog.h"
  "plot3D/Bar.h"
  "plot3D/Cone3D.h"
  "plot3D/Graph3D.h"
  "plot3D/Plot3DDialog.h"
  "plot3D/SurfaceDialog.h"
  "scripting/MuParserScript.h"
  "scripting/MuParserScripting.h"
  "scripting/MyParser.h"
  "scripting/Note.h"
  "scripting/Script.h"
  "scripting/ScriptEdit.h"
  "scripting/ScriptingEnv.h"
  "scripting/ScriptingLangDialog.h"
  "scripting/customevents.h"
  "table/AsciiTableImportFilter.h"
  "table/ExportDialog.h"
  "table/Table.h"
  "table/TableCommentsHeaderModel.h"
  "table/TableDoubleHeaderView.h"
  "table/TableItemDelegate.h"
  "table/TableModel.h"
  "table/TableStatistics.h"
  "table/TableView.h"
  "table/future_SortDialog.h"
  "table/future_Table.h"
  "table/tablecommands.h"
  )
configure_file( "core/version.h.in" "version.h" )

###################### SOURCES ##############################################
set( SRCS
  "analysis/Convolution.cpp"
  "analysis/Correlation.cpp"
  "analysis/Differentiation.cpp"
  "analysis/ExponentialFit.cpp"
  "analysis/FFT.cpp"
  "analysis/FFTFilter.cpp"
  "analysis/Filter.cpp"
  "analysis/Fit.cpp"
  "analysis/Integration.cpp"
  "analysis/Interpolation.cpp"
  "analysis/MultiPeakFit.cpp"
  "analysis/NonLinearFit.cpp"
  "analysis/PluginFit.cpp"
  "analysis/PolynomialFit.cpp"
  "analysis/SigmoidalFit.cpp"
  "analysis/SmoothFilter.cpp"
  "analysis/dialogs/ExpDecayDialog.cpp"
  "analysis/dialogs/FFTDialog.cpp"
  "analysis/dialogs/FilterDialog.cpp"
  "analysis/dialogs/FitDialog.cpp"
  "analysis/dialogs/IntDialog.cpp"
  "analysis/dialogs/InterpolationDialog.cpp"
  "analysis/dialogs/PolynomFitDialog.cpp"
  "analysis/dialogs/SmoothCurveDialog.cpp"
  "analysis/fit_gsl.cpp"
  "aspects/AbstractAspect.cpp"
  "aspects/AbstractFilter.cpp"
  "aspects/AbstractPart.cpp"
  "aspects/AbstractSimpleFilter.cpp"
  "aspects/AspectPrivate.cpp"
  "aspects/PartMdiView.cpp"
  "aspects/Project.cpp"
  "aspects/ProjectConfigPage.cpp"
  "aspects/column/Column.cpp"
  "aspects/column/ColumnPrivate.cpp"
  "aspects/column/columncommands.cpp"
  "aspects/datatypes/DateTime2StringFilter.cpp"
  "aspects/datatypes/Double2StringFilter.cpp"
  "aspects/datatypes/NumericDateTimeBaseFilter.cpp"
  "aspects/datatypes/String2DateTimeFilter.cpp"
  "aspects/future_Folder.cpp"
  "core/ApplicationWindow.cpp"
  "core/ColorBox.cpp"
  "core/ColorButton.cpp"
  "core/Folder.cpp"
  "core/IconLoader.cpp"
  "core/MyWidget.cpp"
  "core/PatternBox.cpp"
  "core/PenWidget.cpp"
  "core/PlotWizard.cpp"
  "core/TeXTableSettings.cpp"
  "core/TeXTableSettingsWidget.cpp"
  "core/TextFormatButtons.cpp"
  "core/dialogs/ConfigDialog.cpp"
  "core/dialogs/ExtensibleFileDialog.cpp"
  "core/dialogs/FindDialog.cpp"
  "core/dialogs/ImageDialog.cpp"
  "core/dialogs/ImportASCIIDialog.cpp"
  "core/dialogs/OpenProjectDialog.cpp"
  "core/dialogs/RenameWindowDialog.cpp"
  "core/dialogs/SymbolDialog.cpp"
  "core/dialogs/TeXTableExportDialog.cpp"
  "core/globals.cpp"
  "lib/ActionManager.cpp"
  "lib/ConfigPageWidget.cpp"
  "matrix/Matrix.cpp"
  "matrix/MatrixModel.cpp"
  "matrix/MatrixView.cpp"
  "matrix/future_Matrix.cpp"
  "matrix/matrixcommands.cpp"
  "plot2D/ArrowMarker.cpp"
  "plot2D/BoxCurve.cpp"
  "plot2D/CanvasPicker.cpp"
  "plot2D/ColorMapEditor.cpp"
  "plot2D/DataPickerTool.cpp"
  "plot2D/DataPointPicker.cpp"
  "plot2D/FunctionCurve.cpp"
  "plot2D/Graph.cpp"
  "plot2D/Grid.cpp"
  "plot2D/ImageMarker.cpp"
  "plot2D/Legend.cpp"
  "plot2D/LineProfileTool.cpp"
  "plot2D/MultiLayer.cpp"
  "plot2D/MultiPeakFitTool.cpp"
  "plot2D/Plot.cpp"
  "plot2D/PlotCurve.cpp"
  "plot2D/PlotEnrichement.cpp"
  "plot2D/QwtBarCurve.cpp"
  "plot2D/QwtErrorPlotCurve.cpp"
  "plot2D/QwtHistogram.cpp"
  "plot2D/QwtPieCurve.cpp"
  "plot2D/RangeSelectorTool.cpp"
  "plot2D/ScaleDraw.cpp"
  "plot2D/ScalePicker.cpp"
  "plot2D/ScreenPickerTool.cpp"
  "plot2D/SelectionMoveResizer.cpp"
  "plot2D/Spectrogram.cpp"
  "plot2D/SymbolBox.cpp"
  "plot2D/TitlePicker.cpp"
  "plot2D/TranslateCurveTool.cpp"
  "plot2D/VectorCurve.cpp"
  "plot2D/dialogs/AssociationsDialog.cpp"
  "plot2D/dialogs/AxesDialog.cpp"
  "plot2D/dialogs/CurveRangeDialog.cpp"
  "plot2D/dialogs/CurvesDialog.cpp"
  "plot2D/dialogs/DataSetDialog.cpp"
  "plot2D/dialogs/ErrDialog.cpp"
  "plot2D/dialogs/FunctionDialog.cpp"
  "plot2D/dialogs/ImageExportDialog.cpp"
  "plot2D/dialogs/LayerDialog.cpp"
  "plot2D/dialogs/LineDialog.cpp"
  "plot2D/dialogs/PlotDialog.cpp"
  "plot2D/dialogs/TextDialog.cpp"
  "plot3D/Bar.cpp"
  "plot3D/Cone3D.cpp"
  "plot3D/Graph3D.cpp"
  "plot3D/Plot3DDialog.cpp"
  "plot3D/SurfaceDialog.cpp"
  "scripting/MuParserScript.cpp"
  "scripting/MuParserScripting.cpp"
  "scripting/MyParser.cpp"
  "scripting/Note.cpp"
  "scripting/Script.cpp"
  "scripting/ScriptEdit.cpp"
  "scripting/ScriptingEnv.cpp"
  "scripting/ScriptingLangDialog.cpp"
  "table/AsciiTableImportFilter.cpp"
  "table/ExportDialog.cpp"
  "table/Table.cpp"
  "table/TableCommentsHeaderModel.cpp"
  "table/TableDoubleHeaderView.cpp"
  "table/TableItemDelegate.cpp"
  "table/TableModel.cpp"
  "table/TableStatistics.cpp"
  "table/TableView.cpp"
  "table/future_SortDialog.cpp"
  "table/future_Table.cpp"
  "table/tablecommands.cpp"
  )

###################### FORMS ##############################################
set( FORMS
  "aspects/ProjectConfigPage.ui"
  "core/MakhberAbout.ui"
  "matrix/matrixcontroltabs.ui"
  "table/DimensionsDialog.ui"
  "table/TableConfigPage.ui"
  "table/controltabs.ui"
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
  muparser::muparser
  Qwt::Qwt
  qwtplot3d
)

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
  "${CMAKE_CURRENT_SOURCE_DIR}"
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
  QT_NO_KEYWORDS
  DYNAMIC_PLUGIN_PATH
)
# target_compile_definitions( libmakhber PUBLIC DYNAMIC_MANUAL_PATH )

if( BUILD_SHARED_LIBS AND WIN32 )
  target_compile_definitions( libmakhber
    PUBLIC MAKHBER_DLL
    PRIVATE MAKHBER_DLL_EXPORT
  )
endif()

if( MSVC )
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

if( WIN32 )
  target_compile_definitions( libmakhber PUBLIC "_USE_MATH_DEFINES" )
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
  target_sources( libmakhber PRIVATE "core/importOPJ.h" "core/importOPJ.cpp" )
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

  set_property( SOURCE "scripting/PythonScripting.cpp"
    APPEND
    PROPERTY COMPILE_DEFINITIONS
      PYTHONPATH="${PYTHONPATH}"
  )

  if( PyQt_SIP )
    set_property( SOURCE "scripting/PythonScripting.cpp"
      APPEND
      PROPERTY COMPILE_DEFINITIONS
        PYQT_SIP
    )
  endif()

  set_property( SOURCE
    "scripting/PythonScripting.cpp"
    "scripting/Script.cpp"
    "scripting/ScriptingEnv.cpp"
    "core/globals.cpp"
    APPEND
    PROPERTY COMPILE_DEFINITIONS SCRIPTING_PYTHON
  )

  if( MINGW )
    set_property( SOURCE "scripting/PythonScripting.cpp"
      APPEND
      PROPERTY COMPILE_DEFINITIONS
      PYTHONHOME=${PYTHON_PREFIX}
    )
  endif()

  target_sources( libmakhber PRIVATE
    "scripting/PythonScript.h"
    "scripting/PythonScripting.h"
    "scripting/PythonScript.cpp"
    "scripting/PythonScripting.cpp"
  )

endif()

if( BUILD_SHARED_LIBS )
  install( TARGETS libmakhber
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()

if( MAKHBER_SCRIPTING_PYTHON )

  install( FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/scripting/makhberrc.py
    ${CMAKE_CURRENT_SOURCE_DIR}/scripting/makhberUtil.py
    COMPONENT Python
    DESTINATION ${PYTHON_INSTALL_PATH}
  )
  install( DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/main/__pycache__"
    COMPONENT Python
    DESTINATION "${PYTHON_INSTALL_PATH}"
  )

  if( MAKHBER_BUNDLE_PYQT5 )
    install( CODE
      "execute_process(
        COMMAND ${Python3_EXECUTABLE} -m pip install pyqt5 pyqt5-sip --no-deps
        --target=\${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTALL_PATH}
      )"
      COMPONENT Python
    )
  endif()

  if( MSVC )
    include( PythonEmbedHashes.cmake )
    install( CODE
      "file( DOWNLOAD
        https://www.python.org/ftp/python/${Python3_VERSION}/python-${Python3_VERSION}-embed-amd64.zip
        python_embed.zip
        SHOW_PROGRESS
        EXPECTED_HASH MD5=${PYTHON_EMBED_HASH}
      )
      file( ARCHIVE_EXTRACT
        INPUT python_embed.zip
        DESTINATION \${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}
        VERBOSE
      )"
      COMPONENT Python
    )
  endif()

endif()

add_subdirectory( main )
