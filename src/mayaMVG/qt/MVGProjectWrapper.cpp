#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>
// #include <maya/MItDependencyNodes.h>
// #include <maya/MFnDependencyNode.h>
// #include <maya/MFnMesh.h>
// #include <maya/MItMeshEdge.h>
// #include <maya/MItMeshVertex.h>
// #include <maya/MItMeshPolygon.h>
#include "mayaMVG/core/MVGGeometryUtil.h"

using namespace mayaMVG;

MVGProjectWrapper::MVGProjectWrapper()
{
	_allPanelNames.append("mvgLPanel");
	_allPanelNames.append("mvgRPanel");
	_visiblePanelNames = _allPanelNames;
	
	_project = MVGProject(MVGProject::_PROJECT);
	if(!_project.isValid()) {
		_project = MVGProject::create(MVGProject::_PROJECT);
		LOG_INFO("New OpenMVG Project.")
	}
    
    // // Initialize currentContext
    // MString context;
    // MVGMayaUtil::getCurrentContext(context);
    // _currentContext = context.asChar();
}

MVGProjectWrapper::~MVGProjectWrapper()
{
}

const QString MVGProjectWrapper::moduleDirectory() const
{
	return QString(_project.moduleDirectory().c_str());
}

const QString MVGProjectWrapper::projectDirectory() const
{
	return QString(_project.projectDirectory().c_str());
}

const QString MVGProjectWrapper::cameraDirectory() const
{
	return QString(_project.cameraDirectory().c_str());
}

const QString MVGProjectWrapper::imageDirectory() const
{
	return QString(_project.imageDirectory().c_str());
}

const QString MVGProjectWrapper::pointCloudFile() const
{
	return QString(_project.pointCloudFile().c_str());
}

const QString MVGProjectWrapper::logText() const
{
	return _logText;
}

void MVGProjectWrapper::setLogText(const QString& text)
{
	_logText = text;
	emit logTextChanged();
}

// const QString MVGProjectWrapper::currentContext() const
// {
//     return _currentContext;
// }

// void MVGProjectWrapper::setCurrentContext(const QString& context)
// {
//     _currentContext = context;
//     Q_EMIT currentContextChanged();
// }

void MVGProjectWrapper::appendLogText(const QString& text)
{
	_logText.append(text + "\n");
	emit logTextChanged();
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
	_project.setProjectDirectory(directory.toStdString());
	emit projectDirectoryChanged();
}

QString MVGProjectWrapper::openFileDialog() const
{
	MString directoryPath;
	MVGMayaUtil::openFileDialog(directoryPath);	
    return MQtUtil::toQString(directoryPath);
}

void MVGProjectWrapper::activeSelectionContext() {
	MVGMayaUtil::activeSelectionContext();
}

void MVGProjectWrapper::activeMVGContext() 
{
	MVGMayaUtil::activeContext();
    // rebuildAllMeshesCacheFromMaya();
    // rebuildCacheFromMaya();
}

void MVGProjectWrapper::loadProject(const QString& projectDirectoryPath)
{	
	_project.setProjectDirectory(projectDirectoryPath.toStdString());
	if(!_project.load()) {
		LOG_ERROR("An error occured when loading project.");
		appendLogText(QString("An error occured when loading project."));
	}
	Q_EMIT projectDirectoryChanged();
	
	reloadProjectFromMaya();
	
	// Select the two first cameras for the views
	if(_cameraList.size() > 1) {
		QList<MVGCameraWrapper*>& cameras = _cameraList.asQList<MVGCameraWrapper>();
		setCameraToView(cameras[0], _visiblePanelNames[0]);
		setCameraToView(cameras[1], _visiblePanelNames[1]);
	}
	// rebuildCacheFromMaya();
}

void MVGProjectWrapper::selectItems(const QList<QString>& cameraNames)
{
    foreach(MVGCameraWrapper* camera, _cameraList.asQList<MVGCameraWrapper>())
        camera->setIsSelected(cameraNames.contains(camera->name()));
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName)
{
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper*cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);
}

void MVGProjectWrapper::reloadProjectFromMaya()
{
	// Cameras
	const std::vector<MVGCamera>& cameraList = _project.cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	_cameraList.clear();
	for(; it != cameraList.end(); ++it) {
		_cameraList.append(new MVGCameraWrapper(*it));	
	}
	emit cameraModelChanged();
	
	// TODO : Camera selection
}

