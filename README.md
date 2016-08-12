MayaMVG
==========

MayaMVG is a Maya plugin that enables to model 3D objects from images.

Requirements
============
* Compiler compatible with your version of Maya
* cmake
* Maya
* Qt compatible with you version of Maya (Qt4.8 until Maya 2016)

Installation
============
```
> git submodule update --init --recursive
> ./configure -DMAYA_EXECUTABLE=$MAYA_BIN -DQT_QMAKE_EXECUTABLE=$MAYA_QMAKE_PATH -DCMAKE_INSTALL_PREFIX=$INSTALL_DIRECTORY -DCMAKE_BUILD_TYPE=release
> make
> make install
```


Documentation
=============
Plugin documentation is available [here](doc/Documentation.v0.4.2.md)


Development
===========
Clang format rules for MayaMVG can be found [here](.clang-format).
To format code easily, install Clang (dont forget to add bin path to PATH) and execute :
```
> ./format_all_files.sh
```
