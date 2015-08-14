#pragma once

#include "mayaMVG/maya/context/MVGManipulatorCache.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/core/MVGPointCloudItem.hpp"
#include "mayaMVG/maya/cmd/MVGEditCmd.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include <maya/MPxManipulatorNode.h>

namespace mayaMVG
{
class MVGManipulator : public MPxManipulatorNode
{
public:
    enum Space
    {
        kWorld = 0,
        kCamera,
        kView
    };

public:
    MVGManipulator();
    virtual ~MVGManipulator() {}

public:
    void setContext(MVGContext* c) { _context = c; }
    MVGManipulatorCache* getCache() const { return _cache; }
    void setCache(MVGManipulatorCache* m) { _cache = m; }

public:
    MPoint getMousePosition(M3dView&, Space = kCamera);
    void getMousePosition(M3dView&, MPoint&, Space = kCamera);
    const MPointArray& getFinalWSPoints() const;
    const MPointArray& getIntermediateCSPoints() const;
    const MPointArray getIntersectedPoints(M3dView&, Space = kCamera) const;
    void getIntersectedPoints(M3dView&, MPointArray&, Space = kCamera) const;
    void getIntermediateCSEdgePoints(M3dView& view,
                                     const MVGManipulatorCache::EdgeData* onPressEdgeData,
                                     const MPoint& onPressCSMousePos,
                                     MPointArray& intermediateCSEdgePoints);
    const MPointArray
    getIntermediateCSEdgePoints(M3dView& view, const MVGManipulatorCache::EdgeData* onPressEdgeData,
                                const MPoint& onPressCSPoint);
    void getTranslatedWSEdgePoints(M3dView& view,
                                   const MVGManipulatorCache::EdgeData* originEdgeData,
                                   MPoint& originCSPosition, MPoint& targetWSPosition,
                                   MPointArray& targetEdgeWSPositions) const;

public:
    static void drawIntersection2D(const MPointArray& intersectedVSPoints,
                                   const MFn::Type intersectionType);
    void drawVisibleItems(M3dView& view) const;

protected:
    MVGEditCmd* newEditCmd();
    void drawIntersection() const;
    virtual void computeFinalWSPoints(M3dView& view) = 0;

protected:
    MVGManipulatorCache* _cache;
    MVGManipulatorCache::MVGComponent _onPressIntersectedComponent;
    MPoint _onPressCSPoint;
    MPointArray _finalWSPoints;
    int _cameraID;
    std::vector<MVGPointCloudItem> _visiblePointCloudItems;
    MIntArray _snapedPoints;

public:
    static bool _displayVisiblePoints;

private:
    MVGContext* _context;
};

} // namespace
