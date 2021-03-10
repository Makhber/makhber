# Changes

## Makhber 0.9.0

Released: 10-03-2020

### General

- First release after forking the original project (scidavis).
- Release Installers for Windows with Python support, the user must have installed Python 3.9 x64 in the standard directory "C:\Program Files\Python39".
- Enable HiDPI Scaling.
- Fix 3DGraphs not visible on Windows.
- Fix exporting 3DGraphs to Vector Formats (ps, pdf and svg).
- Fix importing Fit Plugins on Windows.
- Fix some bugs and issues.

### Core

- Move Build system from qmake to cmake.
- Restore building on macos.
- Support more platforms and compilers (MSVC, MinGW-w64).
- Replace/Remove Deprecated decalarations by Qt 5.15
- Move from UnitTest++ to GoogleTest.
- Replace Travis-ci by GitHub-Actions.
