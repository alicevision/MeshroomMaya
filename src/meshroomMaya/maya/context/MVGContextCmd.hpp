#pragma once

#include <maya/MPxContextCommand.h>

namespace meshroomMaya
{

class MVGContext;

class MVGContextCmd : public MPxContextCommand
{
public:
    static MString name;
    static MString instanceName;

public:
    MVGContextCmd();

public:
    virtual MPxContext* makeObj();
    static void* creator();
    virtual MStatus doEditFlags();
    virtual MStatus doQueryFlags();
    virtual MStatus appendSyntax();

private:
    MVGContext* _context;
};

} // namespace
