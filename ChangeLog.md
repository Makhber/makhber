# Changelog

## Makhber 0.10.0 Alpha

Released: 14-05-2022

### General

- New Project file format `*.mkbr` based on json
- Drop support for opening SciDAVis/QtiPlot project files.
- Fix some bugs and issues

### Core

- Port to Qwt>6.1
- Requires Qt>=5.12
- Remove/Replace functions deprecated or dropped by Qt6
- Silent warnings from different compilers
- Update packaging for different platforms

## Makhber 0.9.5.3

Released: 18-04-2022

### General

- Update dependencies

## Makhber 0.9.5.2

Released: 18-02-2022

### General

- Fix reporting version when it contains a tweak version.

### Packaging

- Fix building snap package on aarch64

## Makhber 0.9.5.1

Released: 12-02-2022

### General

- Fix importing ascii files.

### Packaging

- Update dependencies

## Makhber 0.9.5

Released: 17-12-2021

### General

- Fix a crash when saving a project with a plot function.
- Fix searching for updates for tweak version.

### Packaging

- Package RPM with scipting python support.

### Python

- Fix a segfault when loading a project from a Python script with Python 3.10

## Makhber 0.9.4.3

Released: 11-12-2021

### Packaging

- Update dependencies (GSL 2.7.1, PyQt5 5.15.6)
- RPM for Fedora 35

### Python

- Fix a segfault when loading a project from a Python script with PyQt5>5.15.4

## Makhber 0.9.4.2

Released: 10-08-2021

### Packaging

- Adapt snap package to snapcraft 5.0
- update python packages for flatpak
- Enable APFS filesystem for packaging macOS DMG (requires cmake>=3.21)
- Build againt the latest gsl 2.7

## Makhber 0.9.4

Released: 24-05-2021

### General

- Some minor fixes.

### Packaging

- Debian and RPM packages for Ubuntu 20.04 and Fedora 34
- Fix Windows Installer by including OpenSSL library and adding the possiblity to select components.
- Fix file association on Linux.

## Makhber 0.9.3

Released: 16-04-2021

### Packaging

- makhber is now available from [Flathub](https://flathub.org/apps/details/com.github.makhber.Makhber).
- makhber is now available as an AppImage.
- Enable Python scripting with all Linux packages (Snap, Flatpak and AppImage).
- Python is now embedded in the Windows Installer, No need to install Python on the user machine.

## Makhber 0.9.2

Released: 10-04-2021

### Packaging

- makhber is now available as a flatpak file (To be published to flatpak store).
- Some fixes for the snap package.

## Makhber 0.9.1

Released: 08-04-2021

### Packaging

- makhber is now avalaible as a snap package (beta channel) from snap store.

### Python

- Drop Python2
- Support SIP >= 5

## Makhber 0.9.0

Released: 10-03-2021

### General

- First release after forking the original project (scidavis).
- Enable HiDPI Scaling.
- Fix 3DGraphs not visible on Windows.
- Fix exporting 3DGraphs to Vector Formats (ps, pdf and svg).
- Fix importing Fit Plugins on Windows.
- Fix some bugs and issues.

### Packaging

- Release Installers for Windows with Python support, the user must have installed Python 3.9 x64 in the standard directory "C:\Program Files\Python39".

### Core

- Move Build system from qmake to cmake.
- Restore building on macos.
- Support more platforms and compilers (MSVC, MinGW-w64).
- Replace/Remove Deprecated decalarations by Qt 5.15
- Move from UnitTest++ to GoogleTest.
- Replace Travis-ci by GitHub-Actions.
