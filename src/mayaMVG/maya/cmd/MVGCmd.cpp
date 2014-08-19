#include "QtDeclarative/QDeclarativeView"
#include "QtDeclarative/qdeclarativecontext.h"
#include "mayaMVG/maya/cmd/MVGCmd.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/qt/MVGMainWidget.h"
#include <mayaMVG/qt/MVGProjectWrapper.h>
#include <maya/MQtUtil.h>
#include <maya/MGlobal.h>
#include <maya/MEventMessage.h>
#include <maya/MMessage.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>

using namespace mayaMVG;

namespace {

	static const char * projectPathFlag = "-p";
	static const char * projectPathFlagLong = "-project";

    void selectionChangedCB(void* userData) {
		if(!userData)
			return;
		MDagPath path;
		MObject component;
		MSelectionList list;
		MGlobal::getActiveSelectionList(list);
		QList<QString> selectedCameras;
		for ( size_t i = 0; i < list.length(); i++) {
			list.getDagPath(i, path, component);
			path.extendToShape();
			if(path.isValid() 
				&& ((path.child(0).apiType() == MFn::kCamera) || (path.apiType() == MFn::kCamera))) {
				MFnDependencyNode fn(path.transform());
				selectedCameras.push_back(fn.name().asChar());
			}
		}
		// MVGProjectWrapper::instance().selectItems(selectedCameras);
	}

    void currentContextChangedCB(void* userData)
    {
        if(!userData)
            return;
        MString context;
        MStatus status;
        status = MVGMayaUtil::getCurrentContext(context);
        CHECK(status)
        // MVGProjectWrapper::instance().setCurrentContext(QString(context.asChar()));
    }

    void sceneChangedCB(void* userData)
    {
        if(!userData)
            return;
        // TODO : check project validity
        MDagPath path;
        if(!MVGMayaUtil::getDagPathByName(MVGProject::_PROJECT.c_str(), path))
        {
            LOG_ERROR("Scene is not valid")
            // MVGProjectWrapper::instance().clear();
            MStatus status;
            status = MVGMayaUtil::deleteMVGContext();
            status = MVGMayaUtil::deleteMVGWindow();
            CHECK(status)
            return;
        }
        // MVGProjectWrapper::instance().reloadMVGCamerasFromMaya();       
        // MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();
        // MVGProjectWrapper::instance().rebuildCacheFromMaya();
    }

    void undoCB(void * userData)
    {
        if(!userData)
           return;
        // TODO : rebuild only the mesh modified
        // TODO : don't rebuild on action that don't modify any mesh
        MString redoName;
        MVGMayaUtil::getRedoName(redoName);        
        // Don't rebuild if selection action
        int spaceIndex = redoName.index(' ');
        MString cmdName = redoName.substring(0, spaceIndex - 1);
        if(cmdName != "select" && cmdName != "miCreateDefaultPresets")
            MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
    }

    void redoCB(void * userData)
    {
        if(!userData)
           return;
        MString undoName;
        MVGMayaUtil::getUndoName(undoName);
        // Don't rebuild if selection action
        int spaceIndex = undoName.index(' ');
        MString cmdName = undoName.substring(0, spaceIndex - 1);
        if(cmdName != "select" && cmdName != "miCreateDefaultPresets")
            MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
    }

}

MCallbackIdArray MVGCmd::_callbackIDs;

MVGCmd::MVGCmd() {
}

MVGCmd::~MVGCmd() {
}

void * MVGCmd::creator() {
	return new MVGCmd();
}

MSyntax MVGCmd::newSyntax() {
	MSyntax s;
	s.addFlag(projectPathFlag, projectPathFlagLong, MSyntax::kString);
	s.enableEdit(false);
	s.enableQuery(false);
	return s;
}

MStatus MVGCmd::doIt(const MArgList& args) {
	MStatus status = MS::kSuccess;

	// parsing ARGS
	MSyntax syntax = MVGCmd::newSyntax();
	MArgDatabase argData(syntax, args);

	// create maya window
	status = MVGMayaUtil::createMVGWindow();
	if(!status) {
		LOG_ERROR("Unable to create MVGWindow.")
		return status;
	}

	// get layout from
	QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
	if(!menuLayout) {
		LOG_ERROR("Unable to retrieve MVGMenuLayout.")
		return MS::kFailure;
	}

	// create maya MVGContext
	status = MVGMayaUtil::createMVGContext();
	if(!status) {
		LOG_ERROR("Unable to create MVGContext.")
		return status;
	}

	// set window properties
	QWidget* mayaWindow = MVGMayaUtil::getMVGWindow();
	if(!mayaWindow) {
		LOG_ERROR("Unable to retrieve MVGWindow.")
		return MS::kFailure;
	}
	mayaWindow->setProperty("mvg_window", QString("MVGWindow"));

	// set layout properties
	const QString& leftPanelName = "mvgLPanel";
	const QString& rightPanelName = "mvgRPanel";
	QWidget* leftViewport = MVGMayaUtil::getMVGViewportLayout(leftPanelName.toStdString().c_str());
	QWidget* rightViewport = MVGMayaUtil::getMVGViewportLayout(rightPanelName.toStdString().c_str());
	if(!leftViewport || !rightViewport) {
		LOG_ERROR("Unable to retrieve MVG panels.");
		return MS::kFailure;
	}
	leftViewport->setProperty("mvg_panel", leftPanelName);
	rightViewport->setProperty("mvg_panel", rightPanelName);

	// create MVG menu
	MVGMainWidget* mainWidget = new MVGMainWidget(menuLayout);
	MQtUtil::addWidgetToMayaLayout(mainWidget->view(), menuLayout);
	
	// // Reload project from Maya
	// MVGProjectWrapper::instance().reloadProjectFromMaya();
	// MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();

    //Maya callbacks
	_callbackIDs.append(MEventMessage::addEventCallback("PostToolChanged", currentContextChangedCB, mayaWindow));
	_callbackIDs.append(MEventMessage::addEventCallback("NewSceneOpened", sceneChangedCB, mayaWindow));
	_callbackIDs.append(MEventMessage::addEventCallback("SceneOpened", sceneChangedCB, mayaWindow));
	_callbackIDs.append(MEventMessage::addEventCallback("Undo", undoCB, mayaWindow));
	_callbackIDs.append(MEventMessage::addEventCallback("Redo", redoCB, mayaWindow));
    _callbackIDs.append(MEventMessage::addEventCallback("SelectionChanged", selectionChangedCB, mainWidget->view()));

	// -p
	// if(argData.isFlagSet(projectPathFlag)) {
	// 	MString projectPath;
	// 	argData.getFlagArgument(projectPathFlag, 0, projectPath);
	// 	MVGProjectWrapper::instance().loadProject(MQtUtil::toQString(projectPath));
	// }

	return status;
}
