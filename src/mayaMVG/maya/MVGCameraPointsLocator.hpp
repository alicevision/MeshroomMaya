#pragma once

#include <maya/MPxLocatorNode.h>
#include <maya/MTypeId.h>
#include <maya/MPxDrawOverride.h>
#include <maya/MUIDrawManager.h>
#include <maya/MFrameContext.h>
#include <maya/MPointArray.h>
#include <maya/MUserData.h>

namespace mayaMVG
{

/**
 * MVGCameraPointsLocator is a locator dedicated to drawing the points 
 * seen by the two cameras (Left/Right) selected in MayaMVG.
 */
class MVGCameraPointsLocator : public MPxLocatorNode
{
public:
    enum EDisplayMode {
        eDisplayModeNone = 0,
        eDisplayModeBoth,
        eDisplayModeEach,
        eDisplayModeCommonOnly
    };
    
    struct DrawData {
        MPointArray lPoints;
        MPointArray rPoints;
        MPointArray cPoints;
        MColor lColor;
        MColor rColor;
        MColor cColor;
        float pointSize;
        EDisplayMode displayMode;
    };

public:
    MVGCameraPointsLocator();
    virtual ~MVGCameraPointsLocator();

    virtual void postConstructor();
    static void* creator();
    static MStatus initialize();
    void getDrawData(DrawData& data) const;
    virtual void draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
                      M3dView::DisplayStatus status);

public:
    static MObject aLeftViewPoints;
    static MObject aRightViewPoints;
    static MObject aCommonPoints;
    static MObject aLeftPointsColor;
    static MObject aRightPointsColor;
    static MObject aCommonPointsColor;
    static MObject aDisplayMode;
    static MTypeId _id;
    static MString classification;
    static MString registrantId;
};


class CameraPointsLocatorData : public MUserData
{
public:
    CameraPointsLocatorData() : MUserData(false) {} // Don't delete after draw
    virtual ~CameraPointsLocatorData() {}
    
    MVGCameraPointsLocator::DrawData drawData;    
};

/**
 * Draw override for MVGCameraPointsLocator, providing Viewport 2.0 compatibility.
 */
class MVGCameraPointsDrawOverride : public MHWRender::MPxDrawOverride 
{
public:
    static MHWRender::MPxDrawOverride* creator(const MObject& obj)
    {
        return new MVGCameraPointsDrawOverride(obj);
    }
public:
    virtual ~MVGCameraPointsDrawOverride() {};

    virtual MHWRender::DrawAPI supportedDrawAPIs() const {
        return MHWRender::kOpenGL;
    }
    virtual bool hasUIDrawables() const { return true; };

    virtual bool isBounded(
        const MDagPath& objPath,
        const MDagPath& cameraPath) const {
        return false;
    }
    
    static void draw(const MHWRender::MDrawContext&, const MUserData*);

    virtual MUserData* prepareForDraw(
            const MDagPath& objPath,
            const MDagPath& cameraPath,
            const MHWRender::MFrameContext& frameContext,
            MUserData* oldData);
    
    virtual void addUIDrawables(
            const MDagPath& objPath,
            MHWRender::MUIDrawManager& drawManager,
            const MHWRender::MFrameContext& frameContext,
            const MUserData* data);
    
private:
    MVGCameraPointsDrawOverride(const MObject& obj):
    MHWRender::MPxDrawOverride(obj, MVGCameraPointsDrawOverride::draw)
    {
    }
};

}