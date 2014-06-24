#include "QtDeclarative/QDeclarativeView"
#include "QtDeclarative/qdeclarativecontext.h"
#include "mayaMVG/maya/cmd/MVGCmd.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/qt/MVGViewportEventFilter.h"
#include "mayaMVG/qt/MVGWindowEventFilter.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/qt/MVGMainWidget.h"
#include <mayaMVG/qt/MVGProjectWrapper.h>
#include <maya/MQtUtil.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MEventMessage.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>

using namespace mayaMVG;

namespace {

	static const char * helpFlag = "-h";
	static const char * helpFlagLong = "-help";
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
		MVGProjectWrapper::instance().selectItems(selectedCameras);
	}

}

MVGCmd::MVGCmd() {
}

MVGCmd::~MVGCmd() {
}

void * MVGCmd::creator() {
	return new MVGCmd();
}

MSyntax MVGCmd::newSyntax() {
	MSyntax s;
	s.addFlag(helpFlag, helpFlagLong);
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

	MString nodeName;
	MString attributeName;
	MString uiName;

	// -h
	if (argData.isFlagSet(helpFlag)) {
		// TODO
	}
	
	// create maya window
	status = MVGMayaUtil::createMVGWindow();
	if(!status) {
		LOG_ERROR("Unable to create MVGContext.")
		return status;
	}
	QWidget* mayaWindow = MVGMayaUtil::getMVGWindow();
	QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
	if(!mayaWindow || !menuLayout) {
		LOG_ERROR("Unable to retrieve MVGWindow.")
		return status;
	}

	// create MVG menu
	MVGMainWidget* mainWidget = new MVGMainWidget(menuLayout);
	MQtUtil::addWidgetToMayaLayout(mainWidget->view(), menuLayout);
			
	// create maya MVGContext
	status = MVGMayaUtil::createMVGContext();
	if(!status) {
		LOG_ERROR("Unable to create MVGContext.")
		return status;
	}

	// install mouse event filter on maya viewports
	MVGViewportEventFilter * viewportEventFilter = new MVGViewportEventFilter(mayaWindow);
	QWidget* leftViewport = MVGMayaUtil::getMVGViewportLayout("mvgLPanel");
	QWidget* rightViewport = MVGMayaUtil::getMVGViewportLayout("mvgRPanel");
	if(!leftViewport || !rightViewport) {
		LOG_ERROR("Unable to retrieve maya viewport layouts.");
		return MS::kFailure;
	}

	leftViewport->installEventFilter(viewportEventFilter);
	leftViewport->setProperty("mvg_panel", "mvgLPanel");
	rightViewport->installEventFilter(viewportEventFilter);
	rightViewport->setProperty("mvg_panel", "mvgRPanel");

	// maya callbacks
	MCallbackIdArray callbackIDs;
	callbackIDs.append(MEventMessage::addEventCallback("SelectionChanged", selectionChangedCB, mainWidget->view()));

	// install a window event filter on 'mayaWindow'
	// needed to remove all maya callbacks and all Qt event filters 
	MVGWindowEventFilter * windowEventFilter = new MVGWindowEventFilter(callbackIDs, viewportEventFilter, mayaWindow);
	mayaWindow->installEventFilter(windowEventFilter); // auto delete on window close
	
	// -p
	if(argData.isFlagSet(projectPathFlag)) {
		MString projectPath;
		argData.getFlagArgument(projectPathFlag, 0, projectPath);
		MVGProjectWrapper::instance().loadProject(MQtUtil::toQString(projectPath));
	}

	return status;
}
