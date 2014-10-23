#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/qt/MVGUserLog.hpp"
#include "mayaMVG/qt/MVGQt.hpp"
#include <openMVG/multiview/triangulation_nview.hpp>
#include <maya/MArgList.h>

namespace mayaMVG
{

namespace // empty namespace
{

void triangulatePoint(const std::map<int, MPoint>& cameraToClickedCSPoints,
                      MPoint& triangulatedWSPoint)
{
    size_t cameraCount = cameraToClickedCSPoints.size();
    assert(cameraCount > 1);
    // prepare n-view triangulation data
    openMVG::Mat2X imagePoints(2, cameraCount);
    std::vector<openMVG::Mat34> projectiveCameras;

    std::map<int, MPoint>::const_iterator it = cameraToClickedCSPoints.begin();
    for(size_t i = 0; it != cameraToClickedCSPoints.end(); ++i, ++it)
    {
        MVGCamera camera(it->first);
        // clicked point matrix (image space)
        MPoint clickedISPosition;
        MVGGeometryUtil::cameraToImageSpace(camera, it->second, clickedISPosition);
        imagePoints.col(i) = openMVG::Vec2(clickedISPosition.x, clickedISPosition.y);
        // projective camera vector
        projectiveCameras.push_back(camera.getPinholeCamera()._P);
    }
    // call n-view triangulation function
    openMVG::Vec4 result;
    openMVG::TriangulateNViewAlgebraic(imagePoints, projectiveCameras, &result);
    triangulatedWSPoint.x = result(0);
    triangulatedWSPoint.y = result(1);
    triangulatedWSPoint.z = result(2);
    assert(result(3) != 0.0);
    triangulatedWSPoint = triangulatedWSPoint / result(3);
}

} // empty namespace

MTypeId MVGMoveManipulator::_id(0x99222); // FIXME
MString MVGMoveManipulator::_drawDbClassification("drawdb/geometry/moveManipulator");
MString MVGMoveManipulator::_drawRegistrantID("moveManipulatorNode");
MVGMoveManipulator::MoveMode MVGMoveManipulator::_mode = kNViewTriangulation;

void* MVGMoveManipulator::creator()
{
    return new MVGMoveManipulator();
}

MStatus MVGMoveManipulator::initialize()
{
    return MS::kSuccess;
}

void MVGMoveManipulator::postConstructor()
{
    registerForMouseMove();
}

void MVGMoveManipulator::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
                              M3dView::DisplayStatus dispStatus)
{
    view.beginGL();

    // enable gl picking (this will enable the calls to doPress/doRelease)
    MGLuint glPickableItem;
    glFirstHandle(glPickableItem);
    colorAndName(view, glPickableItem, true, mainColor());
    // FIXME should not do these kind of things
    MVGDrawUtil::begin2DDrawing(view.portWidth(), view.portHeight());
    MVGDrawUtil::drawCircle2D(MPoint(0, 0), MColor(0, 0, 0), 1, 5);
    MVGDrawUtil::end2DDrawing();

    // retrieve a nice GL state
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_LINE_STIPPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    MPointArray onPressWSPoints;
    MPointArray intermediateCSPoints;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kMeshVertComponent:
            intermediateCSPoints.append(getMousePosition(view));
            onPressWSPoints.append(_onPressIntersectedComponent.vertex->worldPosition);
            break;
        case MFn::kMeshEdgeComponent:
            getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPosition,
                                        intermediateCSPoints);
            onPressWSPoints.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
            onPressWSPoints.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
            break;
        default:
            break;
    }

    { // 2D drawing

        MVGDrawUtil::begin2DDrawing(view.portWidth(), view.portHeight());
        MPoint mouseVSPosition = getMousePosition(view, kView);

        // Draw in active view
        if(MVGMayaUtil::isActiveView(view))
            drawCursor(mouseVSPosition);
        // Draw in MayaMVG viewports
        if(!MVGMayaUtil::isMVGView(view))
        {
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        drawPlacedPoints(view, _cache);

        // Draw in active MayaMVG viewport
        if(!MVGMayaUtil::isActiveView(view))
        {
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        // draw intersection
        MPointArray intersectedVSPoints;
        getIntersectedPositions(view, intersectedVSPoints, MVGManipulator::kView);
        MVGManipulator::drawIntersection2D(intersectedVSPoints);

        // draw triangulation
        if(_mode == kNViewTriangulation)
            MVGDrawUtil::drawTriangulatedPoints(
                view, onPressWSPoints,
                MVGGeometryUtil::cameraToViewSpace(view, intermediateCSPoints));

        MVGDrawUtil::end2DDrawing();
    }

    MVGDrawUtil::drawFinalWSPoints(_finalWSPositions, onPressWSPoints);

    glDisable(GL_BLEND);
    view.endGL();
}

MStatus MVGMoveManipulator::doPress(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doPress(view);
    // use only the left mouse button
    if(!(QApplication::mouseButtons() & Qt::LeftButton))
        return MS::kFailure;

    // set this view as the active view
    _cache->setActiveView(view);

    // check if we intersect w/ a mesh component
    _onPressCSPosition = getMousePosition(view);
    if(!_cache->checkIntersection(10.0, _onPressCSPosition))
    {
        _onPressIntersectedComponent = _cache->getIntersectedComponent();
        return MPxManipulatorNode::doPress(view);
    }

    // store the intersected component
    _onPressIntersectedComponent = _cache->getIntersectedComponent();

    // compute final positions
    computeFinalWSPositions(view);

    return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doRelease(view);

    if(_onPressIntersectedComponent.type == MFn::kInvalid) // not moving a component
        return MPxManipulatorNode::doRelease(view);

    // compute the final vertex/edge position depending on move mode
    computeFinalWSPositions(view);

    // prepare commands data
    MIntArray indices;
    MPointArray clickedCSPoints;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kMeshVertComponent:
        {
            indices.append(_onPressIntersectedComponent.vertex->index);
            clickedCSPoints.append(getMousePosition(view));
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            indices.append(_onPressIntersectedComponent.edge->vertex1->index);
            indices.append(_onPressIntersectedComponent.edge->vertex2->index);
            getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPosition,
                                        clickedCSPoints);
            break;
        }
        default:
            break;
    }
    MVGEditCmd* cmd = newEditCmd();
    if(cmd)
    {
        cmd->move(_onPressIntersectedComponent.meshPath, indices, _finalWSPositions,
                  clickedCSPoints, _cache->getActiveCamera().getId());
        MArgList args;
        if(cmd->doIt(args))
            cmd->finalize();
    }

    // clear the intersected component (stored on mouse press)
    _onPressIntersectedComponent = MVGManipulatorCache::IntersectedComponent();
    _finalWSPositions.clear();
    _cache->rebuildMeshesCache();

    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{
    _cache->checkIntersection(10.0, getMousePosition(view));
    //    _onMoveIntersectedComponent = _cache->getIntersectedComponent();
    return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{
    computeFinalWSPositions(view);
    return MPxManipulatorNode::doDrag(view);
}

void MVGMoveManipulator::computeFinalWSPositions(M3dView& view)
{
    // clear last computed positions
    _finalWSPositions.clear();

    // TODO in case we are intersecting a component of the same type, return this component
    // positions

    // compute final vertex/edge positions
    switch(_mode)
    {
        case kNViewTriangulation:
        {
            switch(_onPressIntersectedComponent.type)
            {
                case MFn::kMeshVertComponent:
                {
                    MPointArray intermediateCSPositions;
                    intermediateCSPositions.append(getMousePosition(view));
                    MPoint triangulatedWSPoint;
                    if(triangulate(view, _onPressIntersectedComponent.vertex,
                                   intermediateCSPositions[0], triangulatedWSPoint))
                        _finalWSPositions.append(triangulatedWSPoint);
                    break;
                }
                case MFn::kMeshEdgeComponent:
                {
                    MPoint triangulatedWSPoint;
                    bool vertex1Computed = false;
                    bool vertex2Computed = false;
                    MPointArray intermediateCSPositions;
                    getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                                _onPressCSPosition, intermediateCSPositions);
                    if(triangulate(view, _onPressIntersectedComponent.edge->vertex1,
                                   intermediateCSPositions[0], triangulatedWSPoint))
                    {
                        vertex1Computed = true;
                        _finalWSPositions.append(triangulatedWSPoint);
                    }
                    if(triangulate(view, _onPressIntersectedComponent.edge->vertex2,
                                   intermediateCSPositions[1], triangulatedWSPoint))
                    {
                        vertex2Computed = true;
                        _finalWSPositions.append(triangulatedWSPoint);
                    }
                    // in case we can move only one vertex
                    if(_finalWSPositions.length() == 1)
                    {
                        MVector edgeWS = _onPressIntersectedComponent.edge->vertex2->worldPosition -
                                         _onPressIntersectedComponent.edge->vertex1->worldPosition;
                        if(vertex1Computed)
                            _finalWSPositions.append(_finalWSPositions[0] + edgeWS);
                        if(vertex2Computed)
                        {
                            _finalWSPositions.append(_finalWSPositions[0]);
                            _finalWSPositions[0] = _finalWSPositions[1] - edgeWS;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case kPointCloudProjection:
        {
            MVGMesh mesh(_onPressIntersectedComponent.meshPath);
            switch(_onPressIntersectedComponent.type)
            {
                case MFn::kMeshVertComponent:
                {
                    MPointArray intermediateCSPositions;
                    intermediateCSPositions.append(getMousePosition(view));
                    MIntArray connectedFacesIDs =
                        mesh.getConnectedFacesToVertex(_onPressIntersectedComponent.vertex->index);
                    if(connectedFacesIDs.length() < 1)
                        return;
                    MIntArray verticesIDs = mesh.getFaceVertices(connectedFacesIDs[0]);
                    MPointArray cameraSpacePoints;
                    int movingVertexIDInThisFace = -1;
                    for(size_t i = 0; i < verticesIDs.length(); ++i)
                    {
                        // replace the moved vertex position with the current mouse position (camera
                        // space)
                        if(verticesIDs[i] == _onPressIntersectedComponent.vertex->index)
                        {
                            cameraSpacePoints.append(getMousePosition(view));
                            movingVertexIDInThisFace = i;
                            continue;
                        }
                        MPoint vertexWSPoint;
                        mesh.getPoint(verticesIDs[i], vertexWSPoint);
                        cameraSpacePoints.append(
                            MVGGeometryUtil::worldToCameraSpace(view, vertexWSPoint));
                    }
                    assert(movingVertexIDInThisFace != -1);
                    MPointArray worldSpacePoints;
                    MVGPointCloud cloud(MVGProject::_CLOUD);
                    if(cloud.projectPoints(view, cameraSpacePoints, worldSpacePoints))
                    {
                        // add only the moved vertex position, not the other projected vertices
                        _finalWSPositions.append(worldSpacePoints[movingVertexIDInThisFace]);
                    }
                    break;
                }
                case MFn::kMeshEdgeComponent:
                {
                    MIntArray connectedFacesIDs =
                        mesh.getConnectedFacesToEdge(_onPressIntersectedComponent.edge->index);
                    if(connectedFacesIDs.length() < 1)
                        return;
                    MIntArray verticesIDs = mesh.getFaceVertices(connectedFacesIDs[0]);
                    MPointArray intermediateCSPositions;
                    getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                                _onPressCSPosition, intermediateCSPositions);
                    MPointArray cameraSpacePoints;
                    MIntArray movingVerticesIDInThisFace;
                    for(size_t i = 0; i < verticesIDs.length(); ++i)
                    {
                        // replace the moved vertex position with the current mouse position (camera
                        // space)
                        if(verticesIDs[i] == _onPressIntersectedComponent.edge->vertex1->index)
                        {

                            cameraSpacePoints.append(intermediateCSPositions[0]);
                            movingVerticesIDInThisFace.append(i);
                            continue;
                        }
                        if(verticesIDs[i] == _onPressIntersectedComponent.edge->vertex2->index)
                        {
                            cameraSpacePoints.append(intermediateCSPositions[1]);
                            movingVerticesIDInThisFace.append(i);
                            continue;
                        }
                        MPoint vertexWSPoint;
                        mesh.getPoint(verticesIDs[i], vertexWSPoint);
                        cameraSpacePoints.append(
                            MVGGeometryUtil::worldToCameraSpace(view, vertexWSPoint));
                    }
                    assert(movingVerticesIDInThisFace.length() == 2);
                    // Switch order if necessary
                    if(movingVerticesIDInThisFace[0] == 0 &&
                       movingVerticesIDInThisFace[1] == verticesIDs.length() - 1)
                    {
                        MIntArray tmp = movingVerticesIDInThisFace;
                        movingVerticesIDInThisFace[0] = tmp[1];
                        movingVerticesIDInThisFace[1] = tmp[0];
                    }
                    MPointArray worldSpacePoints;
                    MVGPointCloud cloud(MVGProject::_CLOUD);
                    if(cloud.projectPoints(view, cameraSpacePoints, worldSpacePoints))
                    {
                        // add only the moved vertices positions, not the other projected vertices
                        _finalWSPositions.append(worldSpacePoints[movingVerticesIDInThisFace[0]]);
                        _finalWSPositions.append(worldSpacePoints[movingVerticesIDInThisFace[1]]);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case kAdjacentFaceProjection:
        {
            MVGMesh mesh(_onPressIntersectedComponent.meshPath);
            switch(_onPressIntersectedComponent.type)
            {
                case MFn::kMeshVertComponent:
                {
                    MPointArray intermediateCSPositions;
                    intermediateCSPositions.append(getMousePosition(view));
                    // get face vertices
                    MIntArray connectedFacesIDs =
                        mesh.getConnectedFacesToVertex(_onPressIntersectedComponent.vertex->index);
                    if(connectedFacesIDs.length() < 1)
                        return;
                    MIntArray verticesIDs = mesh.getFaceVertices(connectedFacesIDs[0]);
                    MPointArray faceWSPoints;
                    for(size_t i = 0; i < verticesIDs.length(); ++i)
                    {
                        MPoint vertexWSPoint;
                        mesh.getPoint(verticesIDs[i], vertexWSPoint);
                        faceWSPoints.append(vertexWSPoint);
                    }
                    // compute moved point
                    MPoint projectedWSPoint;
                    if(MVGGeometryUtil::projectPointOnPlane(view, intermediateCSPositions[0],
                                                            faceWSPoints, projectedWSPoint))
                        _finalWSPositions.append(projectedWSPoint);
                    break;
                }
                case MFn::kMeshEdgeComponent:
                {
                    MIntArray connectedFacesIDs =
                        mesh.getConnectedFacesToEdge(_onPressIntersectedComponent.edge->index);
                    if(connectedFacesIDs.length() < 1)
                        return;
                    MIntArray verticesIDs = mesh.getFaceVertices(connectedFacesIDs[0]);
                    MPointArray faceWSPoints;
                    for(size_t i = 0; i < verticesIDs.length(); ++i)
                    {
                        MPoint vertexWSPoint;
                        mesh.getPoint(verticesIDs[i], vertexWSPoint);
                        faceWSPoints.append(vertexWSPoint);
                    }
                    // compute moved point
                    MPointArray intermediateCSPositions;
                    getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                                _onPressCSPosition, intermediateCSPositions);
                    MVGGeometryUtil::projectPointsOnPlane(view, intermediateCSPositions,
                                                          faceWSPoints, _finalWSPositions);
                    break;
                }
                default:
                    break;
            }
            break;
        }
    }
}

bool MVGMoveManipulator::triangulate(M3dView& view, MVGManipulatorCache::VertexData* vertex,
                                     const MPoint& currentVertexPositionsInActiveView,
                                     MPoint& triangulatedWSPoint)
{
    // retrieve blind data
    std::map<int, MPoint> blindData = vertex->blindData;
    // override blind data for the active camera
    blindData[_cache->getActiveCamera().getId()] = currentVertexPositionsInActiveView;
    if(blindData.size() < 2)
        return false;
    triangulatePoint(blindData, triangulatedWSPoint);
    return true;
}

// static
void MVGMoveManipulator::drawCursor(const MPoint& originVS)
{
    MVGDrawUtil::drawArrowsCursor(originVS, MVGDrawUtil::_cursorColor);

    MPoint offsetMouseVSPosition = originVS + MPoint(10, 10);
    switch(_mode)
    {
        case MVGMoveManipulator::kNViewTriangulation:
            MVGDrawUtil::drawFullCross(offsetMouseVSPosition, 5, 1, MVGDrawUtil::_triangulateColor);
            break;
        case MVGMoveManipulator::kPointCloudProjection:
            MVGDrawUtil::drawPointCloudCursorItem(offsetMouseVSPosition,
                                                  MVGDrawUtil::_pointCloudColor);
            break;
        case MVGMoveManipulator::kAdjacentFaceProjection:
            MVGDrawUtil::drawPlaneCursorItem(offsetMouseVSPosition,
                                             MVGDrawUtil::_adjacentFaceColor);
            break;
    }
}
// static
void MVGMoveManipulator::drawPlacedPoints(M3dView& view, MVGManipulatorCache* cache)
{
    MDagPath cameraPath;
    view.getCamera(cameraPath);
    MVGCamera camera(cameraPath);

    std::map<std::string, MVGManipulatorCache::MeshData> meshData = cache->getMeshData();
    // browse meshes
    std::map<std::string, MVGManipulatorCache::MeshData>::iterator it = meshData.begin();
    for(; it != meshData.end(); ++it)
    {
        // browse vertices
        std::vector<MVGManipulatorCache::VertexData>& vertices = it->second.vertices;
        for(std::vector<MVGManipulatorCache::VertexData>::iterator verticesIt = vertices.begin();
            verticesIt != vertices.end(); ++verticesIt)
        {
            std::map<int, MPoint>::iterator currentData =
                verticesIt->blindData.find(camera.getId());
            if(currentData == verticesIt->blindData.end())
                continue;

            // 2D position
            MPoint clickedVSPoint =
                MVGGeometryUtil::cameraToViewSpace(view, verticesIt->blindData[camera.getId()]);
            MVGDrawUtil::drawFullCross(clickedVSPoint, 10, 1.5, MVGDrawUtil::_triangulateColor);
            // Link between 2D/3D positions
            MPoint vertexVS = MVGGeometryUtil::worldToViewSpace(view, verticesIt->worldPosition);
            MVGDrawUtil::drawLine2D(clickedVSPoint, vertexVS, MVGDrawUtil::_triangulateColor, 1.5f,
                                    1.f, true);

            // Number of placed points
            MString nbView;
            nbView += (int)(verticesIt->blindData.size());
            view.drawText(nbView,
                          MVGGeometryUtil::viewToWorldSpace(view, clickedVSPoint + MPoint(5, 5)));
        }
    }
}
} // namespace
