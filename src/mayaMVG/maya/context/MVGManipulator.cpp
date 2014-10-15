#include "mayaMVG/maya/context/MVGManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"

namespace mayaMVG
{

MVGEditCmd* MVGManipulator::newEditCmd()
{
    return dynamic_cast<MVGEditCmd*>(_context->newCmd());
}

void MVGManipulator::getMousePosition(M3dView& view, MPoint& point, MVGManipulator::Space space)
{
    short x, y;
    mousePosition(x, y);
    switch(space)
    {
        case kWorld:
            point = MVGGeometryUtil::viewToWorldSpace(view, MPoint(x, y));
            break;
        case kCamera:
            point = MVGGeometryUtil::viewToCameraSpace(view, MPoint(x, y));
            break;
        case kView:
            point = MPoint(x, y);
            break;
    }
}

MPoint MVGManipulator::getMousePosition(M3dView& view, MVGManipulator::Space space)
{
    MPoint position;
    getMousePosition(view, position, space);
    return position;
}

void MVGManipulator::getIntersectedPositions(M3dView& view, MPointArray& positions,
                                             MVGManipulator::Space space)
{
    MPointArray intersectedPositions;
    MVGManipulatorCache::IntersectedComponent intersectedComponent =
        _cache->getIntersectedComponent();
    switch(intersectedComponent.type)
    {
        case MFn::kMeshVertComponent:
            intersectedPositions.append(intersectedComponent.vertex->worldPosition);
            break;
        case MFn::kMeshEdgeComponent:
            intersectedPositions.append(intersectedComponent.edge->vertex1->worldPosition);
            intersectedPositions.append(intersectedComponent.edge->vertex2->worldPosition);
            break;
        default:
            break;
    }
    if(intersectedPositions.length() == 0)
        return;
    switch(space)
    {
        case kCamera:
            intersectedPositions = MVGGeometryUtil::worldToCameraSpace(view, intersectedPositions);
            break;
        case kView:
            intersectedPositions = MVGGeometryUtil::worldToViewSpace(view, intersectedPositions);
            break;
        case kWorld:
        default:
            break;
    }
    for(size_t i = 0; i < intersectedPositions.length(); ++i)
        positions.append(intersectedPositions[i]);
}

MPointArray MVGManipulator::getIntersectedPositions(M3dView& view, MVGManipulator::Space space)
{
    MPointArray positions;
    getIntersectedPositions(view, positions, space);
    return positions;
}

void MVGManipulator::getOnMoveCSEdgePoints(M3dView& view,
                                           const MVGManipulatorCache::EdgeData* onPressEdgeData,
                                           const MPoint& onPressCSPoint,
                                           MPointArray& onMoveCSEdgePoints)
{
    // vertex 1
    MVector mouseToVertexCSOffset =
        MVGGeometryUtil::worldToCameraSpace(view, onPressEdgeData->vertex1->worldPosition) -
        onPressCSPoint;
    onMoveCSEdgePoints.append(getMousePosition(view) + mouseToVertexCSOffset);
    // vertex 2
    mouseToVertexCSOffset =
        MVGGeometryUtil::worldToCameraSpace(view, onPressEdgeData->vertex2->worldPosition) -
        onPressCSPoint;
    onMoveCSEdgePoints.append(getMousePosition(view) + mouseToVertexCSOffset);
}

MPointArray MVGManipulator::getOnMoveCSEdgePoints(M3dView& view,
                                      const MVGManipulatorCache::EdgeData* onPressEdgeData,
                                      const MPoint& onPressCSPoint)
{
    MPointArray onMoveCSEdgePoints;
    getOnMoveCSEdgePoints(view, onPressEdgeData, onPressCSPoint, onMoveCSEdgePoints);
    return onMoveCSEdgePoints;
}

void MVGManipulator::drawIntersection(M3dView& view) const
{
    MVGManipulatorCache::IntersectedComponent intersectedComponent =
        _cache->getIntersectedComponent();
    switch(intersectedComponent.type)
    {
        case MFn::kMeshVertComponent:
        {
            MPoint realVSVertexPosition;
            MVGGeometryUtil::worldToViewSpace(view, intersectedComponent.vertex->worldPosition,
                                              realVSVertexPosition);
            MVGDrawUtil::drawCircle2D(realVSVertexPosition, MColor(1, 1, 1), 10, 30);
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            MPoint edgeVSPosition1;
            MPoint edgeVSPosition2;
            MVGGeometryUtil::worldToViewSpace(
                view, intersectedComponent.edge->vertex1->worldPosition, edgeVSPosition1);
            MVGGeometryUtil::worldToViewSpace(
                view, intersectedComponent.edge->vertex2->worldPosition, edgeVSPosition2);
            MVGDrawUtil::drawLine2D(edgeVSPosition1, edgeVSPosition2, MColor(1, 1, 1));
            break;
        }
        default:
            break;
    }
}

} // namespace
