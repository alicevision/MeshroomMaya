MayaMVG
==========

MayaMVG is a Maya plugin that enables to model 3D objects from images.

Requirements
============
* GXX
* cmake
* Maya
* qmake

Installation
============
```
> ./configure -DMAYA_EXECUTABLE=$MAYA_BIN -DQT_QMAKE_EXECUTABLE=$MAYA_QMAKE_PATH -DCMAKE_INSTALL_PREFIX=$INSTALL_DIRECTORY -DCMAKE_BUILD_TYPE=release
> make
> make install
