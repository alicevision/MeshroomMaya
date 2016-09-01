#pragma once

#include <maya/MPxCommand.h>

namespace mayaMVG
{

class MVGSelectClosestCamCmd : public MPxCommand
{

public:
    MVGSelectClosestCamCmd(){};

    static void* creator();
    virtual MStatus doIt(const MArgList& args);

public:
    static MString _name;

};

}