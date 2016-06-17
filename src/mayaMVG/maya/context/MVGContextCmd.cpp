#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/maya/context/MVGLocatorManipulator.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MPxManipulatorNode.h>
#include <maya/MUserEventMessage.h>

namespace
{ // empty namespace

static const char* rebuildFlag = "-r";
static const char* rebuildFlagLong = "-rebuild";
static const char* meshFlag = "-m";
static const char* meshFlagLong = "-mesh";
static const char* editModeFlag = "-em";
static const char* editModeFlagLong = "-editMode";
static const char* moveModeFlag = "-mv";
static const char* moveModeFlagLong = "-moveMode";

} // empty namespace

namespace mayaMVG
{

MString MVGContextCmd::name("mayaMVGTool");
MString MVGContextCmd::instanceName("mayaMVGTool1");

MVGContextCmd::MVGContextCmd()
    : _context(NULL)
{
}

MPxContext* MVGContextCmd::makeObj()
{
    _context = new MVGContext();
    return _context;
}

void* MVGContextCmd::creator()
{
    return new MVGContextCmd;
}

MStatus MVGContextCmd::doEditFlags()
{
    MArgParser argData = parser();
    // -rebuild: rebuild cache
    if(argData.isFlagSet(rebuildFlag))
    {
        MVGManipulatorCache& cache = _context->getCache();
        if(argData.isFlagSet(meshFlag))
        {
            MString meshName;
            argData.getFlagArgument(meshFlag, 0, meshName);
            MVGMesh mesh(meshName);
            cache.rebuildMeshCache(mesh.getDagPath());
            return MS::kSuccess;
        }
        cache.rebuildMeshesCache();
    }
    if(argData.isFlagSet(editModeFlag))
    {
        MString editModeString;
        argData.getFlagArgument(editModeFlag, 0, editModeString);
        MVGContext::EEditMode editMode = static_cast<MVGContext::EEditMode>(editModeString.asInt());

        // delete all manipulators
        _context->deleteManipulators();
        MVGManipulatorCache& cache = _context->getCache();
        // then add a new one, depending on edit mode
        MStatus status;
        MObject manipObject;

        MString currentContext;
        MVGMayaUtil::getCurrentContext(currentContext);
        if(currentContext != MVGContextCmd::instanceName)
            MVGMayaUtil::activeContext();

        switch(editMode)
        {
            case MVGContext::eEditModeCreate:
            {
                MVGCreateManipulator* manip =
                    static_cast<MVGCreateManipulator*>(MPxManipulatorNode::newManipulator(
                        "MVGCreateManipulator", manipObject, &status));
                CHECK_RETURN_STATUS(status)
                if(!manip)
                    return MS::kFailure;
                _context->setEditMode(MVGContext::eEditModeCreate);
                cache.clearSelectedComponent();
                cache.rebuildMeshesCache();
                manip->setContext(_context);
                manip->setCache(&cache);
                break;
            }
            case MVGContext::eEditModeMove:
            {
                MVGMoveManipulator* manip = static_cast<MVGMoveManipulator*>(
                    MPxManipulatorNode::newManipulator("MVGMoveManipulator", manipObject, &status));
                CHECK_RETURN_STATUS(status)
                if(!manip)
                    return MS::kFailure;
                _context->setEditMode(MVGContext::eEditModeMove);
                cache.rebuildMeshesCache();
                manip->setContext(_context);
                manip->setCache(&cache);
                break;
            }
            case MVGContext::eEditModeLocator:
            {
                MVGLocatorManipulator* manip =
                    static_cast<MVGLocatorManipulator*>(MPxManipulatorNode::newManipulator(
                        "MVGLocatorManipulator", manipObject, &status));
                CHECK_RETURN_STATUS(status)
                if(!manip)
                    return MS::kFailure;
                _context->setEditMode(MVGContext::eEditModeLocator);
                cache.rebuildMeshesCache();
                manip->clearCameraIDToClickedCSPoint();
                manip->setContext(_context);
                manip->setCache(&cache);
                break;
            }
        }
        if(status)
            _context->addManipulator(manipObject);
    }
    if(argData.isFlagSet(moveModeFlag))
    {
        if(_context->getEditMode() != MVGContext::eEditModeMove)
        {
            LOG_ERROR("moveMode only available with editMode = eMoveMode")
            return MS::kFailure;
        }
        MString moveModeString;
        argData.getFlagArgument(moveModeFlag, 0, moveModeString);
        MVGMoveManipulator::_mode =
            static_cast<MVGMoveManipulator::EMoveMode>(moveModeString.asInt());
        // Remove selection
        if(MVGMoveManipulator::_mode == MVGMoveManipulator::eMoveModeAdjacentFaceProjection ||
           MVGMoveManipulator::_mode == MVGMoveManipulator::eMoveModePointCloudProjection)
            _context->getCache().clearSelectedComponent();
    }
    MUserEventMessage::postUserEvent("modeChangedEvent");
    return MS::kSuccess;
}

MStatus MVGContextCmd::doQueryFlags()
{
    MArgParser argData = parser();

    if(argData.isFlagSet(editModeFlag))
        setResult((int)_context->getEditMode());
    if(argData.isFlagSet(moveModeFlag))
        setResult((int)MVGMoveManipulator::_mode);
    return MS::kSuccess;
}

MStatus MVGContextCmd::appendSyntax()
{
    MSyntax mySyntax = syntax();
    if(MS::kSuccess != mySyntax.addFlag(rebuildFlag, rebuildFlagLong))
        return MS::kFailure;
    if(MS::kSuccess != mySyntax.addFlag(meshFlag, meshFlagLong, MSyntax::kString))
        return MS::kFailure;
    if(MS::kSuccess != mySyntax.addFlag(editModeFlag, editModeFlagLong, MSyntax::kString))
        return MS::kFailure;
    if(MS::kSuccess != mySyntax.addFlag(moveModeFlag, moveModeFlagLong, MSyntax::kString))
        return MS::kFailure;
    return MS::kSuccess;
}

} // namespace
