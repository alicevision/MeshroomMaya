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

using namespace mayaMVG;

namespace {

	static const char * helpFlag = "-h";
	static const char * helpFlagLong = "-help";
	static const char * projectPathFlag = "-p";
	static const char * projectPathFlagLong = "-project";
    
    void currentContextChanged(void* userData) 
    {
        if(!userData)
            return;
        
        MString context;
        MStatus status;
        status = MVGMayaUtil::getCurrentContext(context);
        CHECK(status)
        
        MVGProjectWrapper::instance().setCurrentContext(QString(context.asChar()));
    }
}

MCallbackIdArray MVGCmd::_callbackIDs; // = MCallbackIdArray(); // FIXME /!\ 

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

	// -h
	if (argData.isFlagSet(helpFlag)) {
		// TODO
	}
	
	// create maya window
	const QStringList& qlist = MVGProjectWrapper::instance().getVisiblePanelNames();
	std::vector<MString> mlist;
	mlist.push_back(qlist[0].toStdString().c_str());
	mlist.push_back(qlist[1].toStdString().c_str());
	status = MVGMayaUtil::createMVGWindow();
	if(!status) {
		LOG_ERROR("Unable to create MVGContext.")
		return status;
	}
	QWidget* mayaWindow = MVGMayaUtil::getMVGWindow();
	mayaWindow->setProperty("mvg_window", QString("MVGWindow"));
	QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
	if(!mayaWindow || !menuLayout) {
		LOG_ERROR("Unable to retrieve MVGWindow.")
		return MS::kFailure;
	}
	
	// create maya MVGContext
	status = MVGMayaUtil::createMVGContext();
	if(!status) {
		LOG_ERROR("Unable to create MVGContext.")
		return status;
	}

	const QString& leftPanelName = MVGProjectWrapper::instance().getVisiblePanelNames().at(0);
	const QString& rightPanelName = MVGProjectWrapper::instance().getVisiblePanelNames().at(1);
	
	QWidget* leftViewport = MVGMayaUtil::getMVGViewportLayout(leftPanelName.toStdString().c_str());
	QWidget* rightViewport = MVGMayaUtil::getMVGViewportLayout(rightPanelName.toStdString().c_str());
	if(!leftViewport || !rightViewport) {
		LOG_ERROR("Unable to retrieve maya viewport layouts.");
		return MS::kFailure;
	}
	leftViewport->setProperty("mvg_panel", leftPanelName);
	rightViewport->setProperty("mvg_panel", rightPanelName);

	// create MVG menu
	MVGMainWidget* mainWidget = new MVGMainWidget(menuLayout);
	MQtUtil::addWidgetToMayaLayout(mainWidget->view(), menuLayout);
	
	// Reload project from Maya
	MVGProjectWrapper::instance().reloadProjectFromMaya();
	MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();

    
    //Maya callbacks
	_callbackIDs.append(MEventMessage::addEventCallback("PostToolChanged", currentContextChanged, mayaWindow));

	// -p
	if(argData.isFlagSet(projectPathFlag)) {
		MString projectPath;
		argData.getFlagArgument(projectPathFlag, 0, projectPath);
		MVGProjectWrapper::instance().loadProject(MQtUtil::toQString(projectPath));
	}

	return status;
}
