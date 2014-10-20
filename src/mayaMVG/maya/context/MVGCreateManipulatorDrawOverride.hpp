#pragma once
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MPointArray.h>

namespace mayaMVG
{

class CreateDrawData : public MUserData
{
public:
    CreateDrawData()
        : MUserData(false) // don't delete after draw
        , doDraw(false)
    {
    }
    virtual ~CreateDrawData() {}

public:
    bool doDraw;
    int portWidth;
    int portHeight;
    MPointArray finalWSPoints;
    MPointArray clickedVSPoints;
    MPointArray intersectedVSPoints;
};

class MVGCreateManipulatorDrawOverride : public MHWRender::MPxDrawOverride
{
public:
    MVGCreateManipulatorDrawOverride(const MObject& obj);
    virtual ~MVGCreateManipulatorDrawOverride() {}

public:
    static MHWRender::MPxDrawOverride* creator(const MObject& obj);
    static void draw(const MHWRender::MDrawContext& context, const MUserData* data);

public:
    bool isBounded(const MDagPath& objPath, const MDagPath& cameraPath) const;
    virtual MBoundingBox boundingBox(const MDagPath& objPath, const MDagPath& cameraPath) const;
    virtual MUserData* prepareForDraw(const MDagPath& objPath, const MDagPath& cameraPath,
                                      const MHWRender::MFrameContext& frameContext,
                                      MUserData* oldData);
};

} // namespace
