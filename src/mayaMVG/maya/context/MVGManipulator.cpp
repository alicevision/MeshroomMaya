#include "mayaMVG/maya/context/MVGManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"

namespace mayaMVG
{

MVGManipulator::MVGManipulator()
{
    _cameraID = -1;
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

const MPointArray& MVGManipulator::getFinalWSPoints() const
{
    return _finalWSPoints;
}

void MVGManipulator::getIntersectedPoints(M3dView& view, MPointArray& positions,
                                          MVGManipulator::Space space) const
{
    MPointArray intersectedPositions;
    MVGManipulatorCache::MVGComponent intersectedComponent = _cache->getIntersectedComponent();
    switch(intersectedComponent.type)
    {
        case MFn::kBlindData:
        {
            MPoint pointCSPosition =
                intersectedComponent.vertex->blindData[_cache->getActiveCamera().getId()];
            intersectedPositions.append(MVGGeometryUtil::cameraToWorldSpace(view, pointCSPosition));
            break;
        }
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

const MPointArray MVGManipulator::getIntersectedPoints(M3dView& view,
                                                       MVGManipulator::Space space) const
{
    MPointArray positions;
    getIntersectedPoints(view, positions, space);
    return positions;
}

/**
 * @brief Compute the parallelogram from one edge and one point (mouse position).
 *
 * [AB]: Input clicked edge
 * P: mouse position on press
 * M: moving mouse
 * [DC]: the computed edge keeping AB length and AP==DM
 *
 *     A ____________ D
 *      /           /
 *   P +           + M
 *    /           /
 * B /___________/ C
 *
 * This computation is in 2D Camera Space.
 *
 * @param[in] view Viewort
 * @param[in] onPressEdgeData clicked edge information
 * @param[in] onPressCSMousePos clicked mouse position in Camera Space coordinates
 * @param[out] intermediateCSEdgePoints the 2 new points (D and C) of the parallelogram
 */
void MVGManipulator::getIntermediateCSEdgePoints(
    M3dView& view, const MVGManipulatorCache::EdgeData* onPressEdgeData,
    const MPoint& onPressCSMousePos, MPointArray& intermediateCSEdgePoints)
{
    assert(onPressEdgeData != NULL);
    // vertex 1
    MVector mouseToVertexCSOffset =
        MVGGeometryUtil::worldToCameraSpace(view, onPressEdgeData->vertex1->worldPosition) -
        onPressCSMousePos;
    intermediateCSEdgePoints.append(getMousePosition(view) + mouseToVertexCSOffset);
    // vertex 2
    mouseToVertexCSOffset =
        MVGGeometryUtil::worldToCameraSpace(view, onPressEdgeData->vertex2->worldPosition) -
        onPressCSMousePos;
    intermediateCSEdgePoints.append(getMousePosition(view) + mouseToVertexCSOffset);
}

const MPointArray
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
void MVGManipulator::drawIntersection2D(const MPointArray& intersectedVSPoints,
                                        const MFn::Type intersectionType)
{
    const int arrayLength = intersectedVSPoints.length();
    assert(arrayLength < 3);
    if(intersectedVSPoints.length() <= 0)
        return;

    switch(intersectionType)
    {
        case MFn::kBlindData:
            assert(arrayLength == 1);
            MVGDrawUtil::drawEmptyCross(intersectedVSPoints[0], 7, 2,
                                        MVGDrawUtil::_intersectionColor, 1.5);
            break;
        case MFn::kMeshVertComponent:
            assert(arrayLength == 1);
            MVGDrawUtil::drawCircle2D(intersectedVSPoints[0], MVGDrawUtil::_intersectionColor, 10,
                                      30);
            break;
        case MFn::kMeshEdgeComponent:
            assert(arrayLength == 2);
            MVGDrawUtil::drawLine2D(intersectedVSPoints[0], intersectedVSPoints[1],
                                    MVGDrawUtil::_intersectionColor);
            break;
        default:
            break;
    }
}

} // namespace
