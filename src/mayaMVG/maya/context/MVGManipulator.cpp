#include "mayaMVG/maya/context/MVGManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"
namespace mayaMVG
{

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

const MPointArray& MVGManipulator::getFinalWSPositions() const
{
    return _finalWSPositions;
}

void MVGManipulator::getIntersectedPositions(M3dView& view, MPointArray& positions,
                                             MVGManipulator::Space space) const
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
    if(intersectedPositions.length() <= 0)
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
            break;
    }
    for(size_t i = 0; i < intersectedPositions.length(); ++i)
        positions.append(intersectedPositions[i]);
}

MPointArray MVGManipulator::getIntersectedPositions(M3dView& view,
                                                    MVGManipulator::Space space) const
{
    MPointArray positions;
    getIntersectedPositions(view, positions, space);
    return positions;
}

void MVGManipulator::getIntermediateCSEdgePoints(
    M3dView& view, const MVGManipulatorCache::EdgeData* onPressEdgeData,
    const MPoint& onPressCSPoint, MPointArray& intermediateCSEdgePoints)
{
    assert(onPressEdgeData != NULL);
    // vertex 1
    MVector mouseToVertexCSOffset =
        MVGGeometryUtil::worldToCameraSpace(view, onPressEdgeData->vertex1->worldPosition) -
        onPressCSPoint;
    intermediateCSEdgePoints.append(getMousePosition(view) + mouseToVertexCSOffset);
    // vertex 2
    mouseToVertexCSOffset =
        MVGGeometryUtil::worldToCameraSpace(view, onPressEdgeData->vertex2->worldPosition) -
        onPressCSPoint;
    intermediateCSEdgePoints.append(getMousePosition(view) + mouseToVertexCSOffset);
}

MPointArray
MVGManipulator::getIntermediateCSEdgePoints(M3dView& view,
                                            const MVGManipulatorCache::EdgeData* onPressEdgeData,
                                            const MPoint& onPressCSPoint)
{
    assert(onPressEdgeData != NULL);
    MPointArray intermediateCSEdgePoints;
    getIntermediateCSEdgePoints(view, onPressEdgeData, onPressCSPoint, intermediateCSEdgePoints);
    return intermediateCSEdgePoints;
}

void MVGManipulator::getTranslatedWSEdgePoints(M3dView& view,
                                               const MVGManipulatorCache::EdgeData* originEdgeData,
                                               MPoint& originCSPosition, MPoint& targetWSPosition,
                                               MPointArray& targetEdgeWSPositions) const
{
    assert(originEdgeData != NULL);
    MVector edgeCSVector =
        MVGGeometryUtil::worldToCameraSpace(view, originEdgeData->vertex1->worldPosition) -
        MVGGeometryUtil::worldToCameraSpace(view, originEdgeData->vertex2->worldPosition);
    MVector vertex1ToMouseCSVector =
        originCSPosition -
        MVGGeometryUtil::worldToCameraSpace(view, originEdgeData->vertex1->worldPosition);
    float ratioVertex1 = vertex1ToMouseCSVector.length() / edgeCSVector.length();
    float ratioVertex2 = 1.f - ratioVertex1;

    MVector edgeWSVector =
        originEdgeData->vertex1->worldPosition - originEdgeData->vertex2->worldPosition;
    targetEdgeWSPositions.append(targetWSPosition + ratioVertex1 * edgeWSVector);
    targetEdgeWSPositions.append(targetWSPosition - ratioVertex2 * edgeWSVector);
}

MVGEditCmd* MVGManipulator::newEditCmd()
{
    return dynamic_cast<MVGEditCmd*>(_context->newCmd());
}

// static
void MVGManipulator::drawIntersection2D(const MPointArray& intersectedVSPoints)
{
    assert(intersectedVSPoints.length() < 3);
    if(intersectedVSPoints.length() <= 0)
        return;
    // draw vertex
    if(intersectedVSPoints.length() < 2)
    {
        MVGDrawUtil::drawCircle2D(intersectedVSPoints[0], MColor(1, 1, 1), 10, 30);
        return;
    }
    // draw edge
    MVGDrawUtil::drawLine2D(intersectedVSPoints[0], intersectedVSPoints[1], MColor(1, 1, 1));
}

} // namespace
