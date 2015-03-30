#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/qt/MVGUserLog.hpp"
#include "mayaMVG/qt/MVGQt.hpp"
#include <maya/MArgList.h>

namespace mayaMVG
{

MTypeId MVGCreateManipulator::_id(0x99111); // FIXME
MString MVGCreateManipulator::_drawDbClassification("drawdb/geometry/createManipulator");
MString MVGCreateManipulator::_drawRegistrantID("createManipulatorNode");
bool MVGCreateManipulator::_doSnap = false;

MVGCreateManipulator::MVGCreateManipulator()
{
    _doSnap = false;
}

void* MVGCreateManipulator::creator()
{
    return new MVGCreateManipulator();
}

MStatus MVGCreateManipulator::initialize()
{
    return MS::kSuccess;
}

void MVGCreateManipulator::postConstructor()
{
    registerForMouseMove();
}

void MVGCreateManipulator::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
                                M3dView::DisplayStatus dispStatus)
{

    if(!MVGMayaUtil::isMVGView(view))
        return;
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

    bool isActiveView = MVGMayaUtil::isActiveView(view);
    bool isMVGView = MVGMayaUtil::isMVGView(view);
    // Draw wireframe face
    // TODO : draw alpha poly
    if(isActiveView && isMVGView)
    {
        if(_cameraIDToClickedCSPoints.second.length() == 0 && _finalWSPoints.length() > 3)
            MVGDrawUtil::drawLineLoop3D(_finalWSPoints, MVGDrawUtil::_okayColor, 3.0);
    }

    { // 2D drawing
        MPoint mouseVSPositions = getMousePosition(view, kView);
        MVGDrawUtil::begin2DDrawing(view.portWidth(), view.portHeight());
        // draw clicked points
        MDagPath cameraPath;
        view.getCamera(cameraPath);
        MVGCamera camera(cameraPath);
        if(camera.isValid() && _cameraIDToClickedCSPoints.first == camera.getId())
        {
            MColor drawColor = MVGDrawUtil::_errorColor;
            MPointArray _clickedVSPoints =
                MVGGeometryUtil::cameraToViewSpace(view, _cameraIDToClickedCSPoints.second);
            const MVGCamera& activeCamera = _cache->getActiveCamera();
            if(activeCamera.isValid() && _cameraIDToClickedCSPoints.first == activeCamera.getId())
            {
                _clickedVSPoints.append(mouseVSPositions);
                if(_finalWSPoints.length() == 4)
                    drawColor = MVGDrawUtil::_okayColor;
            }
            MVGDrawUtil::drawClickedPoints(_clickedVSPoints, drawColor);
        }
        if(!isActiveView)
        {
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        // draw cursor
        drawCursor(mouseVSPositions, _cache);
        if(!isMVGView)
        {
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        // draw intersection
        MPointArray intersectedVSPoints;
        getIntersectedPoints(view, intersectedVSPoints, MVGManipulator::kView);
        MVGManipulator::drawIntersection2D(intersectedVSPoints, _cache->getIntersectionType());

        // Draw snaped element
        if(_doSnap)
        {
            if(_snapedPoints.length() == 1)
                MVGDrawUtil::drawCircle2D(
                    MVGGeometryUtil::worldToViewSpace(view, _finalWSPoints[_snapedPoints[0]]),
                    MVGDrawUtil::_intersectionColor, 5, 30);
            else if(_snapedPoints.length() == 2)
                MVGDrawUtil::drawLine2D(
                    MVGGeometryUtil::worldToViewSpace(view, _finalWSPoints[_snapedPoints[0]]),
                    MVGGeometryUtil::worldToViewSpace(view, _finalWSPoints[_snapedPoints[1]]),
                    MVGDrawUtil::_intersectionColor, 3.0);
        }

        MVGDrawUtil::end2DDrawing();
    }
    glDisable(GL_BLEND);
    view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doPress(view);
    // use only the left mouse button
    if(!(QApplication::mouseButtons() & Qt::LeftButton))
        return MPxManipulatorNode::doPress(view);
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doPress(view);
    if(camera.getId() != _cameraIDToClickedCSPoints.first)
    {
        _cameraIDToClickedCSPoints.first = camera.getId();
        _cameraIDToClickedCSPoints.second.clear();
    }
    if(_cache->getActiveCamera().getId() != _cameraID)
    {
        _cameraID = _cache->getActiveCamera().getId();
        _cache->getActiveCamera().getVisibleItems(_visiblePointCloudItems);
    }
    // set this view as the active view
    _cache->setActiveView(view);

    // TODO clear the other views?

    // check if we are intersecting w/ a mesh component
    _onPressCSPoint = getMousePosition(view);
    _cache->checkIntersection(10.0, _onPressCSPoint);
    _onPressIntersectedComponent = _cache->getIntersectedComponent();

    return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doRelease(view);

    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doRelease(view);

    computeFinalWSPoints(view);

    // we are intersecting w/ a mesh component: retrieve the component properties and add its
    // coordinates to the clicked CS points array
    if(_onPressIntersectedComponent.type == MFn::kInvalid)
        _cameraIDToClickedCSPoints.second.append(getMousePosition(view));
    else
        getIntersectedPoints(view, _cameraIDToClickedCSPoints.second);

    // FIXME remove potential extra points

    // If we did not found a plane, remove last clicked point
    if(_finalWSPoints.length() < 4)
    {
        if(_cameraIDToClickedCSPoints.second.length() == 4)
            _cameraIDToClickedCSPoints.second.remove(_cameraIDToClickedCSPoints.second.length() -
                                                     1);
        // Clear component
        _onPressIntersectedComponent = MVGManipulatorCache::MVGComponent();
        return MPxManipulatorNode::doRelease(view);
    }

    // FIXME ensure the polygon is convex

    // TODO : Undo/Redo for mesh creation
    // mesh creation
    if(_onPressIntersectedComponent.type == MFn::kInvalid)
    {
        MVGMesh mesh = MVGMesh::create(MVGProject::_MESH);
        int polygonID;
        mesh.addPolygon(_finalWSPoints, polygonID);
        mesh.setIsActive(true);

        _cache->rebuildMeshesCache();
        _cameraIDToClickedCSPoints.second.clear();
        _finalWSPoints.clear();
        return MPxManipulatorNode::doRelease(view);
    }
    // mesh edit
    MVGEditCmd* cmd = newEditCmd();
    if(!cmd)
        return MS::kFailure;
    MPointArray edgeCSPositions;
    edgeCSPositions.append(MVGGeometryUtil::worldToCameraSpace(view, _finalWSPoints[2]));
    edgeCSPositions.append(MVGGeometryUtil::worldToCameraSpace(view, _finalWSPoints[3]));
    cmd->addFace(_onPressIntersectedComponent.meshPath, _finalWSPoints, edgeCSPositions,
                 _cache->getActiveCamera().getId());

    MArgList args;
    if(cmd->doIt(args))
    {
        cmd->finalize();
        _cache->rebuildMeshCache(_onPressIntersectedComponent.meshPath);
    }

    // Clear data
    _onPressIntersectedComponent = MVGManipulatorCache::MVGComponent();
    _cameraIDToClickedCSPoints.second.clear();
    _finalWSPoints.clear();

    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGCreateManipulator::doMove(M3dView& view, bool& refresh)
{
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doMove(view, refresh);

    _cache->checkIntersection(10.0, getMousePosition(view));
    computeFinalWSPoints(view);
    return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGCreateManipulator::doDrag(M3dView& view)
{
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doDrag(view);

    // TODO : snap w/ current intersection
    _cache->checkIntersection(10.0, getMousePosition(view));
    computeFinalWSPoints(view);
    return MPxManipulatorNode::doDrag(view);
}

MPointArray MVGCreateManipulator::getClickedVSPoints() const
{
    return MVGGeometryUtil::cameraToViewSpace(_cache->getActiveView(),
                                              _cameraIDToClickedCSPoints.second);
}

void MVGCreateManipulator::computeFinalWSPoints(M3dView& view)
{
    _snapedPoints.clear();

    // create polygon
    if(_cameraIDToClickedCSPoints.second.length() > 2)
    {
        _finalWSPoints.clear();
        // add mouse point to the clicked points
        MPointArray previewCSPoints = _cameraIDToClickedCSPoints.second;
        previewCSPoints.append(getMousePosition(view));
        // project clicked points on point cloud
        MVGPointCloud cloud(MVGProject::_CLOUD);
        cloud.projectPoints(view, _visiblePointCloudItems, previewCSPoints, _finalWSPoints);
        return;
    }
    if(_cameraIDToClickedCSPoints.second.length() > 0)
        return;

    // extrude edge
    if(_onPressIntersectedComponent.type != MFn::kMeshEdgeComponent)
        return;

    _cache->checkIntersection(10.0, getMousePosition(view, kCamera));
    const MVGManipulatorCache::MVGComponent& mouseIntersectedComponent =
        _cache->getIntersectedComponent();

    // Retrieve edge points preserving on press edge length
    MPointArray intermediateCSEdgePoints;
    getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                                intermediateCSEdgePoints);
    assert(intermediateCSEdgePoints.length() == 2);
    if(_doSnap)
    {
        // Snap to intersected edge
        if(snapToIntersectedEdge(view, _finalWSPoints, mouseIntersectedComponent))
            return;
        // Snap to intersected vertex
        if(snapToIntersectedVertex(view, _finalWSPoints, intermediateCSEdgePoints))
            return;
    }
    // try to extend face in a plane computed w/ pointcloud
    if(computePCPoints(view, _finalWSPoints, intermediateCSEdgePoints))
        return;

    // extrude face in the plane of the adjacent polygon
    computeAdjacentPoints(view, _finalWSPoints, intermediateCSEdgePoints);
}

bool MVGCreateManipulator::computePCPoints(M3dView& view, MPointArray& finalWSPoints,
                                           const MPointArray& intermediateCSEdgePoints)
{
    finalWSPoints.clear();
    // Get camera space points to project
    MPointArray cameraSpacePoints;
    cameraSpacePoints.append(MVGGeometryUtil::worldToCameraSpace(
        view, _onPressIntersectedComponent.edge->vertex1->worldPosition));
    cameraSpacePoints.append(MVGGeometryUtil::worldToCameraSpace(
        view, _onPressIntersectedComponent.edge->vertex2->worldPosition));
    cameraSpacePoints.append(intermediateCSEdgePoints[1]);
    cameraSpacePoints.append(intermediateCSEdgePoints[0]);

    // Project mouse on point cloud
    MPoint projectedMouseWS;
    MVGPointCloud cloud(MVGProject::_CLOUD);
    MPointArray constraintedPoints;
    constraintedPoints.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
    constraintedPoints.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
    if(!cloud.projectPointsWithLineConstraint(view, _visiblePointCloudItems, cameraSpacePoints,
                                              constraintedPoints, getMousePosition(view),
                                              projectedMouseWS))
        return false;
    MPointArray translatedWSEdgePoints;
    getTranslatedWSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                              projectedMouseWS, translatedWSEdgePoints);
    // Begin with second edge's vertex to keep normal
    finalWSPoints.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
    finalWSPoints.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
    finalWSPoints.append(translatedWSEdgePoints[0]);
    finalWSPoints.append(translatedWSEdgePoints[1]);

    return true;
}

bool MVGCreateManipulator::computeAdjacentPoints(M3dView& view, MPointArray& finalWSPoints,
                                                 const MPointArray& intermediateCSEdgePoints)
{
    finalWSPoints.clear();
    MVGMesh mesh(_onPressIntersectedComponent.meshPath);
    assert(_onPressIntersectedComponent.edge->index != -1);
    MIntArray connectedFacesIDs =
        mesh.getConnectedFacesToEdge(_onPressIntersectedComponent.edge->index);
    if(connectedFacesIDs.length() < 1)
        return false;
    // TODO select the face
    MIntArray verticesIDs = mesh.getFaceVertices(connectedFacesIDs[0]);
    MPointArray faceWSPoints;
    for(size_t i = 0; i < verticesIDs.length(); ++i)
    {
        MPoint vertexWSPoint;
        mesh.getPoint(verticesIDs[i], vertexWSPoint);
        faceWSPoints.append(vertexWSPoint);
    }
    // Compute plane model
    PlaneKernel::Model planeModel;
    if(!MVGGeometryUtil::computePlane(faceWSPoints, planeModel))
        return false;
    // Project moves points on plane
    MPointArray projectedWSPoints;
    if(!MVGGeometryUtil::projectPointsOnPlane(view, intermediateCSEdgePoints, planeModel,
                                              projectedWSPoints))
        return false;
    assert(projectedWSPoints.length() == 2);
    // Begin with second edge's vertex to keep normal
    finalWSPoints.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
    finalWSPoints.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
    finalWSPoints.append(projectedWSPoints[0]);
    finalWSPoints.append(projectedWSPoints[1]);

    return true;
}

bool MVGCreateManipulator::snapToIntersectedEdge(
    M3dView& view, MPointArray& finalWSPoints,
    const MVGManipulatorCache::MVGComponent& intersectedEdge)
{
    if(intersectedEdge.type != MFn::kMeshEdgeComponent)
        return false;
    if(_onPressIntersectedComponent.type != MFn::kMeshEdgeComponent)
        return false;
    finalWSPoints.clear();
    const MPoint& pressedVertex1 = _onPressIntersectedComponent.edge->vertex1->worldPosition;
    const MPoint& pressedVertex2 = _onPressIntersectedComponent.edge->vertex2->worldPosition;
    const MPoint& intersectedVertex1 = intersectedEdge.edge->vertex1->worldPosition;
    const MPoint& intersectedVertex2 = intersectedEdge.edge->vertex2->worldPosition;

    // Don't snap on adjacent edge
    if(pressedVertex1 == intersectedVertex1 || pressedVertex1 == intersectedVertex2 ||
       pressedVertex2 == intersectedVertex1 || pressedVertex2 == intersectedVertex2)
        return false;

    // Begin with second edge's vertex to keep normal
    finalWSPoints.append(pressedVertex2);
    finalWSPoints.append(pressedVertex1);
    finalWSPoints.append(intersectedVertex2);
    finalWSPoints.append(intersectedVertex1);

    // Check points order
    MPoint A = MVGGeometryUtil::worldToCameraSpace(view, finalWSPoints[0]);
    MPoint B = MVGGeometryUtil::worldToCameraSpace(view, finalWSPoints[1]);
    MVector AD = MVGGeometryUtil::worldToCameraSpace(view, finalWSPoints[3]) - A;
    MVector BC = MVGGeometryUtil::worldToCameraSpace(view, finalWSPoints[2]) - B;
    if(MVGGeometryUtil::doEdgesIntersect(A, B, AD, BC))
    {
        MPointArray tmp = finalWSPoints;
        finalWSPoints[3] = tmp[2];
        finalWSPoints[2] = tmp[3];
    }
    return true;
}

bool MVGCreateManipulator::snapToIntersectedVertex(M3dView& view, MPointArray& finalWSPoints,
                                                   const MPointArray& intermediateCSEdgePoints)
{
    if(_onPressIntersectedComponent.type != MFn::kMeshEdgeComponent)
        return false;
    finalWSPoints.setLength(4);
    // Begin with second edge's vertex to keep normal
    finalWSPoints[0] = _onPressIntersectedComponent.edge->vertex2->worldPosition;
    finalWSPoints[1] = _onPressIntersectedComponent.edge->vertex1->worldPosition;

    // Get intersection with "intermediateCSEdgePoints"
    MVGManipulatorCache::MVGComponent edgeIntersectedComponent;
    for(int i = 0; i < intermediateCSEdgePoints.length(); ++i)
    {
        if(_cache->checkIntersection(10.0, intermediateCSEdgePoints[i]))
        {
            edgeIntersectedComponent = _cache->getIntersectedComponent();
            if(edgeIntersectedComponent.type == MFn::kMeshVertComponent)
            {
                _finalWSPoints[i + 2] = edgeIntersectedComponent.vertex->worldPosition;
                _snapedPoints.append(i + 2);
            }
        }
    }
    // No snaped point
    if(_snapedPoints.length() == 0)
        return false;

    // If only one vertex to snap, we need to apply the vector of the
    // extended egde to compute the last point.
    if(_snapedPoints.length() == 1)
    {
        MVector onPressEdgeVector = _onPressIntersectedComponent.edge->vertex2->worldPosition -
                                    _onPressIntersectedComponent.edge->vertex1->worldPosition;
        if(_snapedPoints[0] == 2)
            _finalWSPoints[3] = _finalWSPoints[2] + onPressEdgeVector;
        if(_snapedPoints[0] == 3)
            _finalWSPoints[2] = _finalWSPoints[3] - onPressEdgeVector;
    }

    return true;
}

// static
void MVGCreateManipulator::drawCursor(const MPoint& originVS, MVGManipulatorCache* cache)
{
    MVGDrawUtil::drawTargetCursor(originVS, MVGDrawUtil::_cursorColor);
    if(cache->getIntersectedComponent().type == MFn::kMeshEdgeComponent)
        MVGDrawUtil::drawExtendCursorItem(originVS + MPoint(10, 10), MVGDrawUtil::_createColor);
}
} // namespace
