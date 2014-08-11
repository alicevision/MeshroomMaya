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
    
    void sceneChanged(void* userData)
    {
        if(!userData)
            return;
        // TODO : check project validity
        MDagPath path;
        if(!MVGMayaUtil::getDagPathByName(MVGProject::_PROJECT.c_str(), path))
        {
            LOG_ERROR("Scene is not valid")
            MVGProjectWrapper::instance().clear();
            MStatus status;
            status = MVGMayaUtil::deleteMVGContext();
            status = MVGMayaUtil::deleteMVGWindow();
            CHECK(status)
            return;
        }
        
        MVGProjectWrapper::instance().reloadMVGCamerasFromMaya();       
        MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();
        MVGProjectWrapper::instance().rebuildCacheFromMaya();
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

    const QStringList& qlist = MVGProjectWrapper::instance().getVisiblePanelNames();
	const QString& leftPanelName = qlist[0];
	const QString& rightPanelName = qlist[1];
	
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
	MVGProjectWrapper::instance().reloadMVGCamerasFromMaya();
	MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();

    
    //Maya callbacks
	_callbackIDs.append(MEventMessage::addEventCallback("PostToolChanged", currentContextChanged, mayaWindow));
	_callbackIDs.append(MEventMessage::addEventCallback("NewSceneOpened", sceneChanged, mayaWindow));
	_callbackIDs.append(MEventMessage::addEventCallback("SceneOpened", sceneChanged, mayaWindow));
    
	// -p
	if(argData.isFlagSet(projectPathFlag)) {
		MString projectPath;
		argData.getFlagArgument(projectPathFlag, 0, projectPath);
		MVGProjectWrapper::instance().loadProject(MQtUtil::toQString(projectPath));
	}

	return status;
}
