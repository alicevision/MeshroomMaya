#pragma once

#include <maya/MPxCommand.h>

namespace mayaMVG
{

class MVGImagePlaneCmd : public MPxCommand
{

public:
    MVGImagePlaneCmd(){};
    virtual ~MVGImagePlaneCmd(){};

    static void* creator();
    static MSyntax newSyntax();
    virtual bool hasSyntax() const { return true; }

    virtual MStatus doIt(const MArgList& args);
    virtual bool isUndoable() const;

public:
    static MString _name;

private:
};

} // namespace
