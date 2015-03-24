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
#include "mayaMVG/maya/context/MVGCreateManipulatorDrawOverride.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulatorDrawOverride.hpp"
#include "mayaMVG/maya/mesh/MVGMeshEditNode.hpp"
#include <maya/MFnPlugin.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <maya/MMessage.h>
#include <maya/MDGMessage.h>
#include <maya/MDrawRegistry.h>

using namespace mayaMVG;

namespace
{ // empty namespace

MCallbackIdArray _callbacks;

} // empty namespace

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
    id = MDGMessage::addNodeRemovedCallback(nodeRemovedCB, "mesh", &status);
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
    CHECK(plugin.deregisterCommand("MVGImagePlaneCmd"))
    CHECK(plugin.deregisterContextCommand(MVGContextCmd::name, MVGEditCmd::_name))
    CHECK(plugin.deregisterNode(MVGCreateManipulator::_id))
    CHECK(plugin.deregisterNode(MVGMoveManipulator::_id))
    CHECK(plugin.deregisterNode(MVGMeshEditNode::_id))

    // Deregister draw overrides
    CHECK(MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
        MVGCreateManipulator::_drawDbClassification, MVGCreateManipulator::_drawRegistrantID))
    CHECK(MHWRender::MDrawRegistry::deregisterDrawOverrideCreator(
        MVGMoveManipulator::_drawDbClassification, MVGMoveManipulator::_drawRegistrantID))

    return status;
}
