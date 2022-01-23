
set( CMAKE_AUTOMOC OFF )
set( CMAKE_AUTOUIC OFF )
set( CMAKE_AUTORCC OFF )

set( SRCS
  "applicationWindow.cpp"
  "arrowMarker.cpp"
  "fft.cpp"
  "menus.cpp"
  "readWriteProject.cpp"
  "main.cpp"
  )
if( NOT WIN32 )
  list( APPEND SRCS "3dplot.cpp" "testPaintDevice.cpp" )
endif()

add_executable( unittests ${SRCS} )

find_package( GTest REQUIRED )

if( TARGET GTest::gtest )
  target_link_libraries( unittests libmakhber GTest::gtest )
else()
  target_link_libraries( unittests libmakhber GTest::GTest )
endif()

if( NOT WIN32 )
  target_link_libraries( unittests pthread )
endif()

include(GoogleTest)
gtest_discover_tests( unittests DISCOVERY_MODE PRE_TEST )

set( TestDataFiles
  "3dplot.sciprj"
  "3dplotTable.sciprj"
  "testProject.sciprj"
  "Histo.opj"
  "USstates.opj"
  )
file( COPY ${TestDataFiles} DESTINATION . )

if( NOT (MAKHBER_SEARCH_FOR_UPDATES OR MAKHBER_DOWNLOAD_LINKS) )

  set( menu_files defaultMenus_en.menudat appWithGraphen.menudat )

  foreach( menu ${menu_files} )
    get_filename_component( test_name ${menu} NAME_WLE )
    add_test( NAME menuData_${test_name}
      COMMAND diff -q ${menu} ${CMAKE_CURRENT_SOURCE_DIR}/menuData/${menu}
      )
    set_tests_properties( menuData_${test_name} PROPERTIES DEPENDS unittests )
  endforeach()

endif()

if( MAKHBER_SCRIPTING_PYTHON )

  file( COPY pythonTests/ DESTINATION ./tmp )
  file( COPY ${CMAKE_SOURCE_DIR}/src/scripting/makhberrc.py DESTINATION ./tmp )
  file( COPY ${CMAKE_SOURCE_DIR}/src/main/icons/makhber-logo.png DESTINATION ./tmp )

  file( GLOB pythonTests pythonTests/*.py )
  # qwtPlotCurve segfault with sip5 or macos
  if( SIP_VERSION VERSION_GREATER_EQUAL 5 OR APPLE )
    list( REMOVE_ITEM pythonTests ${CMAKE_CURRENT_SOURCE_DIR}/pythonTests/qwtPlotCurve.py )
  endif()

  find_package( Python3 COMPONENTS NumPy QUIET )
  if( NOT Python3_NumPy_FOUND )
    list( REMOVE_ITEM pythonTests ${CMAKE_CURRENT_SOURCE_DIR}/pythonTests/simulate_a_Hydrogen_spectrum.py )
  endif()

  foreach( pythonTest ${pythonTests} )
    get_filename_component( test_name ${pythonTest} NAME_WLE )
    add_test( NAME python_${test_name}
      COMMAND makhber -l=en -x ${pythonTest}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tmp
    )
    set_tests_properties( python_${test_name} PROPERTIES
      ENVIRONMENT PYTHONPATH=${PYTHONPATH_TEST}
    )
  endforeach()

endif()
