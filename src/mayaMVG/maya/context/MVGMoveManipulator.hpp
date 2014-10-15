#pragma once

#include "mayaMVG/maya/context/MVGManipulator.hpp"

namespace mayaMVG
{

class MVGMoveManipulator : public MVGManipulator
{
public:
    enum MoveMode
    {
        kNViewTriangulation = 0,
        kPointCloudProjection,
        kAdjacentFaceProjection
    };

public:
    MVGMoveManipulator()
        : _mode(kNViewTriangulation)
    {
    }
    virtual ~MVGMoveManipulator() {}

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

private:
    void computeFinalWSPositions(M3dView& view);
    bool triangulate(M3dView& view, MVGManipulatorCache::VertexData* vertex,
                     const MPoint& currentVertexPositionsInActiveView, MPoint& triangulatedWSPoint);

public:
    static MTypeId _id;
    MoveMode _mode;
};

} // namespace
