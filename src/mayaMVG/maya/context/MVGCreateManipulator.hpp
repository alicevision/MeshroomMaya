#pragma once

#include "mayaMVG/maya/context/MVGManipulator.hpp"
#include <maya/MPointArray.h>

namespace mayaMVG
{

class MVGEditCmd;

class MVGCreateManipulator : public MVGManipulator
{
public:
    MVGCreateManipulator() {}
    virtual ~MVGCreateManipulator() {}

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
    MPointArray getClickedVSPoints() const;

private:
    void computeFinalWSPoints(M3dView& view);
    bool computePCPoints(M3dView& view, MPointArray& finalWSPoints,
                         const MPointArray& intermediateCSEdgePoints);
    bool computeAdjacentPoints(M3dView& view, MPointArray& finalWSPoints,
                               const MPointArray& intermediateCSEdgePoints);
    bool snapToIntersectedEdge(M3dView& view, MPointArray& finalWSPoints,
                               const MVGManipulatorCache::IntersectedComponent& intersectedEdge);
    bool snapToIntersectedVertex(M3dView& view, MPointArray& finalWSPoints,
                                 const MPointArray& intermediateCSEdgePoints);

public:
    static void drawCursor(const MPoint& originVS, MVGManipulatorCache* cache);

public:
    static MTypeId _id;
    static MString _drawDbClassification;
    static MString _drawRegistrantID;

private:
    std::pair<int, MPointArray> _cameraIDToClickedCSPoints; // cameraID to clicked points
};

} // namespace
