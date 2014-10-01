#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/qt/MVGPanelWrapper.hpp"
#include "mayaMVG/qt/MVGMainWidget.hpp"
#include <maya/MFnDependencyNode.h>
#include <maya/MSelectionList.h>
#include <maya/MQtUtil.h>

namespace mayaMVG
{

namespace
{ // empty namespace

MVGProjectWrapper* getProjectWrapper()
{
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
        return NULL;
    MVGMainWidget* mainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
    if(!mainWidget)
        return NULL;
    return &mainWidget->getProjectWrapper();
}

} // empty namespace

static void selectionChangedCB(void*)
{
    MVGProjectWrapper* project = getProjectWrapper();
    if(!project)
        return;
    MSelectionList list;
    MGlobal::getActiveSelectionList(list);
    MDagPath path;
    QList<QString> selectedCameras;
    for(size_t i = 0; i < list.length(); i++)
    {
        list.getDagPath(i, path);
        path.extendToShape();
        if(path.isValid() &&
           ((path.child(0).apiType() == MFn::kCamera) || (path.apiType() == MFn::kCamera)))
        {
            MFnDependencyNode fn(path.transform());
            selectedCameras.push_back(fn.name().asChar());
        }
    }
    project->selectItems(selectedCameras);
}

static void currentContextChangedCB(void*)
{
    MVGProjectWrapper* project = getProjectWrapper();
    if(!project)
        return;
    MString context;
    CHECK(MVGMayaUtil::getCurrentContext(context))
    project->setCurrentContext(MQtUtil::toQString(context));
}

static void sceneChangedCB(void*)
{
    MVGProjectWrapper* project = getProjectWrapper();
    if(!project)
        return;
    MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                  "window.mvgReloadPanels()");
    project->loadExistingProject();
    MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
}

static void undoCB(void*)
{
    // TODO : rebuild only the modified mesh
    // TODO : don't rebuild on action that don't modify any mesh
    MString redoName;
    MVGMayaUtil::getRedoName(redoName);
    // Don't rebuild on selection
    int spaceIndex = redoName.index(' ');
    MString cmdName = redoName.substring(0, spaceIndex - 1);
    if(cmdName != "select" && cmdName != "miCreateDefaultPresets")
        MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
    if(cmdName == "doDelete")
    {
        MVGProjectWrapper* project = getProjectWrapper();
        if(!project)
            return;
        MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                      "window.mvgReloadPanels()");
        project->loadExistingProject();
    }
}

static void redoCB(void*)
{
    MString undoName;
    MVGMayaUtil::getUndoName(undoName);
    // Don't rebuild if selection action
    int spaceIndex = undoName.index(' ');
    MString cmdName = undoName.substring(0, spaceIndex - 1);
    if(cmdName != "select" && cmdName != "miCreateDefaultPresets")
        MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
    if(cmdName == "doDelete")
    {
        MVGProjectWrapper* project = getProjectWrapper();
        if(!project)
            return;
        MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                      "window.mvgReloadPanels()");
        project->loadExistingProject();
    }
}

static void nodeRemovedCB(MObject& node, void*)
{
    MVGProjectWrapper* project = getProjectWrapper();
    if(!project)
        return;
    // Check that it is one of ours cameras
    MDagPath cameraPath;
    CHECK_RETURN(MDagPath::getAPathTo(node, cameraPath))
    MVGCamera camera(cameraPath);
    if(!camera.isValid())
        return;
    // Remove camera from UI model
    project->removeCameraFromUI(cameraPath);
    // TODO : use project.reloadMVGCamerasFromMaya();
}

static void modelEditorChangedCB(void*)
{
    MVGProjectWrapper* project = getProjectWrapper();
    if(!project)
        return;
    for(int i = 0; i < project->getPanelList()->count(); ++i)
    {
        MVGPanelWrapper* panel =
            static_cast<MVGPanelWrapper*>(project->getPanelList()->at(i));
        if(!panel)
            return;
        panel->emitIsPointCloudDisplayedChanged();
    }
}

} // namespace
