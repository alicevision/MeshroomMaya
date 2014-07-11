#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>

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
	appendLogText("SelectContextButton clicked");
	MVGMayaUtil::activeSelectionContext();
}

void MVGProjectWrapper::onPlaceContextButtonClicked() 
{
	MVGMayaUtil::activeContext();
}

void MVGProjectWrapper::loadProject(const QString& projectDirectoryPath)
{	
	_project.setProjectDirectory(projectDirectoryPath.toStdString());
	if(!_project.load()) {
		LOG_ERROR("An error occured when loading project.");
		appendLogText(QString("An error occured when loading project."));
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
		setCameraToView(cameras[0], _visiblePanelNames[0]);
		setCameraToView(cameras[1], _visiblePanelNames[1]);
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
	
	_panelToCamera[viewName.toStdString()] = cam->camera().dagPath().fullPathName().asChar();
	rebuildCacheFromMaya();
}


DisplayData* MVGProjectWrapper::getCachedDisplayData(M3dView& view)
{
	if(!MVGMayaUtil::isMVGView(view))
		return NULL;
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	std::map<std::string, DisplayData>::iterator it = _cache.find(cameraPath.fullPathName().asChar());
	
	if(it == _cache.end())
		return NULL;
	
	return &(it->second);
}

void MVGProjectWrapper::rebuildCacheFromMaya() 
{
	// Remove unused camera
	std::map<std::string, DisplayData>::iterator cacheIt = _cache.begin();
	while(cacheIt != _cache.end())
	{
		bool isInView = false;
		for(std::map<std::string, std::string>::iterator camIt = _panelToCamera.begin(); camIt != _panelToCamera.end(); ++camIt)
		{
			if(cacheIt->first == camIt->second)
			{
				isInView = true;
				break;
			}
		}
		
		if(!isInView)
		{
			_cache.erase(cacheIt++);
		} 
		else
		{
			++cacheIt;
		}
	}
	
	// Rebuild cache
	for(std::map<std::string, std::string>::iterator camIt = _panelToCamera.begin(); camIt != _panelToCamera.end(); ++camIt)
	{
		MDagPath cameraPath;
		MVGMayaUtil::getDagPathByName(camIt->second.c_str(), cameraPath);
	
		MVGCamera c(cameraPath);
		if(c.isValid()) {
			DisplayData data;
			data.camera = c;
			data.cameraPoints2D = c.getClickedPoints();
			_cache[cameraPath.fullPathName().asChar()] = data;
		}
	}
	
	// TODO : Rebuild maps

}