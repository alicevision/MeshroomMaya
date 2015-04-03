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
        : _doDrag(false)
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
    void computeFinalWSPoints(M3dView& view);
    void computeTriangulatedPoints(M3dView& view, MPointArray& finalWSPoints);
    void computePCPoints(M3dView& view, MPointArray& finalWSPoints);
    void computeAdjacentPoints(M3dView& view, MPointArray& finalWSPoints);
    MStatus storeTweakInformation();
    MStatus resetTweakInformation();
    bool triangulate(M3dView& view, MVGManipulatorCache::VertexData* vertex,
                     const MPoint& currentVertexPositionsInActiveView, MPoint& triangulatedWSPoint);

public:
    static void drawCursor(const MPoint& originVS);
    static void
    drawPlacedPoints(M3dView& view, const MVGCamera& camera, MVGManipulatorCache* cache,
                     const MVGManipulatorCache::MVGComponent& onPressIntersectedComponent);
    static void drawComplementaryIntersectedBlindData(
        M3dView& view, const MVGCamera& camera,
        const MVGManipulatorCache::MVGComponent& intersectedComponent);
    static void drawVertexOnHover(M3dView& view, MVGManipulatorCache* cache,
                                  const MPoint& mouseVSPosition);
    static void drawSelectedPoint2D(M3dView& view, const MVGCamera& camera,
                                    const MVGManipulatorCache::MVGComponent& selectedComponent);
    static void drawSelectedPoint3D(M3dView& view,
                                    const MVGManipulatorCache::MVGComponent& selectedComponent);
    static void drawPointToBePlaced(M3dView& view, const MVGCamera& camera,
                                    const MVGManipulatorCache::MVGComponent& selectedComponent,
                                    const MPoint& mouseVSPosition);

public:
    static MTypeId _id;
    static MString _drawDbClassification;
    static MString _drawRegistrantID;
    static MoveMode _mode;
    bool _doDrag;

private:
    /// 2D view space points of the moved face.
    /// It's needed to draw face wireframe even if no plane is found.
    MPointArray _intermediateVSPoints;
};

} // namespace
