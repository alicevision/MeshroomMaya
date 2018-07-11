#pragma once

#include <maya/MPxLocatorNode.h>
#include <maya/MTypeId.h>

class MVGDummyLocator : public MPxLocatorNode
{

public:
    MVGDummyLocator();
    virtual ~MVGDummyLocator();

public:
    virtual void postConstructor();
    static void* creator();
    static MStatus initialize();
    virtual void draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
                      M3dView::DisplayStatus status);

public:
    static MTypeId _id;
};
