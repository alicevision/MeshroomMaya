#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/version.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/maya/MVGMayaCallbacks.hpp"
#include "mayaMVG/maya/cmd/MVGCmd.hpp"
#include "mayaMVG/maya/cmd/MVGEditCmd.hpp"
#include "mayaMVG/maya/cmd/MVGImagePlaneCmd.hpp"
#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/maya/context/MVGLocatorManipulator.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulatorDrawOverride.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulatorDrawOverride.hpp"
#include "mayaMVG/maya/mesh/MVGMeshEditNode.hpp"
#include "mayaMVG/maya/MVGDummyLocator.h"
#include <maya/MFnPlugin.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <maya/MSceneMessage.h>
#include <maya/MMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>
#include <maya/MCommandMessage.h>
#include <maya/MUserEventMessage.h>

using namespace mayaMVG;

namespace
{ // empty namespace

MCallbackIdArray _callbacks;
MStringArray _commands;
MString _modeChangedEvent = "modeChangedEvent";

} // empty namespace

MStatus registerMVGHotkeys()
{
    MStatus status;
    MString commandName;
    MString cmd;
    MString keySequence;
    MString editModeString;
    MString moveModeString;

    MGlobal::executePythonCommand("from mayaMVG import context");
    // MVGCreateCommand
    commandName = "MVGCreateCommand";
    editModeString = MVGContext::eEditModeCreate;
    cmd.format("^1s -e -em ^2s ^3s", MVGContextCmd::name, editModeString,
               MVGContextCmd::instanceName);
    keySequence = "0";
    cmd.format("context.initMVGCommand(\"^1s\", \"^2s\", \"mel\", \"^3s\", False, True)",
               commandName, cmd, keySequence);
    status = MGlobal::executePythonCommand(cmd);
    CHECK_RETURN_STATUS(status)
    _commands.append(commandName);
    // MVGTriangulationCommand
    commandName = "MVGTriangulationCommand";
    editModeString = MVGContext::eEditModeMove;
    moveModeString = MVGMoveManipulator::eMoveModeNViewTriangulation;
    cmd.format("^1s -e -em ^2s -mv ^3s ^4s", MVGContextCmd::name, editModeString, moveModeString,
               MVGContextCmd::instanceName);
    keySequence = "1";
    cmd.format("context.initMVGCommand(\"^1s\", \"^2s\", \"mel\", \"^3s\", False, True)",
               commandName, cmd, keySequence);
    status = MGlobal::executePythonCommand(cmd);
    CHECK_RETURN_STATUS(status)
    _commands.append(commandName);
    // MVGMovePointCloudCommand
    commandName = "MVGMovePointCloudCommand";
    editModeString = MVGContext::eEditModeMove;
    moveModeString = MVGMoveManipulator::eMoveModePointCloudProjection;
    cmd.format("^1s -e -em ^2s -mv ^3s ^4s", MVGContextCmd::name, editModeString, moveModeString,
               MVGContextCmd::instanceName);
    keySequence = "2";
    cmd.format("context.initMVGCommand(\"^1s\", \"^2s\", \"mel\", \"^3s\", False, True)",
               commandName, cmd, keySequence);
    status = MGlobal::executePythonCommand(cmd);
    CHECK_RETURN_STATUS(status)
    _commands.append(commandName);
    // MVGMoveAdjacentFaceCommand
    commandName = "MVGMoveAdjacentFaceCommand";
    editModeString = MVGContext::eEditModeMove;
    moveModeString = MVGMoveManipulator::eMoveModeAdjacentFaceProjection;
    cmd.format("^1s -e -em ^2s -mv ^3s ^4s", MVGContextCmd::name, editModeString, moveModeString,
               MVGContextCmd::instanceName);
    keySequence = "3";
    cmd.format("context.initMVGCommand(\"^1s\", \"^2s\", \"mel\", \"^3s\", False, True)",
               commandName, cmd, keySequence);
    status = MGlobal::executePythonCommand(cmd);
    CHECK_RETURN_STATUS(status)
    _commands.append(commandName);

    return status;
}

MStatus deregisterMVGHotkeys()
{
    MStatus status;
    for(int i = _commands.length() - 1; i >= 0; --i)
    {
        MString cmd;
        cmd.format("context.removeMVGCommand(\"^1s\")", _commands[i]);
        status = MGlobal::executePythonCommand(cmd);
        CHECK_RETURN_STATUS(status)
        _commands.remove(i);
    }
    return status;
}

static void quitApplicationCB(void*)
{
    deregisterMVGHotkeys();
}

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, MAYAMVG_VERSION, "Any");

    // Register Maya context, commands & nodes
    CHECK(plugin.registerCommand("MVGCmd", MVGCmd::creator))
    CHECK(plugin.registerCommand("MVGImagePlaneCmd", MVGImagePlaneCmd::creator,
                                 MVGImagePlaneCmd::newSyntax))
    CHECK(plugin.registerContextCommand(MVGContextCmd::name, &MVGContextCmd::creator,
                                        MVGEditCmd::_name, MVGEditCmd::creator,
                                        MVGEditCmd::newSyntax))
    CHECK(plugin.registerNode("MVGCreateManipulator", MVGCreateManipulator::_id,
                              &MVGCreateManipulator::creator, &MVGCreateManipulator::initialize,
                              MPxNode::kManipulatorNode,
                              &MVGCreateManipulator::_drawDbClassification))
    CHECK(plugin.registerNode("MVGMoveManipulator", MVGMoveManipulator::_id,
                              &MVGMoveManipulator::creator, &MVGMoveManipulator::initialize,
                              MPxNode::kManipulatorNode,
                              &MVGMoveManipulator::_drawDbClassification))
    CHECK(plugin.registerNode("MVGLocatorManipulator", MVGLocatorManipulator::_id,
                              &MVGLocatorManipulator::creator, &MVGLocatorManipulator::initialize,
                              MPxNode::kManipulatorNode,
                              &MVGLocatorManipulator::_drawDbClassification))
    CHECK(plugin.registerNode("MVGDummyLocator", MVGDummyLocator::_id, &MVGDummyLocator::creator,
                              &MVGDummyLocator::initialize, MPxNode::kLocatorNode))
    CHECK(plugin.registerNode("MVGMeshEditNode", MVGMeshEditNode::_id, MVGMeshEditNode::creator,
                              MVGMeshEditNode::initialize))

    // Register draw overrides
    CHECK(MHWRender::MDrawRegistry::registerDrawOverrideCreator(
        MVGCreateManipulator::_drawDbClassification, MVGCreateManipulator::_drawRegistrantID,
        MVGCreateManipulatorDrawOverride::creator))
    CHECK(MHWRender::MDrawRegistry::registerDrawOverrideCreator(
        MVGMoveManipulator::_drawDbClassification, MVGMoveManipulator::_drawRegistrantID,
        MVGMoveManipulatorDrawOverride::creator))

    // Register Maya callbacks
    MCallbackId id;
    if(!MUserEventMessage::isUserEvent(_modeChangedEvent))
        MUserEventMessage::registerUserEvent(_modeChangedEvent);
    id = MUserEventMessage::addUserEventCallback(_modeChangedEvent, modeChangedCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("PostToolChanged", currentContextChangedCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("NewSceneOpened", newSceneCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("SceneOpened", sceneChangedCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("Undo", undoCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("Redo", redoCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("quitApplication", quitApplicationCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("SelectionChanged", selectionChangedCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("modelEditorChanged", modelEditorChangedCB, &status);
    if(status)
        _callbacks.append(id);
    id = MEventMessage::addEventCallback("linearUnitChanged", linearUnitChanged, &status);
    if(status)
        _callbacks.append(id);
    id = MDGMessage::addNodeRemovedCallback(nodeRemovedCB, "camera", &status);
    if(status)
        _callbacks.append(id);
    id = MDGMessage::addNodeAddedCallback(nodeAddedCB, "mesh", &status);
    if(status)
        _callbacks.append(id);
    id = MDGMessage::addNodeRemovedCallback(nodeRemovedCB, "mesh", &status);
    if(status)
        _callbacks.append(id);

    // Create custom GUI
    CHECK(plugin.registerUI("mayaMVGCreateUI", "mayaMVGDeleteUI"))
    CHECK(MVGMayaUtil::createMVGContext())

    // Create hotkeys
    CHECK(registerMVGHotkeys())

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    // Delete hotkeys
    deregisterMVGHotkeys();

    // Delete custom GUI
    CHECK(MVGMayaUtil::deleteMVGContext())
    CHECK(MVGMayaUtil::deleteMVGWindow())

    // Deregister Maya callbacks
    CHECK(MUserEventMessage::deregisterUserEvent(_modeChangedEvent))
    CHECK(MMessage::removeCallbacks(_callbacks))

    // Deregister Maya context, commands & nodes
    CHECK(plugin.deregisterCommand("MVGCmd"))
    CHECK(plugin.deregisterCommand("MVGImagePlaneCmd"))
    CHECK(plugin.deregisterContextCommand(MVGContextCmd::name, MVGEditCmd::_name))
    CHECK(plugin.deregisterNode(MVGCreateManipulator::_id))
    CHECK(plugin.deregisterNode(MVGMoveManipulator::_id))
    CHECK(plugin.deregisterNode(MVGLocatorManipulator::_id))
    CHECK(plugin.deregisterNode(MVGMeshEditNode::_id))
    CHECK(plugin.deregisterNode(MVGDummyLocator::_id))

    // Deregister draw overrides
    CHECK(MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
        MVGCreateManipulator::_drawDbClassification, MVGCreateManipulator::_drawRegistrantID))
    CHECK(MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
        MVGMoveManipulator::_drawDbClassification, MVGMoveManipulator::_drawRegistrantID))

    return status;
}
