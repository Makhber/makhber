#[[
FindPyQtBuild
-------

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PyQtBuild_DIR``
  The directory containing pyqtbuild python module

``PyQtBuild_FOUND``
  True if the system has pyqtbuild python module
]]

execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "import pyqtbuild; print(pyqtbuild.__path__[0])"
  OUTPUT_VARIABLE PyQtBuild_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
  COMMAND ${Python3_EXECUTABLE} -c "from pyqtbuild import PYQTBUILD_VERSION_STR; print(PYQTBUILD_VERSION_STR)"
  OUTPUT_VARIABLE PyQtBuild_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args( PyQtBuild
  REQUIRED_VARS PyQtBuild_DIR
  VERSION_VAR PyQtBuild_VERSION
)
