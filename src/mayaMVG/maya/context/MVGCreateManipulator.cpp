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
    if(!MVGMayaUtil::isActiveView(view))
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

    { // 2D drawing
        MPoint mouseVSPositions = getMousePosition(view, kView);
        MVGDrawUtil::begin2DDrawing(view.portWidth(), view.portHeight());
        // draw cursor
        drawCursor(mouseVSPositions, _cache);
        if(!MVGMayaUtil::isMVGView(view))
        {
            MVGDrawUtil::end2DDrawing();
            glDisable(GL_BLEND);
            view.endGL();
            return;
        }
        // draw clicked points
        MPointArray _clickedVSPoints = MVGGeometryUtil::cameraToViewSpace(view, _clickedCSPoints);
        _clickedVSPoints.append(mouseVSPositions);
        MVGDrawUtil::drawClickedPoints(_clickedVSPoints);
        // draw intersection
        MPointArray intersectedVSPoints;
        getIntersectedPositions(view, intersectedVSPoints, MVGManipulator::kView);
        MVGManipulator::drawIntersection2D(intersectedVSPoints);
        MVGDrawUtil::end2DDrawing();
    }

    if(_finalWSPositions.length() > 3)
        MVGDrawUtil::drawPolygon3D(_finalWSPositions, MVGDrawUtil::_faceColor);

    glDisable(GL_BLEND);
    view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doPress(view);
    // use only the left mouse button
    if(!(QApplication::mouseButtons() & Qt::LeftButton))
        return MS::kFailure;

    // set this view as the active view
    _cache->setActiveView(view);

    // TODO clear the other views?

    // check if we are intersecting w/ a mesh component
    _onPressCSPosition = getMousePosition(view);
    _cache->checkIntersection(10.0, _onPressCSPosition);
    _onPressIntersectedComponent = _cache->getIntersectedComponent();

    return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doRelease(view);
    computeFinalWSPositions(view);

    // we are intersecting w/ a mesh component: retrieve the component properties and add its
    // coordinates to the clicked CS points array
    if(_onPressIntersectedComponent.type == MFn::kInvalid)
        _clickedCSPoints.append(getMousePosition(view));
    else
        getIntersectedPositions(view, _clickedCSPoints);

    // FIXME remove potential extra points

    if(_finalWSPositions.length() < 4)
        return MPxManipulatorNode::doRelease(view);

    // FIXME ensure the polygon is convex

    // perform edit command : create face
    MVGEditCmd* cmd = newEditCmd();
    if(!cmd)
        return MS::kFailure;

    MVGPointCloud cloud(MVGProject::_CLOUD);
    if(_onPressIntersectedComponent.type == MFn::kInvalid)
        cmd->create(MDagPath(), _finalWSPositions);
    else
        cmd->create(_onPressIntersectedComponent.meshPath, _finalWSPositions);
    MArgList args;
    if(cmd->doIt(args))
    {
        cmd->finalize();
        // FIXME should only rebuild the cache corresponding to this mesh
        _onPressIntersectedComponent = MVGManipulatorCache::IntersectedComponent();
        _cache->rebuildMeshesCache();
    }

    _clickedCSPoints.clear();
    _finalWSPositions.clear();
    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGCreateManipulator::doMove(M3dView& view, bool& refresh)
{
    _cache->checkIntersection(10.0, getMousePosition(view));
    computeFinalWSPositions(view);
    return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGCreateManipulator::doDrag(M3dView& view)
{
    // TODO : snap w/ current intersection
    computeFinalWSPositions(view);
    return MPxManipulatorNode::doDrag(view);
}

MPointArray MVGCreateManipulator::getClickedVSPoints() const
{
    return MVGGeometryUtil::cameraToViewSpace(_cache->getActiveView(), _clickedCSPoints);
}

void MVGCreateManipulator::computeFinalWSPositions(M3dView& view)
{
    _finalWSPositions.clear();

    // create polygon
    if(_clickedCSPoints.length() > 2)
    {
        // add mouse point to the clicked points
        MPointArray previewCSPoints = _clickedCSPoints;
        previewCSPoints.append(getMousePosition(view));
        // project clicked points on point cloud
        MVGPointCloud cloud(MVGProject::_CLOUD);
        cloud.projectPoints(view, previewCSPoints, _finalWSPositions);
        return;
    }
    if(_clickedCSPoints.length() > 0)
        return;

    // extrude edge
    if(_onPressIntersectedComponent.type == MFn::kMeshEdgeComponent)
    {
        MPointArray intermediateCSEdgePoints;
        getIntermediateCSEdgePoints(view, _onPressIntersectedComponent.edge, _onPressCSPosition,
                                    intermediateCSEdgePoints);
        assert(intermediateCSEdgePoints.length() == 2);

        // try to extend face in a plane computed w/ pointcloud
        {
            MPointArray cameraSpacePoints;
            cameraSpacePoints.append(MVGGeometryUtil::worldToCameraSpace(
                view, _onPressIntersectedComponent.edge->vertex1->worldPosition));
            cameraSpacePoints.append(MVGGeometryUtil::worldToCameraSpace(
                view, _onPressIntersectedComponent.edge->vertex2->worldPosition));
            cameraSpacePoints.append(intermediateCSEdgePoints[1]);
            cameraSpacePoints.append(intermediateCSEdgePoints[0]);

            MVGPointCloud cloud(MVGProject::_CLOUD);
            // we need mousePosition in world space to compute the right offset
            cameraSpacePoints.append(getMousePosition(view));
            MPointArray projectedWSPoints;
            if(cloud.projectPoints(view, cameraSpacePoints, projectedWSPoints))
            {
                MPoint mouseWSPosition = projectedWSPoints[projectedWSPoints.length() - 1];
                MPointArray translatedWSEdgePoints;
                getTranslatedWSEdgePoints(view, _onPressIntersectedComponent.edge,
                                          _onPressCSPosition, mouseWSPosition,
                                          translatedWSEdgePoints);

                _finalWSPositions.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
                _finalWSPositions.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
                _finalWSPositions.append(translatedWSEdgePoints[1]);
                _finalWSPositions.append(translatedWSEdgePoints[0]);
                return;
            }
        }

        // extrude face in the plane of the adjacent polygon
        {
            MVGMesh mesh(_onPressIntersectedComponent.meshPath);
            assert(_onPressIntersectedComponent.edge->index != -1);
            MIntArray connectedFacesIDs =
                mesh.getConnectedFacesToEdge(_onPressIntersectedComponent.edge->index);
            if(connectedFacesIDs.length() < 1)
                return;
            // TODO select the face
            MIntArray verticesIDs = mesh.getFaceVertices(connectedFacesIDs[0]);
            MPointArray faceWSPoints;
            for(size_t i = 0; i < verticesIDs.length(); ++i)
            {
                MPoint vertexWSPoint;
                mesh.getPoint(verticesIDs[i], vertexWSPoint);
                faceWSPoints.append(vertexWSPoint);
            }
            // compute moved point
            MPointArray projectedWSPoints;
            if(!MVGGeometryUtil::projectPointsOnPlane(view, intermediateCSEdgePoints, faceWSPoints,
                                                      projectedWSPoints))
                return;
            assert(projectedWSPoints.length() == 2);
            _finalWSPositions.append(_onPressIntersectedComponent.edge->vertex1->worldPosition);
            _finalWSPositions.append(_onPressIntersectedComponent.edge->vertex2->worldPosition);
            _finalWSPositions.append(projectedWSPoints[1]);
            _finalWSPositions.append(projectedWSPoints[0]);
        }
    }
}

// static
void MVGCreateManipulator::drawCursor(const MPoint& originVS, MVGManipulatorCache* cache)
{
    MVGDrawUtil::drawTargetCursor(originVS, MVGDrawUtil::_cursorColor);
    if(cache->getIntersectedComponent().type == MFn::kMeshEdgeComponent)
        MVGDrawUtil::drawExtendCursorItem(originVS + MPoint(10, 10), MVGDrawUtil::_createColor);
}
} // namespace
