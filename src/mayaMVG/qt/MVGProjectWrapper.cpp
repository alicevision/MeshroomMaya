#include "mayaMVG/qt/MVGProjectWrapper.hpp"
#include <QCoreApplication>
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MQtUtil.h>

namespace mayaMVG
{

MVGProjectWrapper::MVGProjectWrapper()
{
    MVGPanelWrapper* leftPanel = new MVGPanelWrapper("mvgLPanel", "Left");
    MVGPanelWrapper* rightPanel = new MVGPanelWrapper("mvgRPanel", "Right");
    _panelList.append(leftPanel);
    _panelList.append(rightPanel);
    // Initialize currentContext
    MString context;
    MVGMayaUtil::getCurrentContext(context);
    _currentContext = context.asChar();

    // Init unit map
    _unitMap[MDistance::kInches] = "in";
    _unitMap[MDistance::kFeet] = "ft";
    _unitMap[MDistance::kYards] = "yd";
    _unitMap[MDistance::kMiles] = "mi";
    _unitMap[MDistance::kMillimeters] = "mm";
    _unitMap[MDistance::kCentimeters] = "cm";
    _unitMap[MDistance::kKilometers] = "km";
    _unitMap[MDistance::kMeters] = "m";
    assert(_unitMap.size() == MDistance::kLast - 1); // First value is MDistance::kInvalid
}

MVGProjectWrapper::~MVGProjectWrapper()
{
}

const QString MVGProjectWrapper::getProjectDirectory() const
{
    if(!_project.isValid())
        return "";
    return QString(_project.getProjectDirectory().c_str());
}

const QString MVGProjectWrapper::getCurrentContext() const
{
    return _currentContext;
}

void MVGProjectWrapper::setCurrentContext(const QString& context)
{
    _currentContext = context;
    Q_EMIT currentContextChanged();
}

const QString MVGProjectWrapper::getCurrentUnit() const
{
    MDistance::Unit currentUnit = MDistance::uiUnit();
    return _unitMap[currentUnit];
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
    if(_project.isValid())
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
    clear();
    std::vector<MVGProject> projects = MVGProject::list();
    if(projects.empty())
        return;
    _project = projects.front();
    reloadMVGCamerasFromMaya();
}

void MVGProjectWrapper::loadNewProject(const QString& projectDirectoryPath)
{
    // Cancel load
    if(projectDirectoryPath.isEmpty())
        return;

    clear();
    activeSelectionContext();
    if(!_project.isValid())
        _project = MVGProject::create(MVGProject::_PROJECT);
    if(!_project.load(projectDirectoryPath.toStdString()))
    {
        setProjectDirectory("");
        LOG_ERROR("An error occured when loading project.");
        return;
    }
    _project.setProjectDirectory(projectDirectoryPath.toStdString());
    Q_EMIT projectDirectoryChanged();

    reloadMVGCamerasFromMaya();

    // Set image planes in Maya takes a lot of time.
    // So we ask Qt to process events (UI) before Maya will freeze the
    // application.
    qApp->processEvents();

    // Select the two first cameras for the views
    if(_cameraList.size() > 1 && _panelList.count() > 1)
    {
        QList<MVGCameraWrapper*>& cameras = _cameraList.asQList<MVGCameraWrapper>();
        setCameraToView(cameras[0], static_cast<MVGPanelWrapper*>(_panelList.get(0))->getName(),
                        false);
        setCameraToView(cameras[1], static_cast<MVGPanelWrapper*>(_panelList.get(1))->getName(),
                        false);
    }
}

void MVGProjectWrapper::selectItems(const QList<QString>& cameraNames) const
{
    foreach(MVGCameraWrapper* camera, _cameraList.asQList<MVGCameraWrapper>())
        camera->setIsSelected(cameraNames.contains(camera->getName()));
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

void MVGProjectWrapper::pushImageInCache(const std::string& cameraName)
{
    std::list<std::string>::iterator camera =
        std::find(_cachedImagePlanes.begin(), _cachedImagePlanes.end(), cameraName);
    if(camera != _cachedImagePlanes.end()) // Camera is already in the list
        return;

    if(_cachedImagePlanes.size() == IMAGE_CACHE_SIZE)
    {
        _camerasByName[_cachedImagePlanes.front()]->getCamera().unloadImagePlane();
        _cachedImagePlanes.pop_front();
    }
    _cachedImagePlanes.push_back(cameraName);
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName, bool rebuildCache)
{
    MVGCameraWrapper* cameraWrapper = static_cast<MVGCameraWrapper*>(camera);

    // Load new active camera
    cameraWrapper->getCamera().loadImagePlane();

    // If new camera is in cache remove from cacheList
    std::list<std::string>::iterator cameraIt =
        std::find(_cachedImagePlanes.begin(), _cachedImagePlanes.end(),
                  cameraWrapper->getName().toStdString());
    if(cameraIt != _cachedImagePlanes.end())
        _cachedImagePlanes.remove(cameraWrapper->getName().toStdString());

    // Add old active camera to cached image list
    std::map<std::string, std::string>::iterator cameraInViewIt =
        _activeCameraNameByView.find(viewName.toStdString());
    if(cameraInViewIt != _activeCameraNameByView.end())
    {
        if(cameraInViewIt->second != cameraWrapper->getName().toStdString())
        {
            const std::string activeCameraName = cameraInViewIt->second;
            if(_camerasByName[activeCameraName]->getViews().size() < 2)
                pushImageInCache(activeCameraName);
        }
    }

    // Set UI
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper* cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);

    // Update active camera   _activeCameraNameByView[viewName.toStdString()] =
    // cameraWrapper->getName().toStdString();

    // rebuild cache
    if(rebuildCache)
        MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");

    //    LOG_INFO(" ### CACHE ### ")
    //    for(std::list<std::string>::iterator it = _cachedImagePlanes.begin(); it !=
    //    _cachedImagePlanes.end(); ++it)
    //        LOG_INFO("* " << *it)
}

void MVGProjectWrapper::scaleScene(const double scaleSize) const
{
    if(!_project.scaleScene(scaleSize))
        LOG_ERROR("Cannot scale scene")
}

void MVGProjectWrapper::clear()
{
    _cameraList.clear();
    _camerasByName.clear();
    while(!_cachedImagePlanes.empty())
        _cachedImagePlanes.pop_back();

    Q_EMIT projectDirectoryChanged();
}

void MVGProjectWrapper::removeCameraFromUI(MDagPath& cameraPath)
{
    MVGCamera camera(cameraPath);
    if(!camera.isValid())
        return;

    for(int i = 0; i < _cameraList.count(); ++i)
    {
        MVGCameraWrapper* cameraWrapper = static_cast<MVGCameraWrapper*>(_cameraList.at(i));
        if(!cameraWrapper)
            continue;
        if(cameraWrapper->getCamera().getName() != camera.getName())
            continue;
        MDagPath leftCameraPath, rightCameraPath;
        MVGMayaUtil::getCameraInView(leftCameraPath, "mvgLPanel");
        leftCameraPath.extendToShape();
        if(leftCameraPath.fullPathName() == cameraPath.fullPathName())
            MVGMayaUtil::clearCameraInView("mvgLPanel");
        MVGMayaUtil::getCameraInView(rightCameraPath, "mvgRPanel");
        rightCameraPath.extendToShape();
        if(rightCameraPath.fullPathName() == cameraPath.fullPathName())
            MVGMayaUtil::clearCameraInView("mvgRPanel");
        // Remove cameraWrapper
        _cameraList.removeAt(i);
    }
}

void MVGProjectWrapper::emitCurrentUnitChanged()
{
    Q_EMIT currentUnitChanged();
}

void MVGProjectWrapper::reloadMVGCamerasFromMaya()
{
    _cameraList.clear();
    _camerasByName.clear();
    if(!_project.isValid())
    {
        LOG_ERROR("Project is not valid");
        return;
    }
    Q_EMIT projectDirectoryChanged();
    const std::vector<MVGCamera>& cameraList = MVGCamera::getCameras();
    std::vector<MVGCamera>::const_iterator it = cameraList.begin();
    for(; it != cameraList.end(); ++it)
    {
        MVGCameraWrapper* cameraWrapper = new MVGCameraWrapper(*it);
        _cameraList.append(cameraWrapper);
        _camerasByName[it->getName()] = cameraWrapper;
    }
    Q_EMIT cameraModelChanged();
    // TODO : Camera selection
}

} // namespace
