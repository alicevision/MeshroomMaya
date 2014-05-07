#include "MVGProjectWrapper.h"

// MayaMVG
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"

using namespace mayaMVG;

MVGProjectWrapper::MVGProjectWrapper()
{
	_project = new MVGProject();
}

MVGProjectWrapper::~MVGProjectWrapper()
{
	delete _project;
}

const QString MVGProjectWrapper::projectDirectory() const
{
	return QString(_project->projectDirectory().c_str());
}

const QString MVGProjectWrapper::cameraDirectory() const
{
	return QString(_project->cameraDirectory().c_str());
}

const QString MVGProjectWrapper::imageDirectory() const
{
	return QString(_project->imageDirectory().c_str());
}

const QList<QObject*>& MVGProjectWrapper::cameraModel() const
{
	return _cameraList;
}
	
QObject* MVGProjectWrapper::getCameraAtIndex(int index) const
{
	return _cameraList.at(index);
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
	_project->setProjectDirectory(directory.toStdString());
	
	emit projectDirectoryChanged();
}

void MVGProjectWrapper::addCamera(const MVGCamera& camera)
{
	_cameraList.append(new MVGCameraWrapper(camera));
	emit cameraModelChanged();
	
}

void MVGProjectWrapper::onBrowseDirectoryButtonClicked()
{
	// TODO : add parent Widget
	QString directory = QFileDialog::getExistingDirectory(NULL, "Choose directory");
						
	if(directory.isEmpty()) {
		LOG_INFO("Directory is empty");
		return;
	}
	
	loadProject(directory);
}

void MVGProjectWrapper::onSelectContextButtonClicked() {
	LOG_INFO("SelectContextButton clicked");
	
}

void MVGProjectWrapper::onPlaceContextButtonClicked() 
{
	LOG_INFO("PlaceContextButton clicked");
	MVGMayaUtil::activeContext();
}
		
void MVGProjectWrapper::onMoveContextButtonClicked()
{
	LOG_INFO("MoveContextButton clicked");
}

void MVGProjectWrapper::loadProject(QString projectDirectoryPath)
{
	if(!_project->load(projectDirectoryPath.toStdString()))
	{
		LOG_ERROR("An error occured when loading project.")
	}
	
	emit projectDirectoryChanged();
	
	// Populate menu
	const std::vector<MVGCamera>& cameraList = _project->cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	
	for(; it != cameraList.end(); ++it) {
		addCamera(*it);
	}
}

void MVGProjectWrapper::selectItems(const QList<QString>& cameraNames)
{
	for(int i = 0; i < _cameraList.size(); ++i)
	{
		dynamic_cast<MVGCameraWrapper*>(getCameraAtIndex(i))->setState("NORMAL");
		if(cameraNames.contains(dynamic_cast<MVGCameraWrapper*>(getCameraAtIndex(i))->name()))
			dynamic_cast<MVGCameraWrapper*>(getCameraAtIndex(i))->setState("SELECTED");
		
		
	}
}

//void MVGProjectWrapper::createProject(std::string projectDirectory, std::string imageDirectoryName, std::string cameraDirectoryName)
//{
//	_project->setProjectDirectory(projectDirectory);
//	_project->setImageDirectoryName(imageDirectoryName);
//	_project->setCameraDirectoryName(cameraDirectoryName);
//}