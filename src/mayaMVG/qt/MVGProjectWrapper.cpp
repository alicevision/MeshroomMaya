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

void MVGProjectWrapper::addCamerasToIHMSelection(const QStringList& selectedCameraNames,
                                                 bool center)
{
    // Reset old selection to false
    for(QStringList::const_iterator it = _selectedCameras.begin(); it != _selectedCameras.end();
        ++it)
    {
        MVGCameraWrapper* camera = _camerasByName[it->toStdString()];
        camera->setIsSelected(false);
    }
    _selectedCameras.clear();
    // Set new selection to true
    for(QStringList::const_iterator it = selectedCameraNames.begin();
        it != selectedCameraNames.end(); ++it)
    {
        if(_camerasByName.count(it->toStdString()) == 0)
            continue;
        MVGCameraWrapper* camera = _camerasByName[it->toStdString()];
        camera->setIsSelected(true);
        _selectedCameras.append(camera->getName());
        // Replace listView and set image in first viewort
        // TODO : let the user define in which viewport he wants to display the selected camera
        if(center && camera->getName() == selectedCameraNames[0])
        {
            setCameraToView(camera, static_cast<MVGPanelWrapper*>(_panelList.get(0))->getName());
            Q_EMIT centerCameraListByIndex(_cameraList.indexOf(camera));
        }
    }
}

void MVGProjectWrapper::addCamerasToMayaSelection(const QStringList& cameraNames) const
{
    if(!_project.isValid())
        return;
    std::vector<std::string> cameras;
    for(QStringList::const_iterator it = cameraNames.begin(); it != cameraNames.end(); ++it)
        cameras.push_back(it->toStdString());
    _project.selectCameras(cameras);
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName, bool rebuildCache)
{
    MVGCameraWrapper* cameraWrapper = static_cast<MVGCameraWrapper*>(camera);

    // Push command
    _project.pushLoadCurrentImagePlaneCommand(viewName.toStdString());

    // Set UI
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper* cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);

    // Update active camera
    _activeCameraNameByView[viewName.toStdString()] = cameraWrapper->getName().toStdString();

    // rebuild cache
    if(rebuildCache)
        MGlobal::executeCommandOnIdle("mayaMVGTool -e -rebuild mayaMVGTool1");
}

void MVGProjectWrapper::setCamerasNear(const double near)
{
    // TODO : undoable ?
    for(std::map<std::string, MVGCameraWrapper*>::const_iterator it = _camerasByName.begin();
        it != _camerasByName.end(); ++it)
        it->second->getCamera().setNear(near);
}
void MVGProjectWrapper::setCamerasFar(const double far)
{
    // TODO : undoable ?
    for(std::map<std::string, MVGCameraWrapper*>::const_iterator it = _camerasByName.begin();
        it != _camerasByName.end(); ++it)
        it->second->getCamera().setFar(far);
}

void MVGProjectWrapper::setCameraLocatorScale(const double scale)
{
    // TODO : undoable ?
    for(std::map<std::string, MVGCameraWrapper*>::const_iterator it = _camerasByName.begin();
        it != _camerasByName.end(); ++it)
        it->second->getCamera().setLocatorScale(scale);
}

void MVGProjectWrapper::scaleScene(const double scaleSize) const
{
    double internalUnit = MDistance::uiToInternal(scaleSize);
    if(!_project.scaleScene(internalUnit))
        LOG_ERROR("Cannot scale scene")
}

void MVGProjectWrapper::clear()
{
    _cameraList.clear();
    _camerasByName.clear();
    _activeCameraNameByView.clear();
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
    _activeCameraNameByView.clear();
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
