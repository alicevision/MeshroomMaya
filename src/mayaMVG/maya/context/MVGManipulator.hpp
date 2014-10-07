#pragma once

#include "mayaMVG/maya/context/MVGManipulatorCache.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
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
    MVGManipulator() {}
    virtual ~MVGManipulator() {}

public:
    void setContext(MVGContext* c) { _context = c; }
    void setCache(MVGManipulatorCache* m) { _cache = m; }

protected:
    MVGEditCmd* newEditCmd();
    MPoint getMousePosition(M3dView&, Space = kCamera);
    void getMousePosition(M3dView&, MPoint&, Space = kCamera);
    MPointArray getIntersectedPositions(M3dView&, Space = kCamera);
    void getIntersectedPositions(M3dView&, MPointArray&, Space = kCamera);
    void getOnMoveCSEdgePoints(M3dView& view, const MVGManipulatorCache::EdgeData* onPressEdgeData,
                               const MPoint& onPressCSPoint, MPointArray& onMoveCSEdgePoints);
    MPointArray getOnMoveCSEdgePoints(M3dView& view,
                                      const MVGManipulatorCache::EdgeData* onPressEdgeData,
                                      const MPoint& onPressCSPoint);
    void getTranslatedWSEdgePoints(M3dView& view,
                                   const MVGManipulatorCache::EdgeData* originEdgeData,
                                   MPoint& originCSPosition, MPoint& targetWSPosition,
                                   MPointArray& targetEdgeWSPositions);
    void drawIntersection(M3dView& view) const;

protected:
    virtual void computeFinalWSPositions(M3dView& view) = 0;

protected:
    MVGManipulatorCache* _cache;
    MVGManipulatorCache::IntersectedComponent _onPressIntersectedComponent;
    MPoint _onPressCSPosition;
    MPointArray _finalWSPositions;

private:
    MVGContext* _context;
};

} // namespace
