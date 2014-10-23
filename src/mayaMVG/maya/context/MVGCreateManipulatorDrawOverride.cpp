#include "mayaMVG/maya/context/MVGCreateManipulatorDrawOverride.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MHWGeometryUtilities.h>
#include <maya/MDrawContext.h>
#include <maya/MDataHandle.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MViewport2Renderer.h>

namespace mayaMVG
{

MVGCreateManipulatorDrawOverride::MVGCreateManipulatorDrawOverride(const MObject& obj)
    : MHWRender::MPxDrawOverride(obj, MVGCreateManipulatorDrawOverride::draw)
{
}

MHWRender::MPxDrawOverride* MVGCreateManipulatorDrawOverride::creator(const MObject& obj)
{
    return new MVGCreateManipulatorDrawOverride(obj);
}

bool MVGCreateManipulatorDrawOverride::isBounded(const MDagPath& objPath,
                                                 const MDagPath& cameraPath) const
{
    return false;
}

MBoundingBox MVGCreateManipulatorDrawOverride::boundingBox(const MDagPath& /*objPath*/,
                                                           const MDagPath& /*cameraPath*/) const
{
    return MBoundingBox();
}

MUserData* MVGCreateManipulatorDrawOverride::prepareForDraw(
    const MDagPath& objPath, const MDagPath& cameraPath,
    const MHWRender::MFrameContext& frameContext, MUserData* oldData)
{
    MStatus status;
    MObject node = objPath.node(&status);
    CHECK(status)
    MFnDependencyNode depFn(node);
    MVGCreateManipulator* manipulator = dynamic_cast<MVGCreateManipulator*>(depFn.userNode());
    if(!manipulator)
        return oldData;

    MVGManipulatorCache* cache = manipulator->getCache();
    assert(cache != NULL);

    // retrieve data cache (create if does not exist)
    CreateDrawData* data = dynamic_cast<CreateDrawData*>(oldData);
    if(!data)
        data = new CreateDrawData();

    MDagPath activeCameraPath;
    cache->getActiveView().getCamera(activeCameraPath);

    int x, y, width, height;
    frameContext.getViewportDimensions(x, y, width, height);

    data->doDraw = (activeCameraPath == cameraPath);
    data->portWidth = width;
    data->portHeight = height;
    data->mouseVSPoint =
        manipulator->getMousePosition(cache->getActiveView(), MVGManipulator::kView);
    data->cache = cache;
    data->intersectedVSPoints.clear();
    manipulator->getIntersectedPositions(cache->getActiveView(), data->intersectedVSPoints,
                                         MVGManipulator::kView);
    data->finalWSPoints = manipulator->getFinalWSPositions();
    data->clickedVSPoints = manipulator->getClickedVSPoints();
    // add mouse position
    data->clickedVSPoints.append(data->mouseVSPoint);
    return data;
}

void MVGCreateManipulatorDrawOverride::draw(const MHWRender::MDrawContext& /*context*/,
                                            const MUserData* data)
{

    // get draw data
    const CreateDrawData* userdata = dynamic_cast<const CreateDrawData*>(data);
    if(!userdata)
        return;
    if(!userdata->doDraw)
        return;

    MVGDrawUtil::begin2DDrawing(userdata->portWidth, userdata->portHeight);
    MVGCreateManipulator::drawCursor(userdata->mouseVSPoint, userdata->cache);
    MVGDrawUtil::drawClickedPoints(userdata->clickedVSPoints);
    MVGManipulator::drawIntersection2D(userdata->intersectedVSPoints);
    if(userdata->finalWSPoints.length() > 3)
        MVGDrawUtil::drawPolygon3D(userdata->finalWSPoints, MVGDrawUtil::_faceColor);
    MVGDrawUtil::end2DDrawing();
}

} // namespace
