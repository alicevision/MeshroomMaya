#pragma once

#include "meshroomMaya/maya/context/MVGManipulatorCache.hpp"
#include <maya/MPxDrawOverride.h>
#include <maya/MUserData.h>
#include <maya/MPointArray.h>

namespace meshroomMaya
{

class MoveDrawData : public MUserData
{
public:
    MoveDrawData()
        : MUserData(false) // don't delete after draw
        , doDraw(false)
        , cache(NULL)
    {
    }
    virtual ~MoveDrawData() {}

public:
    bool doDraw;
    int portWidth;
    int portHeight;
    MPoint mouseVSPoint;
    MPointArray finalWSPoints;
    MPointArray intersectedVSPoints;
    MVGManipulatorCache* cache;
};

class MVGMoveManipulatorDrawOverride : public MHWRender::MPxDrawOverride
{
public:
    MVGMoveManipulatorDrawOverride(const MObject& obj);
    virtual ~MVGMoveManipulatorDrawOverride() {}

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
