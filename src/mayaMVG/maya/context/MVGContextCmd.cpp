#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/core/MVGLog.hpp"

#include <maya/MPxManipulatorNode.h>
#include <mayaMVG/maya/MVGMayaUtil.hpp>

namespace
{ // empty namespace

static const char* rebuildFlag = "-r";
static const char* rebuildFlagLong = "-rebuild";
static const char* editModeFlag = "-em";
static const char* editModeFlagLong = "-editMode";
static const char* moveModeFlag = "-mv";
static const char* moveModeFlagLong = "-moveMode";

} // empty namespace

namespace mayaMVG
{

MString MVGContextCmd::name("mayaMVGTool");

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
        cache.rebuildMeshesCache();
    }
    if(argData.isFlagSet(editModeFlag))
    {
        MString editModeString;
        argData.getFlagArgument(editModeFlag, 0, editModeString);
        MVGContext::EditMode editMode = static_cast<MVGContext::EditMode>(editModeString.asInt());

        // delete all manipulators
        _context->deleteManipulators();
        MVGManipulatorCache& cache = _context->getCache();
        // then add a new one, depending on edit mode
        MStatus status;
        MObject manipObject;

        MString currentContext;
        MVGMayaUtil::getCurrentContext(currentContext);
        if(currentContext != "mayaMVGTool1")
            MVGMayaUtil::activeContext();

        switch(editMode)
        {
            case MVGContext::eModeCreate:
            {
                MVGCreateManipulator* manip =
                    static_cast<MVGCreateManipulator*>(MPxManipulatorNode::newManipulator(
                        "MVGCreateManipulator", manipObject, &status));
                CHECK_RETURN_STATUS(status)
                if(!manip)
                    return MS::kFailure;
                _context->setEditMode(MVGContext::eModeCreate);
                cache.rebuildMeshesCache();
                manip->setContext(_context);
                manip->setCache(&cache);
                break;
            }
            case MVGContext::eModeMove:
            {
                MVGMoveManipulator* manip = static_cast<MVGMoveManipulator*>(
                    MPxManipulatorNode::newManipulator("MVGMoveManipulator", manipObject, &status));
                CHECK_RETURN_STATUS(status)
                if(!manip)
                    return MS::kFailure;
                _context->setEditMode(MVGContext::eModeMove);
                cache.rebuildMeshesCache();
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
        if(_context->getEditMode() != MVGContext::eModeMove)
        {
            LOG_ERROR("moveMode only available with editMode = eMoveMode")
            return MS::kFailure;
        }
        MString moveModeString;
        argData.getFlagArgument(moveModeFlag, 0, moveModeString);
        MVGMoveManipulator::_mode =
            static_cast<MVGMoveManipulator::MoveMode>(moveModeString.asInt());
    }
    return MS::kSuccess;
}

MStatus MVGContextCmd::doQueryFlags()
{
    return MS::kSuccess;
}

MStatus MVGContextCmd::appendSyntax()
{
    MSyntax mySyntax = syntax();
    if(MS::kSuccess != mySyntax.addFlag(rebuildFlag, rebuildFlagLong))
        return MS::kFailure;
    if(MS::kSuccess != mySyntax.addFlag(editModeFlag, editModeFlagLong, MSyntax::kString))
        return MS::kFailure;
    if(MS::kSuccess != mySyntax.addFlag(moveModeFlag, moveModeFlagLong, MSyntax::kString))
        return MS::kFailure;
    return MS::kSuccess;
}

} // namespace
