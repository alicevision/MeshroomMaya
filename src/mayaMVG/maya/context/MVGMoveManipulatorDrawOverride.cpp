#include "mayaMVG/maya/context/MVGMoveManipulatorDrawOverride.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
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

MVGMoveManipulatorDrawOverride::MVGMoveManipulatorDrawOverride(const MObject& obj)
    : MHWRender::MPxDrawOverride(obj, MVGMoveManipulatorDrawOverride::draw)
{
}

MHWRender::MPxDrawOverride* MVGMoveManipulatorDrawOverride::creator(const MObject& obj)
{
    return new MVGMoveManipulatorDrawOverride(obj);
}

bool MVGMoveManipulatorDrawOverride::isBounded(const MDagPath& objPath,
                                               const MDagPath& cameraPath) const
{
    return false;
}

MBoundingBox MVGMoveManipulatorDrawOverride::boundingBox(const MDagPath& /*objPath*/,
                                                         const MDagPath& /*cameraPath*/) const
{
    return MBoundingBox();
}

MUserData*
MVGMoveManipulatorDrawOverride::prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath,
                                               const MHWRender::MFrameContext& frameContext,
                                               MUserData* oldData)
{
    MStatus status;
    MObject node = objPath.node(&status);
    CHECK(status)
    MFnDependencyNode depFn(node);
    MVGMoveManipulator* manipulator = dynamic_cast<MVGMoveManipulator*>(depFn.userNode());
    if(!manipulator)
        return oldData;

    MVGManipulatorCache* cache = manipulator->getCache();
    assert(cache != NULL);

    // retrieve data cache (create if does not exist)
    MoveDrawData* data = dynamic_cast<MoveDrawData*>(oldData);
    if(!data)
        data = new MoveDrawData();

    MDagPath activeCameraPath;
    cache->getActiveView().getCamera(activeCameraPath);

    int x, y, width, height;
    frameContext.getViewportDimensions(x, y, width, height);

    data->doDraw = (activeCameraPath == cameraPath);
    data->portWidth = width;
    data->portHeight = height;
    data->cache = cache;
    data->mouseVSPoint =
        manipulator->getMousePosition(cache->getActiveView(), MVGManipulator::kView);
    data->intersectedVSPoints.clear();
    manipulator->getIntersectedPoints(cache->getActiveView(), data->intersectedVSPoints,
                                         MVGManipulator::kView);
    return data;
}

void MVGMoveManipulatorDrawOverride::draw(const MHWRender::MDrawContext& /*context*/,
                                          const MUserData* data)
{

    // get draw data
    const MoveDrawData* userdata = dynamic_cast<const MoveDrawData*>(data);
    if(!userdata)
        return;
    if(!userdata->doDraw)
        return;

    MVGDrawUtil::begin2DDrawing(userdata->portWidth, userdata->portHeight);
    MVGMoveManipulator::drawCursor(userdata->mouseVSPoint);
    //    MVGManipulator::drawIntersection2D(userdata->intersectedVSPoints);
    //        if(MVGMoveManipulator::_mode == MVGMoveManipulator::kNViewTriangulation)
    //            MVGDrawUtil::drawTriangulation(
    //                userdata->cache->getActiveView(), userdata->onPressWSPoints,
    //                userdata->intermediateVSPositions);
    MVGDrawUtil::end2DDrawing();

    //        if(userdata->finalWSPoints.length() > 3)
    //            MVGDrawUtil::drawPolygon3D(userdata->finalWSPoints, MVGDrawUtil::_faceColor);
}

} // namespace
