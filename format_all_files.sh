#!/bin/bash
find ./src/mayaMVG -type f \( -iname \*.?pp -o -iname \*.h \) -exec clang-format -i {} \;
