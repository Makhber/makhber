# Makhber - Installation Notes


## Generic requirements

In order to compile Makhber, you need to install the following
libraries. Easiest is to use your package manager to install prebuilt versions.
- Qt >= 5.14
- GSL
- muParser
- zlib

For the optional Python scripting feature, you also need:
- Python >= 3.7
- PyQt >= 5.14
- sip >= 4.19

## Linux

1. In the top level directory:
```SHELL
mkdir build && cd build
cmake .. <Options>
make
```

## Windows - MSVC

TODO

## macOS

TODO
