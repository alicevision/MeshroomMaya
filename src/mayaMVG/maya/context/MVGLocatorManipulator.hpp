#pragma once

#include "mayaMVG/maya/context/MVGManipulator.hpp"
#include <maya/MPointArray.h>

namespace mayaMVG
{

class MVGEditCmd;

class MVGLocatorManipulator : public MVGManipulator
{
public:
    MVGLocatorManipulator() {}
    virtual ~MVGLocatorManipulator() {}

public:
    static void* creator();
    static MStatus initialize();

public:
    virtual void postConstructor();
    virtual void draw(M3dView&, const MDagPath&, M3dView::DisplayStyle, M3dView::DisplayStatus);
    virtual MStatus doPress(M3dView& view);
    virtual MStatus doRelease(M3dView& view);
    virtual MStatus doMove(M3dView& view, bool& refresh);
    virtual MStatus doDrag(M3dView& view);

public:
    void computeFinalWSPoints(M3dView& view);
    const std::map<int, MPoint>& getCameraIDToClickedCSPoint() { return _cameraIDToClickedCSPoint; }
    void clearCameraIDToClickedCSPoint() { _cameraIDToClickedCSPoint.clear(); }

    static void drawCursor(const MPoint& originVS);

private:
    void createLocator(const MString& locatorName);
    void setLocator();

public:
    static MTypeId _id;
    static MString _drawDbClassification;
    static MString _drawRegistrantID;

private:
    std::map<int, MPoint> _cameraIDToClickedCSPoint; // cameraID to clicked points for locator
};

} // namespace
