#include "MVGCameraPointsLocator.hpp"

#include "MVGMayaUtil.hpp"
#include "context/MVGDrawUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnNumericAttribute.h>

namespace mayaMVG
{

MTypeId MVGCameraPointsLocator::_id(0xaf27d); // FIXME
MString MVGCameraPointsLocator::classification("drawdb/geometry/cameraPointsLocator");
MString MVGCameraPointsLocator::registrantId("cameraPointsLocatorNode");

MObject MVGCameraPointsLocator::aLeftViewPoints;
MObject MVGCameraPointsLocator::aRightViewPoints;
MObject MVGCameraPointsLocator::aCommonPoints;
MObject MVGCameraPointsLocator::aLeftPointsColor;
MObject MVGCameraPointsLocator::aRightPointsColor;
MObject MVGCameraPointsLocator::aCommonPointsColor;
MObject MVGCameraPointsLocator::aDisplayMode;


MVGCameraPointsLocator::MVGCameraPointsLocator() {
}

MVGCameraPointsLocator::~MVGCameraPointsLocator() {
}


MStatus MVGCameraPointsLocator::initialize()
{
    // Point Array attributes
    MFnTypedAttribute tAttr;
    MFnNumericAttribute nAttr;
    MStatus status;
    aLeftViewPoints = tAttr.create("mvgLPanelPoints", "mvglp", MFnData::kPointArray, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aLeftViewPoints))

    aLeftPointsColor = nAttr.createColor("mvgLPointsColor", "mvglc");
    CHECK_RETURN_STATUS(status)
    nAttr.setDefault(0.0f, 0.4f, 1.0f);
    nAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aLeftPointsColor))
            
    aRightViewPoints = tAttr.create("mvgRPanelPoints", "mvgrp", MFnData::kPointArray, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aRightViewPoints))
            
    aRightPointsColor = nAttr.createColor("mvgRPointsColor", "mvgrc");
    CHECK_RETURN_STATUS(status)
    nAttr.setStorable(true);
    nAttr.setDefault(1.0f, 0.4f, 0.0f);
    CHECK_RETURN_STATUS(addAttribute(aRightPointsColor))

    aCommonPoints = tAttr.create("mvgCommonPoints", "mvgcp", MFnData::kPointArray, &status);
    CHECK_RETURN_STATUS(status)
    tAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aCommonPoints))

    aCommonPointsColor = nAttr.createColor("mvgCommonPointsColor", "mvgcc");
    CHECK_RETURN_STATUS(status)
    nAttr.setStorable(true);
    nAttr.setDefault(0.2f, 1.0f, 0.2f);
    CHECK_RETURN_STATUS(addAttribute(aCommonPointsColor))
    
    // Display mode attribute
    MFnEnumAttribute eAttr;
    aDisplayMode = eAttr.create("mvgDisplayMode", "mvgdm", MFnData::kNumeric, &status);
    eAttr.addField("None", MVGCameraPointsLocator::eDisplayModeNone);
    eAttr.addField("Both", MVGCameraPointsLocator::eDisplayModeBoth);
    eAttr.addField("Each", MVGCameraPointsLocator::eDisplayModeEach);
    eAttr.addField("Common Points Only", MVGCameraPointsLocator::eDisplayModeCommonOnly);
    eAttr.setDefault(0); // Display none by default
    CHECK_RETURN_STATUS(status)
    eAttr.setStorable(true);
    CHECK_RETURN_STATUS(addAttribute(aDisplayMode))

    return MS::kSuccess;
}

void* MVGCameraPointsLocator::creator()
{
    return new MVGCameraPointsLocator();
}

void MVGCameraPointsLocator::postConstructor()
{
}

void MVGCameraPointsLocator::getDrawData(DrawData& data) const
{
    int displayMode;
    MVGMayaUtil::getIntAttribute(thisMObject(), "mvgDisplayMode", displayMode);
    data.displayMode = EDisplayMode(displayMode);

    // Don't need to retrieve points data from plugs if we're not displaying anything
    if(displayMode == MVGCameraPointsLocator::eDisplayModeNone)
    {
        data.lPoints.clear();
        data.rPoints.clear();
        data.cPoints.clear();
    }
    else 
    {
        // Get points from attributes
        MVGMayaUtil::getPointArrayAttribute(thisMObject(), "mvgLPanelPoints", data.lPoints);
        MVGMayaUtil::getPointArrayAttribute(thisMObject(), "mvgRPanelPoints", data.rPoints);
        MVGMayaUtil::getPointArrayAttribute(thisMObject(), "mvgCommonPoints", data.cPoints);
    }

    MVGMayaUtil::getColorAttribute(thisMObject(), "mvgLPointsColor", data.lColor);
    MVGMayaUtil::getColorAttribute(thisMObject(), "mvgRPointsColor", data.rColor);
    MVGMayaUtil::getColorAttribute(thisMObject(), "mvgCommonPointsColor", data.cColor);

    // TODO: expose this as an attribute too
    data.pointSize = 2.0f;
}

void MVGCameraPointsLocator::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
                           M3dView::DisplayStatus displayStatus)
{
    // Get display mode
    int displayMode;
    MVGMayaUtil::getIntAttribute(thisMObject(), "mvgDisplayMode", displayMode);

    if(displayMode == MVGCameraPointsLocator::eDisplayModeNone)
        return;

    bool drawLeft = true, drawRight = true, drawCommon = true;
    // Adjust display mode for non MVG views
    const bool isMGVView = MVGMayaUtil::isMVGView(view);
    if(!isMGVView && displayMode == MVGCameraPointsLocator::eDisplayModeEach)
        displayMode = MVGCameraPointsLocator::eDisplayModeBoth;

    // Determine which view is being drawn
    // TODO: find a proper way to do this...
    M3dView lView;
    M3dView::getM3dViewFromModelEditor("mvgLPanel", lView);
    const bool isLeft = lView.widget() == view.widget();

    switch(displayMode)
    {
        case MVGCameraPointsLocator::eDisplayModeEach:
            drawLeft = isLeft;
            drawRight = !drawLeft;
            break;
        case MVGCameraPointsLocator::eDisplayModeCommonOnly:
            drawLeft = false;
            drawRight = false;
            break;
        case MVGCameraPointsLocator::eDisplayModeBoth:
        default:
            break;
    }
    
    DrawData data;
    getDrawData(data);
    
    view.beginGL();
    if(drawLeft)
        MVGDrawUtil::drawPoints3D(data.lPoints, data.lColor, data.pointSize);
    if(drawRight)
        MVGDrawUtil::drawPoints3D(data.rPoints, data.rColor, data.pointSize);
    if(drawCommon)
        MVGDrawUtil::drawPoints3D(data.cPoints, data.cColor, data.pointSize);
    view.endGL();
}


MUserData* MVGCameraPointsDrawOverride::prepareForDraw(
        const MDagPath& objPath,
        const MDagPath& cameraPath,
        const MHWRender::MFrameContext& frameContext,
        MUserData* oldData) {
    
    // get the node
    MStatus status;
    MFnDependencyNode node(objPath.node(), &status);
    if (!status) return NULL;
    MVGCameraPointsLocator* locatorNode = dynamic_cast<MVGCameraPointsLocator*>(node.userNode());
    if (!locatorNode) return NULL;

    // access/create user data for draw callback
    CameraPointsLocatorData* data = dynamic_cast<CameraPointsLocatorData*>(oldData);
    if (!data)
    {
        data = new CameraPointsLocatorData();
    }

    // compute data and cache it
    locatorNode->getDrawData(data->drawData);
    return data;
}

void MVGCameraPointsDrawOverride::draw(const MHWRender::MDrawContext& /*context*/,
                                            const MUserData* data)
{
    // Custom drawing is done through addUIDrawables
}


void MVGCameraPointsDrawOverride::addUIDrawables(
        const MDagPath& objPath,
        MHWRender::MUIDrawManager& drawManager,
        const MHWRender::MFrameContext& frameContext,
        const MUserData* data)
{
    const CameraPointsLocatorData* d = dynamic_cast<const CameraPointsLocatorData*>(data);
    if (!data)
            return;
    
    drawManager.beginDrawable();
    drawManager.setPointSize(d->drawData.pointSize);
    if(d->drawData.displayMode != MVGCameraPointsLocator::eDisplayModeCommonOnly)
    {
        drawManager.setColor(d->drawData.lColor);
        drawManager.points(d->drawData.lPoints, false);
        drawManager.setColor(d->drawData.rColor);
        drawManager.points(d->drawData.rPoints, false);
    }
    drawManager.setColor(d->drawData.cColor);
    drawManager.points(d->drawData.cPoints, false);
    drawManager.endDrawable();
}

}
