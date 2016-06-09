#include "mayaMVG/maya/context/MVGLocatorManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/qt/MVGUserLog.hpp"
#include "mayaMVG/qt/MVGQt.hpp"
#include <maya/MArgList.h>
#include <maya/MFnTransform.h>

namespace mayaMVG
{

MTypeId MVGLocatorManipulator::_id(0x99333); // FIXME
MString MVGLocatorManipulator::_drawDbClassification("drawdb/geometry/locatorManipulator");
MString MVGLocatorManipulator::_drawRegistrantID("locatorManipulatorNode");

void* MVGLocatorManipulator::creator()
{
    return new MVGLocatorManipulator();
}

MStatus MVGLocatorManipulator::initialize()
{
    return MS::kSuccess;
}

void MVGLocatorManipulator::postConstructor()
{
    registerForMouseMove();
}

void MVGLocatorManipulator::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
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

    MPoint mouseVSPosition = getMousePosition(view, MVGManipulator::kView);
    // 2D Drawing
    {
        MVGDrawUtil::begin2DDrawing(view.portWidth(), view.portHeight());

        // draw clicked points
        MDagPath cameraPath;
        view.getCamera(cameraPath);
        MVGCamera camera(cameraPath);
        std::map<int, MPoint>::iterator it = _cameraIDToClickedCSPoint.find(camera.getId());
        if(camera.isValid() && it != _cameraIDToClickedCSPoint.end())
        {
            MPoint VSPoint;
            if(_cameraID == camera.getId() && _doDrag)
                VSPoint = mouseVSPosition;
            else
                VSPoint = MVGGeometryUtil::cameraToViewSpace(view, it->second);
            MVGDrawUtil::drawFullCross(VSPoint, 10, 1.5f, MVGDrawUtil::_triangulateColor);
        }

        // Draw in active view
        if(isActiveView)
            drawCursor(mouseVSPosition);
        MVGDrawUtil::end2DDrawing();
    }

    glDisable(GL_BLEND);
    view.endGL();
}

MStatus MVGLocatorManipulator::doPress(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doPress(view);
    // use only the left mouse button
    if(!(QApplication::mouseButtons() & Qt::LeftButton))
        return MPxManipulatorNode::doPress(view);

    _doDrag = true;
    const MVGCamera& camera = _cache->getActiveCamera();
    if(!camera.isValid())
        return MPxManipulatorNode::doPress(view);

    // set this view as the active view
    _cache->setActiveView(view);
    if(camera.getId() != _cameraID)
        _cameraID = camera.getId();

    return MPxManipulatorNode::doPress(view);
}

MStatus MVGLocatorManipulator::doRelease(M3dView& view)
{
    if(!MVGMayaUtil::isActiveView(view) || !MVGMayaUtil::isMVGView(view))
        return MPxManipulatorNode::doRelease(view);

    _doDrag = false;
    MPoint onReleaseCSPoint = getMousePosition(view);

    std::map<int, MPoint>::iterator it = _cameraIDToClickedCSPoint.find(_cameraID);
    if(it != _cameraIDToClickedCSPoint.end())
        it->second = onReleaseCSPoint;
    else
        _cameraIDToClickedCSPoint[_cameraID] = onReleaseCSPoint;

    // Update locator
    if(_cameraIDToClickedCSPoint.size() > 1)
        setLocator();

    return MPxManipulatorNode::doRelease(view);
}

MStatus MVGLocatorManipulator::doMove(M3dView& view, bool& refresh)
{
    return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGLocatorManipulator::doDrag(M3dView& view)
{
    return MPxManipulatorNode::doDrag(view);
}

void MVGLocatorManipulator::computeFinalWSPoints(M3dView& view)
{
    return;
}

// static
void MVGLocatorManipulator::drawCursor(const MPoint& originVS)
{
    MVGDrawUtil::drawArrowsCursor(originVS, MVGDrawUtil::_cursorColor);

    MPoint offsetMouseVSPosition = originVS + MPoint(10, 10);
    MVGDrawUtil::drawLocatorCursorItem(offsetMouseVSPosition);
}

void MVGLocatorManipulator::createLocator(const MString& locatorName)
{
    MStatus status;
    MDagModifier dagModifier;
    MDagPath rootPath;
    MVGMayaUtil::getDagPathByName(MVGProject::_PROJECT.c_str(), rootPath);
    MObject rootObject;
    MVGMayaUtil::getObjectByName(rootPath.partialPathName(), rootObject);

    MObject locatorTransform = dagModifier.createNode("transform", rootObject, &status);
    MFnTransform locatorTransformFn(locatorTransform);
    locatorTransformFn.setName(locatorName);
    dagModifier.createNode("MVGDummyLocator", locatorTransform, &status);
    status = dagModifier.doIt();
    CHECK(status)
}

void MVGLocatorManipulator::setLocator()
{
    MStatus status;

    // Create or retrieve locator
    const MString locatorName(MVGProject::_LOCATOR.c_str());
    MObject locatorObject;
    status = MVGMayaUtil::getObjectByName(locatorName, locatorObject);
    if(status != MStatus::kSuccess)
        createLocator(locatorName);
    MVGMayaUtil::getObjectByName(locatorName, locatorObject);

    // Compute triangulated point
    MPoint triangulatedPoint;
    MVGGeometryUtil::triangulatePoint(_cameraIDToClickedCSPoint, triangulatedPoint);
    const MVector triangulatedVector(triangulatedPoint);

    MDagPath dagPath = MDagPath::getAPathTo(locatorObject);
    // Initialize with dagpath to apply world space transformation
    MFnTransform locatorTransformFn(dagPath);
    status = locatorTransformFn.setTranslation(triangulatedVector, MSpace::kWorld);
    CHECK(status)
}
} // namespace
