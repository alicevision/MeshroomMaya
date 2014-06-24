#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>

#include "mayaMVG/qt/MVGUserLog.h"
#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"

namespace mayaMVG {

MVGProjectWrapper::MVGProjectWrapper()
{
	_project = MVGProject(MVGProject::_PROJECT);
	if(!_project.isValid()) {
		_project = MVGProject::create(MVGProject::_PROJECT);
		LOG_INFO("New OpenMVG Project.")
	}
}

MVGProjectWrapper::~MVGProjectWrapper()
{
}

const QString MVGProjectWrapper::moduleDirectory() const
{
	return QString::fromStdString(_project.moduleDirectory());
}

const QString MVGProjectWrapper::projectDirectory() const
{
	return QString::fromStdString(_project.projectDirectory());
}

const QString MVGProjectWrapper::cameraDirectory() const
{
	return QString::fromStdString(_project.cameraDirectory());
}

const QString MVGProjectWrapper::imageDirectory() const
{
	return QString::fromStdString(_project.imageDirectory());
}

const QString MVGProjectWrapper::pointCloudFile() const
{
	return QString::fromStdString(_project.pointCloudFile());
}

const QString MVGProjectWrapper::logText() const
{
	return _logText;
}

void MVGProjectWrapper::setLogText(const QString text)
{
	_logText = text;
	emit logTextChanged();
}

void MVGProjectWrapper::appendLogText(const QString text)
{
	_logText.append(text + "\n");
	emit logTextChanged();
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
	_project.setProjectDirectory(directory.toStdString());
	emit projectDirectoryChanged();
}

void MVGProjectWrapper::addCamera(const MVGCamera& camera)
{
	_cameraList.append(new MVGCameraWrapper(camera));
	emit cameraModelChanged();
}

QString MVGProjectWrapper::openFileDialog() const
{
	MString directoryPath;
	MVGMayaUtil::openFileDialog(directoryPath);	

    return MQtUtil::toQString(directoryPath);
}

void MVGProjectWrapper::onSelectContextButtonClicked() {
	MVGMayaUtil::activeSelectionContext();
}

void MVGProjectWrapper::onPlaceContextButtonClicked() 
{
	MVGMayaUtil::activeContext();
}

void MVGProjectWrapper::loadProject(const QString& projectDirectoryPath)
{	
	_project.setProjectDirectory(projectDirectoryPath.toStdString());
	if(!_project.load())
	{
		USER_ERROR("An error occured when loading project.");
	}
	Q_EMIT projectDirectoryChanged();
	// Populate menu
	const std::vector<MVGCamera>& cameraList = _project.cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	_cameraList.clear();
	for(; it != cameraList.end(); ++it) {
		addCamera(*it);
	}
	// select the two first cameras for the views
	if(_cameraList.size() > 1) {
		QList<MVGCameraWrapper*>& cameras = _cameraList.asQList<MVGCameraWrapper>();
		_project.setCameraInView(cameras[0]->camera(), "mvgLPanel");
		_project.setCameraInView(cameras[1]->camera(), "mvgRPanel");
	}
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
    _project.setCameraInView(cam->camera(), viewName.toStdString());
}

} // namespace mayaMVG 

