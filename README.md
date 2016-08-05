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
> ./configure -DMAYA_EXECUTABLE=$MAYA_BIN -DQT_QMAKE_EXECUTABLE=$MAYA_QMAKE_PATH -DCMAKE_INSTALL_PREFIX=$INSTALL_DIRECTORY -DCMAKE_BUILD_TYPE=release
> make
> make install
```


Documentation
=============
Plugin documentation is available [here](doc/Documentation.v0.4.2.md)