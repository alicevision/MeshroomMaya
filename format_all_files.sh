#!/bin/bash
find ./src/meshroomMaya -type f \( -iname \*.?pp -o -iname \*.h \) -exec clang-format -i {} \;
