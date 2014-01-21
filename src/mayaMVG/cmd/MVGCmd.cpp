#include <QMainWindow>
#include <QWidgetList>
#include <QApplication>
#include <QLayout>

#include "util/MVGLog.h"
#include "util/MVGUtil.h"
#include "cmd/MVGCmd.h"
#include "qt/MVGMenu.h"
#include "qt/MVGEventFilter.h"

#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MQtUtil.h>
#include <maya/MEventMessage.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>

using namespace mayaMVG;

namespace {

	static const char * helpFlag = "-h";
	static const char * helpFlagLong = "-help";

	QMainWindow* getMainWindow() {
		QWidgetList widgetList = QApplication::topLevelWidgets();
		for (int i = 0; i < widgetList.size(); i++) {
			if (widgetList[i]->isWindow()) {
				QMainWindow* castWidget = dynamic_cast<QMainWindow*>(widgetList[i]);
				if (castWidget != NULL && castWidget->parent() == NULL)
					return (castWidget);
			}
		}
		return NULL;
	}

	void _debug_printMayaTree(const QObject* object) {
		QString path;
		QObject* parent = NULL;
		QList<QWidget *> children = object->findChildren<QWidget *>();
		for (int i = 0; i < children.size(); ++i) {
			path = children[i]->objectName();
			parent = children[i]->parent();
			while(parent != object) {
				if(!parent)
					break;
				path = parent->objectName() + QString("|") + path;
				parent = parent->parent();
			}
			LOG_INFO("TREE", path.toStdString())
		}
	}

	void selectionChanged(void* userData) {
		if(!userData)
			return;
		MVGMenu* menu = static_cast<MVGMenu*>(userData);
		MDagPath path;
		MObject component;
		MSelectionList list;
		MGlobal::getActiveSelectionList(list);
		QList<QString> selectedCameras;
		for ( size_t i = 0; i < list.length(); i++) {
			list.getDagPath(i, path, component);
			path.extendToShape();
			if(path.isValid() && (path.apiType() == MFn::kCamera)) {
				MFnDependencyNode fn(path.transform());
				selectedCameras.push_back(fn.name().asChar());
			}
		}
		menu->selectCameras(selectedCameras);
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
	QWidget* mayaWindow = MVGUtil::createMVGWindow();
	if(!mayaWindow)
		return MS::kFailure;

	// add the custom Qt menu
	QWidget* menuLayout = MQtUtil::findLayout("mvgMenuPanel");
	if(!menuLayout)
		return MS::kFailure;
	MVGMenu* menu = new MVGMenu(NULL);
	QWidget* layout = qobject_cast<QWidget*>(menuLayout);
	MQtUtil::addWidgetToMayaLayout(menu, layout);

	// populate mvg menu
	MVGUtil::populateMenu(menu);

	// install mouse event filter on maya viewports
	// WARNING : this may change depending on maya versions.
	//           Use _debug_printMayaTree(mayaWindow) function for debugging purposes.
	MVGMouseEventFilter * mouseEventFilter = new MVGMouseEventFilter();
	QList<QWidget *> children = mayaWindow->findChildren<QWidget *>();
	for (int i = 0; i < children.size(); ++i) {
		if(children[i]->objectName() == ""){
			if(children[i]->parent()->objectName() == "mvgLPanel") {
				children[i]->installEventFilter(mouseEventFilter);
				children[i]->setProperty("mvg_panel", "left");
				children[i]->setProperty("mvg_mouseFiltered", true);
			}
			if(children[i]->parent()->objectName() == "mvgRPanel") {
				children[i]->installEventFilter(mouseEventFilter);
				children[i]->setProperty("mvg_panel", "right");
				children[i]->setProperty("mvg_mouseFiltered", true);
			}
		}
	}

	// register the 'Selection Changed' callback
	MCallbackIdArray callbackIDs;
	callbackIDs.append(MEventMessage::addEventCallback("SelectionChanged", selectionChanged, menu));

	// install a window event filter on 'mayaWindow'
	// needed to remove all maya callbacks and all Qt event filters 
	MVGWindowEventFilter * windowEventFilter = new MVGWindowEventFilter(callbackIDs, mouseEventFilter);
	mayaWindow->installEventFilter(windowEventFilter);

    return status;
}
