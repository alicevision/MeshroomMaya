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
    MVGMoveManipulator() {}
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
    static void drawCursor(const MPoint& originVS);
    static void
    drawPlacedPoints(M3dView& view,
                     const std::map<std::string, MVGManipulatorCache::MeshData>& meshData,
                     const MVGManipulatorCache::IntersectedComponent& onPressIntersectedComponent);
    static void drawVertexOnHover(M3dView& view, MVGManipulatorCache* cache,
                                  const MPoint& mouseVSPosition);

public:
    static MTypeId _id;
    static MString _drawDbClassification;
    static MString _drawRegistrantID;
    static MoveMode _mode;
    bool _doDrag;
};

} // namespace
