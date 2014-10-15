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
    if(!MVGMayaUtil::isMVGView(view) || !MVGMayaUtil::isActiveView(view))
        return;

    view.beginGL();

    // enable gl picking (this will enable the calls to doPress/doRelease)
    MGLuint glPickableItem;
    glFirstHandle(glPickableItem);
    colorAndName(view, glPickableItem, true, mainColor());
    // FIXME should not do these kind of things
    MVGDrawUtil::begin2DDrawing(view);
    MVGDrawUtil::drawCircle2D(MPoint(0, 0), MColor(0, 0, 0), 1, 5);
    MVGDrawUtil::end2DDrawing();

    // retrieve a nice GL state
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_LINE_STIPPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    MVGDrawUtil::begin2DDrawing(view);
    // draw potential intersection
    drawIntersection(view);
    MVGDrawUtil::end2DDrawing();

    // draw the final vertex/edge position depending on move mode
    if(_finalWSPositions.length() > 0)
    {
        switch(_onPressIntersectedComponent.type)
        {
            case MFn::kMeshVertComponent:
                MVGDrawUtil::drawLine3D(_onPressIntersectedComponent.vertex->worldPosition,
                                        _finalWSPositions[0], MColor(1, 0, 0));
                break;
            case MFn::kMeshEdgeComponent:
                MVGDrawUtil::drawLine3D(
                    _onPressIntersectedComponent.edge->vertex1->worldPosition,
                    _finalWSPositions[0], MColor(1, 0, 0));
                if(_finalWSPositions.length() > 1)
                {
                    MVGDrawUtil::drawLine3D(
                        _onPressIntersectedComponent.edge->vertex2->worldPosition,
                        _finalWSPositions[1], MColor(1, 0, 0));
                }
                break;
            default:
                break;
        }

        MVGDrawUtil::begin2DDrawing(view);
        for(size_t i = 0; i < _finalWSPositions.length(); ++i)
            MVGDrawUtil::drawCircle2D(MVGGeometryUtil::worldToViewSpace(view, _finalWSPositions[i]),
                                      MColor(0, 1, 0), 1, 30);
        MVGDrawUtil::end2DDrawing();
    }

    glDisable(GL_BLEND);
    view.endGL();
}

MStatus MVGMoveManipulator::doPress(M3dView& view)
{
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
    if(_onPressIntersectedComponent.type == MFn::kInvalid) // not moving a component
        return MPxManipulatorNode::doRelease(view);

    // compute the final vertex/edge position depending on move mode
    computeFinalWSPositions(view);

    // perform edit command : add blind data
    MVGEditCmd* cmd1 = newEditCmd();
    if(!cmd1)
        return MS::kFailure;

    MIntArray indices;
    MPointArray clickedPoints;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kMeshVertComponent:
        {
            indices.append(_onPressIntersectedComponent.vertex->index);
            clickedPoints.append(getMousePosition(view));
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            indices.append(_onPressIntersectedComponent.edge->vertex1->index);
            indices.append(_onPressIntersectedComponent.edge->vertex2->index);
            getOnMoveCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPosition,
                                  clickedPoints);
            break;
        }
        default:
            break;
    }

    assert(clickedPoints.length() == indices.length());
    cmd1->doLocate(_onPressIntersectedComponent.meshPath, clickedPoints, indices,
                   _cache->getActiveCamera().getId());
    if(cmd1->redoIt())
        cmd1->finalize();

    // perform edit command : move component
    if(_finalWSPositions.length() > 0)
    {
        MVGEditCmd* cmd2 = newEditCmd();
        if(cmd2)
        {
            assert(_finalWSPositions.length() == indices.length());
            cmd2->doMove(_onPressIntersectedComponent.meshPath, _finalWSPositions, indices);
            if(cmd2->redoIt())
                cmd2->finalize();
        }
    }

    // clear the intersected component (stored on mouse press)
    _onPressIntersectedComponent = MVGManipulatorCache::IntersectedComponent();
    _cache->rebuildMeshesCache();
    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{
    _cache->checkIntersection(10.0, getMousePosition(view));
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

    // TODO determine current mode
    _mode = kNViewTriangulation;

    // compute final vertex/edge positions
    switch(_mode)
    {
        case kNViewTriangulation:
        {
            std::vector<MVGCamera> cameras;
            switch(_onPressIntersectedComponent.type)
            {
                case MFn::kMeshVertComponent:
                {
                    MPoint triangulatedWSPoint;
                    if(triangulate(view, _onPressIntersectedComponent.vertex,
                                   getMousePosition(view), triangulatedWSPoint))
                        _finalWSPositions.append(triangulatedWSPoint);
                    break;
                }
                case MFn::kMeshEdgeComponent:
                {
                    MPoint triangulatedWSPoint;
                    bool vertex1Computed = false;
                    bool vertex2Computed = false;
                    MPointArray onMoveCSEdgePoints;
                    getOnMoveCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                          _onPressCSPosition, onMoveCSEdgePoints);
                    if(triangulate(view, _onPressIntersectedComponent.edge->vertex1,
                                   onMoveCSEdgePoints[0], triangulatedWSPoint))
                    {
                        vertex1Computed = true;
                        _finalWSPositions.append(triangulatedWSPoint);
                    }
                    if(triangulate(view, _onPressIntersectedComponent.edge->vertex2,
                                   onMoveCSEdgePoints[1], triangulatedWSPoint))
                    {
                        vertex2Computed = true;
                        _finalWSPositions.append(triangulatedWSPoint);
                    }
                    // in case we can move only one vertex
                    if(_finalWSPositions.length() == 1)
                    {
                        MVector edgeWS =
                            _onPressIntersectedComponent.edge->vertex2->worldPosition -
                            _onPressIntersectedComponent.edge->vertex1->worldPosition;
                        if(vertex1Computed)
                            _finalWSPositions.append(_finalWSPositions[0] + edgeWS);
                        if(vertex2Computed)
                            _finalWSPositions.append(_finalWSPositions[0] - edgeWS);
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
                    if(cloud.projectPolygon(view, cameraSpacePoints, worldSpacePoints))
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
                    MPointArray onMoveCSEdgePoints;
                    getOnMoveCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                          _onPressCSPosition, onMoveCSEdgePoints);
                    MPointArray cameraSpacePoints;
                    MIntArray movingVerticesIDInThisFace;
                    for(size_t i = 0; i < verticesIDs.length(); ++i)
                    {
                        // replace the moved vertex position with the current mouse position (camera
                        // space)
                        if(verticesIDs[i] == _onPressIntersectedComponent.edge->vertex1->index)
                        {

                            cameraSpacePoints.append(onMoveCSEdgePoints[0]);
                            movingVerticesIDInThisFace.append(i);
                            continue;
                        }
                        if(verticesIDs[i] == _onPressIntersectedComponent.edge->vertex2->index)
                        {
                            cameraSpacePoints.append(onMoveCSEdgePoints[1]);
                            movingVerticesIDInThisFace.append(i);
                            continue;
                        }
                        MPoint vertexWSPoint;
                        mesh.getPoint(verticesIDs[i], vertexWSPoint);
                        cameraSpacePoints.append(
                            MVGGeometryUtil::worldToCameraSpace(view, vertexWSPoint));
                    }
                    assert(movingVerticesIDInThisFace.length() == 2);
                    MPointArray worldSpacePoints;
                    MVGPointCloud cloud(MVGProject::_CLOUD);
                    if(cloud.projectPolygon(view, cameraSpacePoints, worldSpacePoints))
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
                    if(MVGGeometryUtil::projectPointOnPlane(view, getMousePosition(view),
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
                    MPointArray onMoveCSEdgePoints;
                    getOnMoveCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                          _onPressCSPosition, onMoveCSEdgePoints);
                    MVGGeometryUtil::projectPointsOnPlane(view, onMoveCSEdgePoints, faceWSPoints,
                                                          _finalWSPositions);
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

} // namespace
