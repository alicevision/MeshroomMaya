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
#include <maya/MFnPointArrayData.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MVectorArray.h>

namespace mayaMVG
{

namespace // empty namespace
{

/**
 * @brief N-View triangulation.
 *
 * @param point2dPerCamera_CS map of 2d points per camera in Camera Space
 * @param outTriangulatedPoint_WS 3D triangulated point in World Space
 */
void triangulatePoint(const std::map<int, MPoint>& point2dPerCamera_CS,
                      MPoint& outTriangulatedPoint_WS)
{
    const size_t cameraCount = point2dPerCamera_CS.size();
    assert(cameraCount > 1);
    // prepare n-view triangulation data
    openMVG::Mat2X imagePoints(2, cameraCount);

    std::vector<openMVG::Mat34> projectiveCameras;
    {
        std::map<int, MPoint>::const_iterator it = point2dPerCamera_CS.begin();
        for(size_t i = 0; it != point2dPerCamera_CS.end(); ++i, ++it)
        {
            MVGCamera camera(it->first);
            const MPoint& point2d_CS = it->second;

            // Retrieve the intrinsic matrix from 'pinholeProjectionMatrix' attribute
            //
            // K Matrix:
            // f*k_u     0      c_u
            //   0     f*k_v    c_v
            //   0       0       1
            // c_u, c_v : the principal point, which would be ideally in the centre of the image.
            //
            MDoubleArray intrinsicsArray;
            MVGMayaUtil::getDoubleArrayAttribute(camera.getDagPath().node(), "mvg_intrinsicParams",
                                                 intrinsicsArray);
            MIntArray sensorSize;
            camera.getSensorSize(sensorSize);

            // Keep ideal matrix with principal point centered
            openMVG::Mat3 K;
            K << intrinsicsArray[0], 0.0, sensorSize[0] / 2.0, 0.0, intrinsicsArray[0],
                sensorSize[1] / 2.0, 0.0, 0.0, 1.0;

            // Retrieve transformation matrix
            const MMatrix inclusiveMatrix = camera.getDagPath().inclusiveMatrix();
            const MTransformationMatrix transformMatrix(inclusiveMatrix);
            openMVG::Mat3 R;
            MMatrix rotationMatrix = transformMatrix.asRotateMatrix();
            for(int m = 0; m < 3; ++m)
            {
                for(int j = 0; j < 3; ++j)
                {
                    // Maya has inverted Y and Z axes
                    int sign = 1;
                    if(m > 0)
                        sign = -1;
                    R(m, j) = sign * rotationMatrix[m][j];
                }
            }

            // Retrieve translation vector
            const openMVG::Vec3 C = TO_VEC3(camera.getCenter());
            const openMVG::Vec3 t = -R * C;

            // Compute projection matrix
            openMVG::Mat34 P;
            openMVG::P_From_KRt(K, R, t, &P);
            projectiveCameras.push_back(P);

            // clicked point matrix (image space)
            MPoint clickedISPosition;
            MVGGeometryUtil::cameraToImageSpace(camera, point2d_CS, clickedISPosition);
            imagePoints.col(i) = openMVG::Vec2(clickedISPosition.x, clickedISPosition.y);
        }
    }

    // call n-view triangulation function
    openMVG::Vec4 result;
    openMVG::TriangulateNViewAlgebraic(imagePoints, projectiveCameras, &result);
    outTriangulatedPoint_WS.x = result(0);
    outTriangulatedPoint_WS.y = result(1);
    outTriangulatedPoint_WS.z = result(2);
    if(result(3) == 0.0)
    {
        LOG_ERROR("Triangulated point w = 0")
        return;
    }
    outTriangulatedPoint_WS = outTriangulatedPoint_WS / result(3);
}

} // empty namespace

MTypeId MVGMoveManipulator::_id(0x99222); // FIXME
MString MVGMoveManipulator::_drawDbClassification("drawdb/geometry/moveManipulator");
MString MVGMoveManipulator::_drawRegistrantID("moveManipulatorNode");
MVGMoveManipulator::EMoveMode MVGMoveManipulator::_mode = eMoveModeNViewTriangulation;

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

    // World space coordinates of edge/vertex intersected on press
    MPointArray onPressIntersectedWSPoints;
    // Camera space positions needed to draw the new element
    // - Mouse position if vertex intersection
    // - Edges points with right offset if edge intersection
    MPointArray intermediateIntersectedCSPoints;
    // Intersection tye
    MFn::Type intersectedComponentType = _cache->getIntersectionType();
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
            if(_mode != eMoveModeNViewTriangulation)
                break;
            intermediateIntersectedCSPoints.append(getMousePosition(view));
            onPressIntersectedWSPoints.append(_onPressIntersectedComponent.vertex->worldPosition);
            break;
        case MFn::kMeshVertComponent:
            intermediateIntersectedCSPoints.append(getMousePosition(view));
            onPressIntersectedWSPoints.append(_onPressIntersectedComponent.vertex->worldPosition);
            break;
        case MFn::kMeshEdgeComponent:
            getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                                        intermediateIntersectedCSPoints);
            onPressIntersectedWSPoints.append(
                _onPressIntersectedComponent.edge->vertex1->worldPosition);
            onPressIntersectedWSPoints.append(
                _onPressIntersectedComponent.edge->vertex2->worldPosition);
            break;
        default:
            break;
    }

    // Retrieve camera
    MDagPath cameraPath;
    view.getCamera(cameraPath);
    MVGCamera camera(cameraPath);
    bool isActiveView = MVGMayaUtil::isActiveView(view);
    bool isMVGView = MVGMayaUtil::isMVGView(view);

    // Draw selected point
    const MVGManipulatorCache::MVGComponent& selectedComponent = _cache->getSelectedComponent();
    if(!_doDrag)
        drawSelectedPoint3D(view, selectedComponent);

    { // 2D drawing

        MVGDrawUtil::begin2DDrawing(view.portWidth(), view.portHeight());
        MPoint mouseVSPosition = getMousePosition(view, kView);

        // Draw in active view
        if(isActiveView)
            drawCursor(mouseVSPosition);
        // Draw in MayaMVG viewports
        if(!isMVGView)
        {
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        drawPlacedPoints(view, camera, _cache, _onPressIntersectedComponent);
        // Draw selected point
        if(!_doDrag)
            drawSelectedPoint2D(view, camera, selectedComponent);
        // Draw in active MayaMVG viewport
        if(!isActiveView)
        {
            drawComplementaryIntersectedBlindData(view, camera, _cache->getIntersectedComponent());
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        // Draw vertex information on hover
        drawVertexOnHover(view, _cache, getMousePosition(view, kView));
        if(!_doDrag)
        {
            // Draw point to be placed
            drawPointToBePlaced(view, camera, selectedComponent, mouseVSPosition);
            // Draw intersection
            MPointArray intersectedVSPoints;
            getIntersectedPoints(view, intersectedVSPoints, MVGManipulator::kView);
            MVGManipulator::drawIntersection2D(intersectedVSPoints, intersectedComponentType);
        }
        // draw triangulation
        if(_mode == eMoveModeNViewTriangulation)
        {
            MPointArray triangulatedWSPoints = onPressIntersectedWSPoints;
            if(_finalWSPoints.length() > 0)
                triangulatedWSPoints = _finalWSPoints;
            MVGDrawUtil::drawTriangulatedPoints(
                view, triangulatedWSPoints,
                MVGGeometryUtil::cameraToViewSpace(view, intermediateIntersectedCSPoints));
        }
        if(_doDrag)
            MVGDrawUtil::drawLineLoop2D(_intermediateVSPoints, MVGDrawUtil::_errorColor, 3.0);

        if(_displayVisiblePoints)
            drawVisibleItems(view);
        MVGDrawUtil::end2DDrawing();
    }

    glDisable(GL_BLEND);
    view.endGL();
}

MStatus MVGMoveManipulator::doPress(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doPress(view);
    // use only the left mouse button
    if(!(QApplication::mouseButtons() & Qt::LeftButton))
        return MPxManipulatorNode::doPress(view);
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doPress(view);

    _doDrag = true;
    if(_cache->getActiveCamera().getId() != _cameraID)
    {
        _cameraID = _cache->getActiveCamera().getId();
        _cache->getActiveCamera().getVisibleItems(_visiblePointCloudItems);
    }

    // set this view as the active view
    _cache->setActiveView(view);

    // check if we intersect w/ a mesh component
    _onPressCSPoint = getMousePosition(view);
    bool triangulationMode = (_mode == eMoveModeNViewTriangulation);
    if(!_cache->checkIntersection(10.0, _onPressCSPoint, triangulationMode))
    {
        _onPressIntersectedComponent = _cache->getIntersectedComponent();
        return MPxManipulatorNode::doPress(view);
    }

    // store the intersected component
    _onPressIntersectedComponent = _cache->getIntersectedComponent();

    // Update selected component
    if(_mode == eMoveModeNViewTriangulation)
    {
        _cache->checkIntersection(10.0, getMousePosition(view), true);
        _cache->setSelectedComponent(_onPressIntersectedComponent);
    }

    // compute final positions
    computeFinalWSPoints(view);

    storeTweakInformation();

    return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
{
    _doDrag = false;
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doRelease(view);

    // Retrieve camera
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doRelease(view);

    // If there is a selected component, and if there is no blind data for the current camera
    // Use the selected component instead of _onPressIntersectedComponent to compute final positions
    const MVGManipulatorCache::MVGComponent& selectedComponent = _cache->getSelectedComponent();
    if(selectedComponent.type == MFn::kMeshVertComponent ||
       selectedComponent.type == MFn::kBlindData)
    {
        // Compute triangulated point with mouse position only if point is not already placed in 2D
        std::map<int, MPoint>::const_iterator currentData =
            selectedComponent.vertex->blindData.find(camera.getId());
        if(currentData == selectedComponent.vertex->blindData.end())
            _onPressIntersectedComponent = selectedComponent;
    }
    if(_onPressIntersectedComponent.type == MFn::kInvalid) // not moving a component
    {
        _cache->clearSelectedComponent();
        return MPxManipulatorNode::doRelease(view);
    }

    // compute the final vertex/edge position depending on move mode
    computeFinalWSPoints(view);

    // prepare commands data
    MIntArray indices;
    // clickedCSPoints contains :
    //  - mouse positions if triangulation mode,
    //  - final positions projected into camera space else
    MPointArray clickedCSPoints;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
            if(_mode != eMoveModeNViewTriangulation)
                break;
        case MFn::kMeshVertComponent:
        {
            indices.append(_onPressIntersectedComponent.vertex->index);
            if(_mode == eMoveModeNViewTriangulation)
                clickedCSPoints.append(getMousePosition(view));
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            indices.append(_onPressIntersectedComponent.edge->vertex1->index);
            indices.append(_onPressIntersectedComponent.edge->vertex2->index);
            if(_mode == eMoveModeNViewTriangulation)
                getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                            _onPressCSPoint, clickedCSPoints);
            break;
        }
        default:
            break;
    }

    if(clickedCSPoints.length() == 0)
    {
        if(_finalWSPoints.length() == 0)
        {
            // clear the intersected component (stored on mouse press)
            _onPressIntersectedComponent = MVGManipulatorCache::MVGComponent();
            return MPxManipulatorNode::doRelease(view);
        }
        clickedCSPoints = MVGGeometryUtil::worldToCameraSpace(view, _finalWSPoints);
    }

    // Retrieve tweak information
    resetTweakInformation();

    // Create command
    MVGEditCmd* cmd = newEditCmd();
    if(cmd)
    {
        bool clearBD = !(_mode == eMoveModeNViewTriangulation);
        cmd->move(_onPressIntersectedComponent.meshPath, indices, _finalWSPoints, clickedCSPoints,
                  _cache->getActiveCamera().getId(), clearBD);
        MArgList args;
        if(cmd->doIt(args))
        {
            cmd->finalize();
            _cache->rebuildMeshCache(_onPressIntersectedComponent.meshPath);
        }
    }

    // clear the intersected component (stored on mouse press)
    _onPressIntersectedComponent = MVGManipulatorCache::MVGComponent();
    _finalWSPoints.clear();

    // Select after rebuilding cache
    if(_mode == eMoveModeNViewTriangulation)
    {
        _cache->checkIntersection(10.0, getMousePosition(view), true);
        MVGManipulatorCache::MVGComponent intersectedComponent = _cache->getIntersectedComponent();
        _cache->setSelectedComponent(intersectedComponent);
    }
    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doMove(view, refresh);

    bool triangulationMode = (_mode == eMoveModeNViewTriangulation);
    _cache->checkIntersection(10.0, getMousePosition(view), triangulationMode);

    return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doDrag(view);

    bool triangulationMode = (_mode == eMoveModeNViewTriangulation);
    _cache->checkIntersection(10.0, getMousePosition(view), triangulationMode);

    // If there is a selected component, and if there is no blind data for the current camera
    // Use the selected component instead of _onPressIntersectedComponent to compute final positions
    // and have 3D preview
    const MVGManipulatorCache::MVGComponent& selectedComponent = _cache->getSelectedComponent();
    if(selectedComponent.type == MFn::kMeshVertComponent ||
       selectedComponent.type == MFn::kBlindData)
        _onPressIntersectedComponent = selectedComponent;

    // Fill verticesIDs
    MIntArray verticesID;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
            if(_mode != eMoveModeNViewTriangulation)
                break;
        case MFn::kMeshVertComponent:
            verticesID.append(_onPressIntersectedComponent.vertex->index);
            break;
        case MFn::kMeshEdgeComponent:
            verticesID.append(_onPressIntersectedComponent.edge->vertex1->index);
            verticesID.append(_onPressIntersectedComponent.edge->vertex2->index);
            break;
    }

    computeFinalWSPoints(view);

    // Set points
    if(_finalWSPoints.length() > 0)
    {
        MVGMesh mesh(_onPressIntersectedComponent.meshPath);
        mesh.setPoints(verticesID, _finalWSPoints);
    }
    else
        resetTweakInformation();

    return MPxManipulatorNode::doDrag(view);
}

void MVGMoveManipulator::computeFinalWSPoints(M3dView& view)
{
    // clear last computed positions
    _intermediateVSPoints.clear();

    // TODO in case we are intersecting a component of the same type, return this component
    // positions

    // compute final vertex/edge positions
    switch(_mode)
    {
        case eMoveModeNViewTriangulation:
            computeTriangulatedPoints(view, _finalWSPoints);
            break;
        case eMoveModePointCloudProjection:
            computePCPoints(view, _finalWSPoints);
            break;
        case eMoveModeAdjacentFaceProjection:
            computeAdjacentPoints(view, _finalWSPoints);
            break;
    }
}

/**
 * @brief Triangulate moved points.
 *
 * "Moved points" could be one vertex or 2 points of an edge.
 *
 * @param view Viewport
 * @param finalWSPoints computed points in 3D World Space coords.
 */
void MVGMoveManipulator::computeTriangulatedPoints(M3dView& view, MPointArray& finalWSPoints)
{
    finalWSPoints.clear();
    MPointArray intermediateCSPositions;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
        case MFn::kMeshVertComponent:
        {
            intermediateCSPositions.append(getMousePosition(view));
            MPoint triangulatedWSPoint;
            if(triangulate(view, _onPressIntersectedComponent.vertex, intermediateCSPositions[0],
                           triangulatedWSPoint))
                finalWSPoints.append(triangulatedWSPoint);
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            // Triangulate the 2 points of the edge
            MPoint triangulatedWSPoint;
            bool isVertex1Computed = false;
            bool isVertex2Computed = false;
            getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                                        intermediateCSPositions);
            if(triangulate(view, _onPressIntersectedComponent.edge->vertex1,
                           intermediateCSPositions[0], triangulatedWSPoint))
            {
                isVertex1Computed = true;
                finalWSPoints.append(triangulatedWSPoint);
            }
            if(triangulate(view, _onPressIntersectedComponent.edge->vertex2,
                           intermediateCSPositions[1], triangulatedWSPoint))
            {
                isVertex2Computed = true;
                finalWSPoints.append(triangulatedWSPoint);
            }
            // in case we can move only one vertex
            if(finalWSPoints.length() == 1)
            {
                MVector edgeWS = _onPressIntersectedComponent.edge->vertex2->worldPosition -
                                 _onPressIntersectedComponent.edge->vertex1->worldPosition;
                if(isVertex1Computed)
                    finalWSPoints.append(finalWSPoints[0] + edgeWS);
                if(isVertex2Computed)
                {
                    finalWSPoints.append(finalWSPoints[0]);
                    finalWSPoints[0] = finalWSPoints[1] - edgeWS;
                }
            }
            break;
        }
        default:
            break;
    }
}

/**
 * @brief Recompute the plane of the moved points to fit the point cloud.
 *
 * "Moved points" could be one vertex or 2 points of an edge.
 *
 * @param view Viewport
 * @param finalWSPoints computed points in 3D World Space coords.
 */
void MVGMoveManipulator::computePCPoints(M3dView& view, MPointArray& finalWSPoints)
{
    finalWSPoints.clear();
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
                cameraSpacePoints.append(MVGGeometryUtil::worldToCameraSpace(view, vertexWSPoint));
            }
            assert(movingVertexIDInThisFace != -1);
            MPointArray worldSpacePoints;
            MVGPointCloud cloud(MVGProject::_CLOUD);
            if(cloud.projectPoints(view, _visiblePointCloudItems, cameraSpacePoints,
                                   worldSpacePoints))
            {
                // add only the moved vertex position, not the other projected vertices
                finalWSPoints.append(worldSpacePoints[movingVertexIDInThisFace]);
            }
            else
            {
                // Save positions for error display
                _intermediateVSPoints = MVGGeometryUtil::cameraToViewSpace(view, cameraSpacePoints);
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
            getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                                        intermediateCSPositions);
            MPointArray cameraSpacePoints;
            // replace the moved edge position
            for(size_t i = 0; i < verticesIDs.length(); ++i)
            {
                if(verticesIDs[i] == _onPressIntersectedComponent.edge->vertex1->index)
                {
                    cameraSpacePoints.append(intermediateCSPositions[0]);
                    continue;
                }
                if(verticesIDs[i] == _onPressIntersectedComponent.edge->vertex2->index)
                {
                    cameraSpacePoints.append(intermediateCSPositions[1]);
                    continue;
                }
                MPoint vertexWSPoint;
                mesh.getPoint(verticesIDs[i], vertexWSPoint);
                cameraSpacePoints.append(MVGGeometryUtil::worldToCameraSpace(view, vertexWSPoint));
            }
            // Project mouse on point cloud
            MPoint projectedMouseWS;
            MVGPointCloud cloud(MVGProject::_CLOUD);
            MPointArray constraintedWSPoints;
            constraintedWSPoints.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
            constraintedWSPoints.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
            if(cloud.projectPointsWithLineConstraint(view, _visiblePointCloudItems,
                                                     cameraSpacePoints, constraintedWSPoints,
                                                     getMousePosition(view), projectedMouseWS))
            {
                MPointArray translatedWSEdgePoints;
                getTranslatedWSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                                          projectedMouseWS, translatedWSEdgePoints);
                // add only the moved vertices positions, not the other projected vertices
                finalWSPoints.append(translatedWSEdgePoints[0]);
                finalWSPoints.append(translatedWSEdgePoints[1]);
            }
            else
            {
                // Save positions for error display
                cameraSpacePoints.remove(cameraSpacePoints.length() - 1);
                _intermediateVSPoints = MVGGeometryUtil::cameraToViewSpace(view, cameraSpacePoints);
            }
            break;
        }
        default:
            break;
    }
}

/**
 * @brief Move points keeping the original plane.
 *
 * "Moved points" could be one vertex or 2 points of an edge.
 *
 * @param view Viewport
 * @param finalWSPoints computed points in 3D World Space coords.
 */
void MVGMoveManipulator::computeAdjacentPoints(M3dView& view, MPointArray& finalWSPoints)
{
    finalWSPoints.clear();
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
            PlaneKernel::Model planeModel;
            MVGGeometryUtil::computePlane(faceWSPoints, planeModel);
            MPoint projectedWSPoint;
            if(MVGGeometryUtil::projectPointOnPlane(view, intermediateCSPositions[0], planeModel,
                                                    projectedWSPoint))
                finalWSPoints.append(projectedWSPoint);
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
            MPointArray projectedWSPoints;
            // Project mouse point to compute mouseWSPoint
            intermediateCSPositions.append(getMousePosition(view));
            PlaneKernel::Model planeModel;
            MVGGeometryUtil::computePlane(faceWSPoints, planeModel);
            if(!MVGGeometryUtil::projectPointsOnPlane(view, intermediateCSPositions, planeModel,
                                                      projectedWSPoints))
                return;
            MPointArray translatedWSEdgePoints;
            getTranslatedWSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPoint,
                                      projectedWSPoints[0], translatedWSEdgePoints);
            // add only the moved vertices positions, not the other projected vertices
            finalWSPoints.append(translatedWSEdgePoints[0]);
            finalWSPoints.append(translatedWSEdgePoints[1]);
            break;
        }
        default:
            break;
    }
}

MStatus MVGMoveManipulator::storeTweakInformation()
{
    MStatus status;
    MDagPath meshPath = _onPressIntersectedComponent.meshPath;
    meshPath.extendToShape();
    MObject meshNodeShape = meshPath.node();
    MFnDependencyNode depNodeFn;
    depNodeFn.setObject(meshNodeShape);
    // Create tweak plug if it does not exist
    MPlug tweakPlug = depNodeFn.findPlug("twi");
    if(tweakPlug.isNull())
    {
        MDagModifier dagModifier;
        MFnTypedAttribute tAttr;
        MObject tweakIndicesAttr = tAttr.create("tweakIndices", "twi", MFnData::kIntArray);
        dagModifier.addAttribute(meshNodeShape, tweakIndicesAttr);
        MObject tweakVectorsAttr = tAttr.create("tweakVectors", "twv", MFnData::kVectorArray);
        dagModifier.addAttribute(meshNodeShape, tweakVectorsAttr);
        dagModifier.doIt();
    }
    // Store tweak information in new attributes
    MIntArray logicalIndices;
    MVectorArray tweakVectors;
    MPlug pntsPlug = depNodeFn.findPlug("pnts");
    MPlug tweak;
    MVector vector;
    for(int i = 0; i < pntsPlug.numElements(); ++i)
    {
        tweak = pntsPlug.elementByPhysicalIndex(i);
        if(tweak.isNull())
            continue;
        logicalIndices.append(tweak.logicalIndex());
        vector =
            MVector(tweak.child(0).asFloat(), tweak.child(1).asFloat(), tweak.child(2).asFloat());
        tweakVectors.append(vector);
    }
    status = MVGMayaUtil::setIntArrayAttribute(meshNodeShape, "twi", logicalIndices);
    CHECK_RETURN_STATUS(status)
    status = MVGMayaUtil::setVectorArrayAttribute(meshNodeShape, "twv", tweakVectors);
    CHECK_RETURN_STATUS(status)
}

MStatus MVGMoveManipulator::resetTweakInformation()
{
    if(_onPressIntersectedComponent.type == MFn::kInvalid)
        return MS::kFailure;
    // Retrieve tweak information
    MStatus status;
    MDagPath meshPath = _onPressIntersectedComponent.meshPath;
    status = meshPath.extendToShape();
    CHECK_RETURN_STATUS(status);
    MObject meshNodeShape = meshPath.node();
    MFnDependencyNode depNodeFn;
    depNodeFn.setObject(meshNodeShape);
    MIntArray logicalIndices;
    MVGMayaUtil::getIntArrayAttribute(meshNodeShape, "twi", logicalIndices);
    MVectorArray tweakVectors;
    MVGMayaUtil::getVectorArrayAttribute(meshNodeShape, "twv", tweakVectors);
    MPlug tweakPlug = depNodeFn.findPlug("pnts");
    MPlug tweak;
    assert(logicalIndices.length() == tweakVectors.length());
    // Clear and fill "pnts" attribute
    MPointArray emptyArray;
    MVGMayaUtil::setPointArrayAttribute(meshNodeShape, "pnts", emptyArray);
    for(int i = 0; i < logicalIndices.length(); i++)
    {
        tweak = tweakPlug.elementByLogicalIndex(logicalIndices[i], &status);
        CHECK_RETURN_STATUS(status);
        status = tweak.child(0).setValue(tweakVectors[i].x);
        status = tweak.child(1).setValue(tweakVectors[i].y);
        status = tweak.child(2).setValue(tweakVectors[i].z);
        CHECK_RETURN_STATUS(status);
    }
    return status;
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
        case MVGMoveManipulator::eMoveModeNViewTriangulation:
            MVGDrawUtil::drawFullCross(offsetMouseVSPosition, 5, 1, MVGDrawUtil::_triangulateColor);
            break;
        case MVGMoveManipulator::eMoveModePointCloudProjection:
            MVGDrawUtil::drawPointCloudCursorItem(offsetMouseVSPosition,
                                                  MVGDrawUtil::_pointCloudColor);
            break;
        case MVGMoveManipulator::eMoveModeAdjacentFaceProjection:
            MVGDrawUtil::drawPlaneCursorItem(offsetMouseVSPosition,
                                             MVGDrawUtil::_adjacentFaceColor);
            break;
    }
}
// static
/**
 * Draw placed points in current camera
 * @param view
 * @param cache
 * @param onPressIntersectedComponent
 */
void MVGMoveManipulator::drawPlacedPoints(
    M3dView& view, const MVGCamera& camera, MVGManipulatorCache* cache,
    const MVGManipulatorCache::MVGComponent& onPressIntersectedComponent)
{
    if(!camera.isValid())
        return;

    // browse meshes
    const std::map<std::string, MVGManipulatorCache::MeshData>& meshData = cache->getMeshData();
    std::map<std::string, MVGManipulatorCache::MeshData>::const_iterator it = meshData.begin();
    for(; it != meshData.end(); ++it)
    {
        // browse vertices
        const std::vector<MVGManipulatorCache::VertexData>& vertices = it->second.vertices;
        for(std::vector<MVGManipulatorCache::VertexData>::const_iterator verticesIt =
                vertices.begin();
            verticesIt != vertices.end(); ++verticesIt)
        {
            std::map<int, MPoint>::const_iterator currentData =
                verticesIt->blindData.find(camera.getId());
            if(currentData == verticesIt->blindData.end())
                continue;

            // Don't draw if point is currently moving //in the current view
            if(onPressIntersectedComponent.meshPath.fullPathName().asChar() == it->first)
            {
                if(onPressIntersectedComponent.type == MFn::kMeshVertComponent ||
                   (onPressIntersectedComponent.type == MFn::kBlindData &&
                    _mode == eMoveModeNViewTriangulation))
                {
                    if(onPressIntersectedComponent.vertex->index == verticesIt->index)
                        continue;
                }
                if(onPressIntersectedComponent.type == MFn::kMeshEdgeComponent)
                {
                    if(onPressIntersectedComponent.edge->vertex1->index == verticesIt->index)
                        continue;
                    if(onPressIntersectedComponent.edge->vertex2->index == verticesIt->index)
                        continue;
                }
            }

            // 2D position
            MPoint clickedVSPoint =
                MVGGeometryUtil::cameraToViewSpace(view, verticesIt->blindData.at(camera.getId()));
            MVGDrawUtil::drawFullCross(clickedVSPoint, 7, 1, MVGDrawUtil::_triangulateColor);
            // Link between 2D/3D positions
            MPoint vertexVS = MVGGeometryUtil::worldToViewSpace(view, verticesIt->worldPosition);
            MVGDrawUtil::drawLine2D(clickedVSPoint, vertexVS, MVGDrawUtil::_triangulateColor, 1.5f,
                                    1.f, true);
            // Number of placed points
            MString nbView;
            nbView += (int)(verticesIt->blindData.size());
            view.setDrawColor(MColor(0.9f, 0.3f, 0.f));
            view.drawText(nbView,
                          MVGGeometryUtil::viewToWorldSpace(view, clickedVSPoint + MPoint(5, 5)));
        }
    }
}

// static
/**
 * Hightlight blind data of intersected points in MVG views
 * @param view
 * @param MVGComponent
 */
void MVGMoveManipulator::drawComplementaryIntersectedBlindData(
    M3dView& view, const MVGCamera& camera,
    const MVGManipulatorCache::MVGComponent& intersectedComponent)
{
    if(intersectedComponent.type != MFn::kBlindData)
        return;
    if(!camera.isValid())
        return;

    const std::map<int, MPoint>::const_iterator it =
        intersectedComponent.vertex->blindData.find(camera.getId());
    if(it != intersectedComponent.vertex->blindData.end())
    {
        MPoint intersectedVSPoint = MVGGeometryUtil::cameraToViewSpace(view, it->second);
        MVGDrawUtil::drawEmptyCross(intersectedVSPoint, 8, 2, MVGDrawUtil::_intersectionColor, 1.5);
    }
}
// static
/**
 * Draw vertex information on hover :
 *      - number of views in which the point is placed
 * @param view
 * @param cache
 * @param mouseVSPosition
 */
void MVGMoveManipulator::drawVertexOnHover(M3dView& view, MVGManipulatorCache* cache,
                                           const MPoint& mouseVSPosition)
{
    MString nbView;
    const MVGManipulatorCache::MVGComponent& intersectedComponent =
        cache->getIntersectedComponent();
    if(!cache->getActiveCamera().isValid())
        return;
    const int cameraID = cache->getActiveCamera().getId();
    std::map<int, MPoint> intersectedBD;
    switch(intersectedComponent.type)
    {
        case MFn::kMeshVertComponent:
        {
            intersectedBD = intersectedComponent.vertex->blindData;
            if(intersectedBD.find(cameraID) != intersectedBD.end())
                break;
            nbView += (int)(intersectedBD.size());
            view.setDrawColor(MVGDrawUtil::_placedInOtherViewColor);
            view.drawText(
                nbView, MVGGeometryUtil::viewToWorldSpace(view, mouseVSPosition + MPoint(12, 12)));
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            intersectedBD = intersectedComponent.edge->vertex1->blindData;
            if(intersectedBD.find(cameraID) == intersectedBD.end())
            {
                nbView += (int)(intersectedBD.size());
                view.setDrawColor(MVGDrawUtil::_placedInOtherViewColor);
                view.drawText(nbView, intersectedComponent.edge->vertex1->worldPosition);
            }
            intersectedBD = intersectedComponent.edge->vertex2->blindData;
            if(intersectedBD.find(cameraID) == intersectedBD.end())
            {
                nbView.clear();
                nbView += (int)(intersectedBD.size());
                view.setDrawColor(MVGDrawUtil::_placedInOtherViewColor);
                view.drawText(nbView, intersectedComponent.edge->vertex2->worldPosition);
            }
            break;
        }
    }
}

// static
/**
 * Highlight the blind data attached to the selected point
 * @param view
 * @param selectedComponent
 */
void
MVGMoveManipulator::drawSelectedPoint2D(M3dView& view, const MVGCamera& camera,
                                        const MVGManipulatorCache::MVGComponent& selectedComponent)
{
    if(selectedComponent.type != MFn::kMeshVertComponent &&
       selectedComponent.type != MFn::kBlindData)
        return;
    std::map<int, MPoint>::const_iterator currentData =
        selectedComponent.vertex->blindData.find(camera.getId());
    if(currentData != selectedComponent.vertex->blindData.end())
    {
        MPoint blindDataVS = MVGGeometryUtil::cameraToViewSpace(view, currentData->second);
        MVGDrawUtil::drawEmptyCross(blindDataVS, 8, 2, MVGDrawUtil::_selectionColor, 1.5);
    }
}

// static
/**
 * Draw a point 3D on the world position of the selected component
 * @param view
 * @param selectedComponent
 */
void
MVGMoveManipulator::drawSelectedPoint3D(M3dView& view,
                                        const MVGManipulatorCache::MVGComponent& selectedComponent)
{
    if(selectedComponent.type != MFn::kMeshVertComponent &&
       selectedComponent.type != MFn::kBlindData)
        return;

    MVGDrawUtil::drawPoint3D(selectedComponent.vertex->worldPosition, MVGDrawUtil::_selectionColor,
                             6.f);
}

// static
/**
 * Draw a full cross at the mouseVSPosition
 * Draw a line from the mouseVSPosition to the point in 3D
 * @param view
 * @param selectedComponent
 * @param mouseVSPosition : position of the mouse in View Space coordinates
 */
void
MVGMoveManipulator::drawPointToBePlaced(M3dView& view, const MVGCamera& camera,
                                        const MVGManipulatorCache::MVGComponent& selectedComponent,
                                        const MPoint& mouseVSPosition)
{
    if(_mode != eMoveModeNViewTriangulation)
        return;
    if(selectedComponent.type != MFn::kMeshVertComponent &&
       selectedComponent.type != MFn::kBlindData)
        return;

    std::map<int, MPoint>::const_iterator currentData =
        selectedComponent.vertex->blindData.find(camera.getId());

    // Only draw if no blind data for the current view
    if(currentData != selectedComponent.vertex->blindData.end())
        return;

    MVGDrawUtil::drawFullCross(mouseVSPosition, 7, 1, MVGDrawUtil::_selectionColor);
    MPoint vertexVS =
        MVGGeometryUtil::worldToViewSpace(view, selectedComponent.vertex->worldPosition);
    MVGDrawUtil::drawLine2D(mouseVSPosition, vertexVS, MVGDrawUtil::_selectionColor, 1.5f, 1.f,
                            true);
}
} // namespace
