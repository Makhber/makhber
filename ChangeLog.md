# Changelog

## Makhber 0.10.0

Released: DD-MM-YYYY

### General

### Core

- Move to Qwt 6.1

## Makhber 0.9.4

Released: 24-05-2021

### General

- Debian and RPM packages for Ubuntu 20.04 and Fedora 34
- Fix Windows Installer by including OpenSSL library and adding the possiblity to select components.
- Fix file association on Linux.
- Other minor fixes.

## Makhber 0.9.3

Released: 16-04-2021

### General

- makhber is now available from [Flathub](https://flathub.org/apps/details/com.github.makhber.Makhber).
- makhber is now available as an AppImage.
- Enable Python scripting with all Linux packages (Snap, Flatpak and AppImage).
- Python is now embedded in the Windows Installer, No need to install Python on the user machine.

## Makhber 0.9.2

Released: 10-04-2021

### General

- makhber is now available as a flatpak file (To be published to flatpak store).
- Some fixes for the snap package.

## Makhber 0.9.1

Released: 08-04-2021

### General

- makhber is now avalaible as a snap package (beta channel) from snap store.

### Core

- Drop Python2
- Support SIP >= 5


## Makhber 0.9.0

Released: 10-03-2021

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
