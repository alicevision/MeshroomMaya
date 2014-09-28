#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/qt/MVGPanelWrapper.h"
#include "mayaMVG/qt/MVGMainWidget.h"
#include <maya/MFnDependencyNode.h>
#include <maya/MSelectionList.h>

namespace mayaMVG
{

void selectionChangedCB(void* /*userData*/)
{
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
        return;
    MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
    if(!mvgMainWidget)
        return;
    MDagPath path;
    MObject component;
    MSelectionList list;
    MGlobal::getActiveSelectionList(list);
    QList<QString> selectedCameras;
    for(size_t i = 0; i < list.length(); i++)
    {
        list.getDagPath(i, path, component);
        path.extendToShape();
        if(path.isValid() &&
           ((path.child(0).apiType() == MFn::kCamera) || (path.apiType() == MFn::kCamera)))
        {
            MFnDependencyNode fn(path.transform());
            selectedCameras.push_back(fn.name().asChar());
        }
    }
    mvgMainWidget->getProjectWrapper().selectItems(selectedCameras);
}

void currentContextChangedCB(void* /*userData*/)
{
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
        return;
    MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
    if(!mvgMainWidget)
        return;
    MString context;
    MStatus status;
    status = MVGMayaUtil::getCurrentContext(context);
    CHECK(status)
    mvgMainWidget->getProjectWrapper().setCurrentContext(QString(context.asChar()));
}

void sceneChangedCB(void* /*userData*/)
{
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
        return;
    MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
    if(!mvgMainWidget)
        return;
    MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                  "window.mvgReloadPanels()");
    mvgMainWidget->getProjectWrapper().loadExistingProject();
    MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
}

void undoCB(void* /*userData*/)
{
    // TODO : rebuild only the mesh modified
    // TODO : don't rebuild on action that don't modify any mesh
    MString redoName;
    MVGMayaUtil::getRedoName(redoName);
    // Don't rebuild if selection action
    int spaceIndex = redoName.index(' ');
    MString cmdName = redoName.substring(0, spaceIndex - 1);
    if(cmdName != "select" && cmdName != "miCreateDefaultPresets")
        MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
    if(cmdName == "doDelete")
    {
        QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
        if(!menuLayout)
            return;
        MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
        if(!mvgMainWidget)
            return;
        MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                      "window.mvgReloadPanels()");
        mvgMainWidget->getProjectWrapper().loadExistingProject();
    }
}

void redoCB(void* /*userData*/)
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
        QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
        if(!menuLayout)
            return;
        MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
        if(!mvgMainWidget)
            return;
        MGlobal::executePythonCommand("from mayaMVG import window;\n"
                                      "window.mvgReloadPanels()");
        mvgMainWidget->getProjectWrapper().loadExistingProject();
    }
}

void nodeRemovedCB(MObject& node, void* /*clientData*/)
{
    MStatus status;

    // Check that it is one of ours cameras
    MDagPath cameraPath;
    status = MDagPath::getAPathTo(node, cameraPath);
    CHECK_RETURN(status)
    MVGCamera camera(cameraPath);
    if(!camera.isValid())
        return;
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
        return;
    MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
    if(!mvgMainWidget)
        return;
    MVGProjectWrapper& project = mvgMainWidget->getProjectWrapper();
    // Remove camera from UI model
    project.removeCameraFromUI(cameraPath);
    // TODO : use project.reloadMVGCamerasFromMaya();
}

void modelEditorChangedCB(void* /*userData*/)
{
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
        return;
    MVGMainWidget* mvgMainWidget = menuLayout->findChild<MVGMainWidget*>("mvgMainWidget");
    if(!mvgMainWidget)
        return;
    MVGProjectWrapper& project = mvgMainWidget->getProjectWrapper();
    for(int i = 0; i < project.getPanelList()->count(); ++i)
    {
        MVGPanelWrapper* panel = static_cast<MVGPanelWrapper*>(project.getPanelList()->at(i));
        if(!panel)
            return;
        panel->emitIsPointCloudDisplayedChanged();
    }
}

} // namespace
