#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <maya/MQtUtil.h>
#include "mayaMVG/core/MVGGeometryUtil.h"

using namespace mayaMVG;

MVGProjectWrapper::MVGProjectWrapper()
{
	_allPanelNames.append("mvgLPanel");
	_allPanelNames.append("mvgRPanel");
	_visiblePanelNames = _allPanelNames;

    // Initialize currentContext
    MString context;
    MVGMayaUtil::getCurrentContext(context);
    _currentContext = context.asChar();
}

MVGProjectWrapper::~MVGProjectWrapper()
{
}

const QString MVGProjectWrapper::projectDirectory() const
{
	if(!_project.isValid())
		return "";
	return QString(_project.projectDirectory().c_str());
}

const QString MVGProjectWrapper::currentContext() const
{
    return _currentContext;
}

void MVGProjectWrapper::setCurrentContext(const QString& context)
{
    _currentContext = context;
    Q_EMIT currentContextChanged();
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
	if(!_project.isValid())
		return;
	_project.setProjectDirectory(directory.toStdString());
	Q_EMIT projectDirectoryChanged();
}

QString MVGProjectWrapper::openFileDialog() const
{
	MString directoryPath;
	MVGMayaUtil::openFileDialog(directoryPath);	

    return MQtUtil::toQString(directoryPath);
}

void MVGProjectWrapper::activeSelectionContext() const
{
	MVGMayaUtil::activeSelectionContext();
}

void MVGProjectWrapper::activeMVGContext() 
{
	MVGMayaUtil::activeContext();
}

void MVGProjectWrapper::loadExistingProject()
{
	_cameraList.clear();
	std::vector<MVGProject> projects = MVGProject::list();
	if(projects.empty())
		return;
	_project = projects.front();
	reloadMVGCamerasFromMaya();
}

void MVGProjectWrapper::loadNewProject(const QString& projectDirectoryPath)
{
    if(!_project.isValid())
        _project = MVGProject::create(MVGProject::_PROJECT);
	if(projectDirectoryPath.isEmpty())
		return;
	if(!_project.load(projectDirectoryPath.toStdString())) {
		LOG_ERROR("An error occured when loading project.");
//		appendLogText(QString("An error occured when loading project."));
		return;
	}
	_project.setProjectDirectory(projectDirectoryPath.toStdString());
	Q_EMIT projectDirectoryChanged();
	
	reloadMVGCamerasFromMaya();
	// Select the two first cameras for the views
	if(_cameraList.size() > 1) {
		QList<MVGCameraWrapper*>& cameras = _cameraList.asQList<MVGCameraWrapper>();
		setCameraToView(cameras[0], _visiblePanelNames[0], false);
		setCameraToView(cameras[1], _visiblePanelNames[1], false);
	}
}

void MVGProjectWrapper::selectItems(const QList<QString>& cameraNames) const
{
    foreach(MVGCameraWrapper* camera, _cameraList.asQList<MVGCameraWrapper>())
        camera->setIsSelected(cameraNames.contains(camera->name()));
}

void MVGProjectWrapper::selectCameras(const QStringList& cameraNames) const
{
	if(!_project.isValid())
		return;
    std::vector<std::string> cameras;
    for(QStringList::const_iterator it = cameraNames.begin(); it != cameraNames.end(); ++it)
        cameras.push_back(it->toStdString());
    _project.selectCameras(cameras);
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName, bool rebuildCache) const
{
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper*cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);
    // rebuild cache
    if(rebuildCache)
    	MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
}

void MVGProjectWrapper::reloadMVGCamerasFromMaya()
{
	_cameraList.clear();
	if(!_project.isValid())
		return;
	Q_EMIT projectDirectoryChanged();
	const std::vector<MVGCamera>& cameraList = _project.cameras();
	std::vector<MVGCamera>::const_iterator it = cameraList.begin();
	for(; it != cameraList.end(); ++it)
		_cameraList.append(new MVGCameraWrapper(*it));
	Q_EMIT cameraModelChanged();
	// TODO : Camera selection
}
