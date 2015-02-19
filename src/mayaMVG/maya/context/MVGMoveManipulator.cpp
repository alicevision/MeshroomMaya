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
    size_t cameraCount = point2dPerCamera_CS.size();
    assert(cameraCount > 1);
    // prepare n-view triangulation data
    openMVG::Mat2X imagePoints(2, cameraCount);
    std::vector<openMVG::Mat34> projectiveCameras;

    // Retrieve transformation matrix of the Maya full DAG.
    MVGPointCloud cloud(MVGProject::_CLOUD);
    MMatrix mayaInclusiveMatrix = MMatrix::identity;
    if(cloud.isValid() && cloud.getDagPath().isValid())
        mayaInclusiveMatrix = cloud.getDagPath().inclusiveMatrix();
    MTransformationMatrix transformMatrix(mayaInclusiveMatrix);
    MMatrix rotationMatrix = transformMatrix.asRotateMatrix();
    openMVG::Mat3 rotation;
    for(int i = 0; i < 3; ++i)
    {
        for(int j = 0; j < 3; ++j)
            rotation(i, j) = rotationMatrix[i][j];
    }

    {
        std::map<int, MPoint>::const_iterator it = point2dPerCamera_CS.begin();
        for(size_t i = 0; it != point2dPerCamera_CS.end(); ++i, ++it)
        {
            MVGCamera camera(it->first);
            const MPoint& point2d_CS = it->second;

            // Retrieve the intrinsic matrix.
            //
            // K Matrix:
            // f*k_u     0      c_u
            //   0     f*k_v    c_v
            //   0       0       1
            // c_u, c_v : the principal point, which would be ideally in the centre of the image.
            //
            // In maya we keep the camera with an ideal optical center at the center
            // of the camera sensor: c_u = width/2 and c_v = height/2
            // We compensate this offset on the image plane.
            // So here, for the triangulation, we should use the ideal K.
            openMVG::Mat3 K = camera.getPinholeCamera()._K;
            std::pair<double, double> imageSize = camera.getImageSize();
            K(0, 2) = imageSize.first * 0.5;
            K(1, 2) = imageSize.second * 0.5;

            // Recompute the full Projection Matric P with the maya parent
            // transformations applied.
            MPoint cameraCenter = TO_MPOINT(camera.getPinholeCamera()._C);
            cameraCenter *= mayaInclusiveMatrix;
            const openMVG::Mat3 R = camera.getPinholeCamera()._R;
            openMVG::Vec3 C = TO_VEC3(cameraCenter);
            const openMVG::Vec3 t = -R * rotation * C;
            openMVG::Mat34 P;
            openMVG::P_From_KRt(K, R * rotation, t, &P);
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

    // World space coordinates of edge/vertex intersected on press
    MPointArray onPressIntersectedWSPoints;
    // Camera space positions needed to draw the new element
    // - Mouse position if vertex intersection
    // - Edges points with right offset if edge intersection
    MPointArray intermediateIntersectedCSPoints;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
            if(_mode != kNViewTriangulation)
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
        drawPlacedPoints(view, _cache, _onPressIntersectedComponent);
        MFn::Type intersectedComponentType = _cache->getIntersectiontType();
        // Draw in active MayaMVG viewport
        if(!MVGMayaUtil::isActiveView(view))
        {
            drawComplementaryIntersectedBlindData(view, _cache->getIntersectedComponent());
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        // Draw vertex information on hover
        drawVertexOnHover(view, _cache, getMousePosition(view, kView));
        // draw intersection
        if(!_doDrag)
        {
            MPointArray intersectedVSPoints;
            getIntersectedPoints(view, intersectedVSPoints, MVGManipulator::kView);
            MVGManipulator::drawIntersection2D(intersectedVSPoints, intersectedComponentType);
        }
        // draw triangulation
        if(_mode == kNViewTriangulation)
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
    
    if(_cache->getActiveCamera().getId() != _cameraID)
    {
        _cameraID = _cache->getActiveCamera().getId();
        _cache->getActiveCamera().getVisibleItems(_visiblePointCloudItems);
    }

    // set this view as the active view
    _cache->setActiveView(view);

    // check if we intersect w/ a mesh component
    _onPressCSPoint = getMousePosition(view);
    bool triangulationMode = (_mode == kNViewTriangulation);
    if(!_cache->checkIntersection(10.0, _onPressCSPoint, triangulationMode))
    {
        _onPressIntersectedComponent = _cache->getIntersectedComponent();
        return MPxManipulatorNode::doPress(view);
    }

    // store the intersected component
    _onPressIntersectedComponent = _cache->getIntersectedComponent();

    // compute final positions
    computeFinalWSPositions(view);

    storeTweakInformation();

    _doDrag = true;
    return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
{
    _doDrag = false;
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doRelease(view);

    if(_onPressIntersectedComponent.type == MFn::kInvalid) // not moving a component
        return MPxManipulatorNode::doRelease(view);

    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doRelease(view);
    // compute the final vertex/edge position depending on move mode
    computeFinalWSPositions(view);

    // prepare commands data
    MIntArray indices;
    // clickedCSPoints contains :
    //  - mouse positions if triangulation mode,
    //  - final positions projected into camera space else
    MPointArray clickedCSPoints;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
            if(_mode != kNViewTriangulation)
                break;
        case MFn::kMeshVertComponent:
        {
            indices.append(_onPressIntersectedComponent.vertex->index);
            if(_mode == kNViewTriangulation)
                clickedCSPoints.append(getMousePosition(view));
            break;
        }
        case MFn::kMeshEdgeComponent:
        {
            indices.append(_onPressIntersectedComponent.edge->vertex1->index);
            indices.append(_onPressIntersectedComponent.edge->vertex2->index);
            if(_mode == kNViewTriangulation)
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
            return MPxManipulatorNode::doRelease(view);
        clickedCSPoints = MVGGeometryUtil::worldToCameraSpace(view, _finalWSPoints);
    }

    // Retrieve tweak information
    resetTweakInformation();

    // Create command
    MVGEditCmd* cmd = newEditCmd();
    if(cmd)
    {
        bool clearBD = !(_mode == kNViewTriangulation);
        cmd->move(_onPressIntersectedComponent.meshPath, indices, _finalWSPoints, clickedCSPoints,
                  _cache->getActiveCamera().getId(), clearBD);
        MArgList args;
        if(cmd->doIt(args))
            cmd->finalize();
    }

    // clear the intersected component (stored on mouse press)
    _onPressIntersectedComponent = MVGManipulatorCache::IntersectedComponent();
    _finalWSPoints.clear();
    _cache->rebuildMeshesCache();

    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doMove(view, refresh);

    bool triangulationMode = (_mode == kNViewTriangulation);
    _cache->checkIntersection(10.0, getMousePosition(view), triangulationMode);
    return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doDrag(view);

    bool triangulationMode = (_mode == kNViewTriangulation);
    _cache->checkIntersection(10.0, getMousePosition(view), triangulationMode);
    // Fill verticesIDs
    MIntArray verticesID;
    switch(_onPressIntersectedComponent.type)
    {
        case MFn::kBlindData:
            if(_mode != kNViewTriangulation)
                break;
        case MFn::kMeshVertComponent:
            verticesID.append(_onPressIntersectedComponent.vertex->index);
            break;
        case MFn::kMeshEdgeComponent:
            verticesID.append(_onPressIntersectedComponent.edge->vertex1->index);
            verticesID.append(_onPressIntersectedComponent.edge->vertex2->index);
            break;
    }

    computeFinalWSPositions(view);

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

void MVGMoveManipulator::computeFinalWSPositions(M3dView& view)
{
    // clear last computed positions
    _finalWSPoints.clear();
    _intermediateVSPoints.clear();

    // TODO in case we are intersecting a component of the same type, return this component
    // positions

    // compute final vertex/edge positions
    switch(_mode)
    {
        case kNViewTriangulation:
        {
            switch(_onPressIntersectedComponent.type)
            {
                case MFn::kBlindData:
                case MFn::kMeshVertComponent:
                {
                    MPointArray intermediateCSPositions;
                    intermediateCSPositions.append(getMousePosition(view));
                    MPoint triangulatedWSPoint;
                    if(triangulate(view, _onPressIntersectedComponent.vertex,
                                   intermediateCSPositions[0], triangulatedWSPoint))
                        _finalWSPoints.append(triangulatedWSPoint);
                    break;
                }
                case MFn::kMeshEdgeComponent:
                {
                    MPoint triangulatedWSPoint;
                    bool vertex1Computed = false;
                    bool vertex2Computed = false;
                    MPointArray intermediateCSPositions;
                    getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge,
                                                _onPressCSPoint, intermediateCSPositions);
                    if(triangulate(view, _onPressIntersectedComponent.edge->vertex1,
                                   intermediateCSPositions[0], triangulatedWSPoint))
                    {
                        vertex1Computed = true;
                        _finalWSPoints.append(triangulatedWSPoint);
                    }
                    if(triangulate(view, _onPressIntersectedComponent.edge->vertex2,
                                   intermediateCSPositions[1], triangulatedWSPoint))
                    {
                        vertex2Computed = true;
                        _finalWSPoints.append(triangulatedWSPoint);
                    }
                    // in case we can move only one vertex
                    if(_finalWSPoints.length() == 1)
                    {
                        MVector edgeWS = _onPressIntersectedComponent.edge->vertex2->worldPosition -
                                         _onPressIntersectedComponent.edge->vertex1->worldPosition;
                        if(vertex1Computed)
                            _finalWSPoints.append(_finalWSPoints[0] + edgeWS);
                        if(vertex2Computed)
                        {
                            _finalWSPoints.append(_finalWSPoints[0]);
                            _finalWSPoints[0] = _finalWSPoints[1] - edgeWS;
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
                    if(cloud.projectPoints(view, _visiblePointCloudItems, cameraSpacePoints,
                                           worldSpacePoints))
                    {
                        // add only the moved vertex position, not the other projected vertices
                        _finalWSPoints.append(worldSpacePoints[movingVertexIDInThisFace]);
                    }
                    else
                    {
                        // Save positions for error display
                        _intermediateVSPoints =
                            MVGGeometryUtil::cameraToViewSpace(view, cameraSpacePoints);
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
                                                _onPressCSPoint, intermediateCSPositions);
                    MPointArray cameraSpacePoints;
                    for(size_t i = 0; i < verticesIDs.length(); ++i)
                    {
                        // replace the moved vertex position with the current mouse position (camera
                        // space)
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
                        cameraSpacePoints.append(
                            MVGGeometryUtil::worldToCameraSpace(view, vertexWSPoint));
                    }
                    // we need mousePosition in world space to compute the right offset
                    cameraSpacePoints.append(getMousePosition(view));
                    MPointArray worldSpacePoints;
                    MVGPointCloud cloud(MVGProject::_CLOUD);
                    if(cloud.projectPoints(view, _visiblePointCloudItems, cameraSpacePoints,
                                           worldSpacePoints, cameraSpacePoints.length() - 1))
                    {
                        MPointArray translatedWSEdgePoints;
                        getTranslatedWSEdgePoints(view, _onPressIntersectedComponent.edge,
                                                  _onPressCSPoint, worldSpacePoints[0],
                                                  translatedWSEdgePoints);
                        // add only the moved vertices positions, not the other projected vertices
                        _finalWSPoints.append(translatedWSEdgePoints[0]);
                        _finalWSPoints.append(translatedWSEdgePoints[1]);
                    }
                    else
                    {
                        // Save positions for error display
                        cameraSpacePoints.remove(cameraSpacePoints.length() - 1);
                        _intermediateVSPoints =
                            MVGGeometryUtil::cameraToViewSpace(view, cameraSpacePoints);
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
                        _finalWSPoints.append(projectedWSPoint);
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
                                                _onPressCSPoint, intermediateCSPositions);
                    MVGGeometryUtil::projectPointsOnPlane(view, intermediateCSPositions,
                                                          faceWSPoints, _finalWSPoints);
                    break;
                }
                default:
                    break;
            }
            break;
        }
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
void MVGMoveManipulator::drawPlacedPoints(
    M3dView& view, MVGManipulatorCache* cache,
    const MVGManipulatorCache::IntersectedComponent& onPressIntersectedComponent)
{
    MDagPath cameraPath;
    view.getCamera(cameraPath);
    MVGCamera camera(cameraPath);
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

            // Don't draw if point is currently moving
            if(onPressIntersectedComponent.type == MFn::kMeshVertComponent ||
               (onPressIntersectedComponent.type == MFn::kBlindData &&
                _mode == kNViewTriangulation))
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
void MVGMoveManipulator::drawComplementaryIntersectedBlindData(
    M3dView& view, const MVGManipulatorCache::IntersectedComponent& intersectedComponent)
{
    if(intersectedComponent.type != MFn::kBlindData)
        return;
    MDagPath cameraPath;
    view.getCamera(cameraPath);
    MVGCamera camera(cameraPath);
    if(!camera.isValid())
        return;

    const std::map<int, MPoint>::const_iterator it =
        intersectedComponent.vertex->blindData.find(camera.getId());
    if(it != intersectedComponent.vertex->blindData.end())
    {
        MPoint intersectedVSPoint = MVGGeometryUtil::cameraToViewSpace(view, it->second);
        MVGDrawUtil::drawEmptyCross(intersectedVSPoint, 7, 2, MVGDrawUtil::_intersectionColor, 1.5);
    }
}
// static
void MVGMoveManipulator::drawVertexOnHover(M3dView& view, MVGManipulatorCache* cache,
                                           const MPoint& mouseVSPosition)
{
    MString nbView;
    const MVGManipulatorCache::IntersectedComponent& intersectedComponent =
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
            view.setDrawColor(MColor(0.9f, 0.3f, 0.f));
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
                view.setDrawColor(MColor(0.9f, 0.3f, 0.f));
                view.drawText(nbView, intersectedComponent.edge->vertex1->worldPosition);
            }
            intersectedBD = intersectedComponent.edge->vertex2->blindData;
            if(intersectedBD.find(cameraID) == intersectedBD.end())
            {
                nbView.clear();
                nbView += (int)(intersectedBD.size());
                view.setDrawColor(MColor(0.9f, 0.3f, 0.f));
                view.drawText(nbView, intersectedComponent.edge->vertex2->worldPosition);
            }
            break;
        }
    }
}
} // namespace
