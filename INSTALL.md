# Makhber - Installation Notes

## Generic requirements

In order to compile Makhber, you need to install the following
libraries. Easiest is to use your package manager to install prebuilt versions.

- Qt >= 5.12
- Qwt >= 6.1
- GSL
- muParser
- zlib

For the optional Python scripting feature, you also need:

- Python >= 3.5
- PyQt5
- sip >= 4.19
- PyQt-builder (with sip >= 5)

For testing:

- GoogleTest

## Linux

1. Install Qt using the [official online installer](https://www.qt.io/download) or using the package manager:

    ```SHELL
    sudo apt install \
      qtbase5-dev \
      libqt5svg5-dev \
      libqt5opengl5-dev \
      qttools5-dev
    ```

2. Install the required packages:

    ```SHELL
    sudo apt install libqwt-qt5-dev libgsl-dev libmuparser-dev zlib1g-dev
    ```

3. Install python modules (optional):

    Using the package manager:

    ```SHELL
    sudo apt install pyqt5-dev python3-pyqt5
    ```

    Or using pip:

    ```SHELL
    pip3 install sip pyqt5 pyqt-builder
    ```

4. Install GoogleTest (optional):

   ```SHELL
   sudo apt install libgtest-dev
   ```

5. In the top level directory:

    ```SHELL
    mkdir build && cd build
    cmake .. <CMake-Options> <Project-Options>
    make
    ```

## Windows - MSVC

1. Install Qt using the [official online installer](https://www.qt.io/download) or using [vcpkg](https://vcpkg.io)

    ```SHELL
    vcpkg install qt5-base qt5-svg qt5-tools qt5-translations --triplet x64-windows
    ```

2. Install the required packages using vcpkg:

    ```SHELL
    vcpkg install qwt gsl muparser zlib --triplet x64-windows
    ```

3. Install python modules (optional) using pip:

    ```SHELL
    pip3 install sip pyqt5 pyqt-builder
    ```

4. Install GoogleTest (optional) using vcpkg:

    ```SHELL
    vcpkg install gtest --triplet x64-windows
    ```

5. In the top level directory:

    ```SHELL
    mkdir build
    cd build
    cmake .. <CMake-Options> <Project-Options> -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
    cmake --build . --config <CONFIG>
    ```

## macOS

1. Install Qt either using the [official online installer](https://www.qt.io/download) or using [homebrew](https://brew.sh)

    ```SHELL
    brew install qt@5
    ```

2. Install the required packages using homebrew:

    ```SHELL
    brew install qwt gsl muparser
    ```

3. Install GoogleTest (optional) using vcpkg:

    ```SHELL
    brew install googletest
    ```

4. In the top level directory:

    ```SHELL
    mkdir build && cd build
    cmake .. <CMake-Options> <Project-Options>
    make
    ```

## Build Options

### CMake Options

`-DCMAKE_BUILD_TYPE=<CONFIG>` : Release or Debug

`-DCMAKE_INSTALL_PREFIX=<Path>` : Where to install the project

### Project Options

`-DMAKHBER_BUILD_TESTS=<ON/OFF>` : Enable/Disable testing (default:off)

`-DMAKHBER_SCRIPTING_PYTHON=<ON/OFF>` : Enable/Disable Python scripiting (default:off)

`-DMAKHBER_ORIGIN_IMPORT=<ON/OFF>` : Enable importing originLab projects (deafult:on)
