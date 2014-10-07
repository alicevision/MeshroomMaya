#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/maya/MVGMayaCallbacks.hpp"
#include "mayaMVG/maya/cmd/MVGCmd.hpp"
#include "mayaMVG/maya/cmd/MVGEditCmd.hpp"
#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/maya/mesh/MVGMeshEditNode.hpp"
#include <maya/MFnPlugin.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <maya/MMessage.h>
#include <maya/MDGMessage.h>

using namespace mayaMVG;

namespace
{ // empty namespace

MCallbackIdArray _callbacks;

} // empty namespace

MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "0.1.0", "Any");

    // Register Maya context, commands & nodes
    CHECK(plugin.registerCommand("MVGCmd", MVGCmd::creator))
    CHECK(plugin.registerContextCommand(MVGContextCmd::name, &MVGContextCmd::creator,
                                        MVGEditCmd::_name, MVGEditCmd::creator,
                                        MVGEditCmd::newSyntax))
    CHECK(plugin.registerNode("MVGCreateManipulator", MVGCreateManipulator::_id,
                              &MVGCreateManipulator::creator, &MVGCreateManipulator::initialize,
                              MPxNode::kManipulatorNode))
    CHECK(plugin.registerNode("MVGMoveManipulator", MVGMoveManipulator::_id,
                              &MVGMoveManipulator::creator, &MVGMoveManipulator::initialize,
                              MPxNode::kManipulatorNode))
    CHECK(plugin.registerNode("MVGMeshEditNode", MVGMeshEditNode::_id, MVGMeshEditNode::creator,
                              MVGMeshEditNode::initialize))

    // Register Maya callbacks
    MCallbackId id;
    id = MEventMessage::addEventCallback("PostToolChanged", currentContextChangedCB, &status);
    if(status)
        _callbacks.append(id);
    MEventMessage::addEventCallback("NewSceneOpened", sceneChangedCB, &status);
    if(status)
        _callbacks.append(id);
    MEventMessage::addEventCallback("SceneOpened", sceneChangedCB, &status);
    if(status)
        _callbacks.append(id);
    MEventMessage::addEventCallback("Undo", undoCB, &status);
    if(status)
        _callbacks.append(id);
    MEventMessage::addEventCallback("Redo", redoCB, &status);
    if(status)
        _callbacks.append(id);
    MEventMessage::addEventCallback("SelectionChanged", selectionChangedCB, &status);
    if(status)
        _callbacks.append(id);
    MEventMessage::addEventCallback("modelEditorChanged", modelEditorChangedCB, &status);
    if(status)
        _callbacks.append(id);
    MDGMessage::addNodeRemovedCallback(nodeRemovedCB, "camera", &status);
    if(status)
        _callbacks.append(id);

    // Create custom GUI
    CHECK(plugin.registerUI("mayaMVGCreateUI", "mayaMVGDeleteUI"))
    CHECK(MVGMayaUtil::createMVGContext())

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj);

    // Delete custom GUI
    CHECK(MVGMayaUtil::deleteMVGContext())
    CHECK(MVGMayaUtil::deleteMVGWindow())

    // Deregister Maya callbacks
    CHECK(MMessage::removeCallbacks(_callbacks))

    // Deregister Maya context, commands & nodes
    CHECK(plugin.deregisterCommand("MVGCmd"))
    CHECK(plugin.deregisterContextCommand(MVGContextCmd::name, MVGEditCmd::_name))
    CHECK(plugin.deregisterNode(MVGCreateManipulator::_id))
    CHECK(plugin.deregisterNode(MVGMoveManipulator::_id))
    CHECK(plugin.deregisterNode(MVGMeshEditNode::_id))

    return status;
}
