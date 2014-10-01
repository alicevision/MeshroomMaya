#include "mayaMVG/qt/MVGProjectWrapper.hpp"
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MQtUtil.h>
#include <QCoreApplication>

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

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName,
                                        bool rebuildCache) const
{
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
        c->setInView(viewName, false);
    MVGCameraWrapper* cam = qobject_cast<MVGCameraWrapper*>(camera);
    cam->setInView(viewName, true);
    // rebuild cache
    if(rebuildCache)
        MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
}

void MVGProjectWrapper::clear()
{
    _cameraList.clear();
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

void MVGProjectWrapper::reloadMVGCamerasFromMaya()
{
    _cameraList.clear();
    if(!_project.isValid())
    {
        LOG_ERROR("Project is not valid");
        return;
    }
    Q_EMIT projectDirectoryChanged();
    const std::vector<MVGCamera>& cameraList = MVGCamera::getCameras();
    std::vector<MVGCamera>::const_iterator it = cameraList.begin();
    for(; it != cameraList.end(); ++it)
        _cameraList.append(new MVGCameraWrapper(*it));
    Q_EMIT cameraModelChanged();
    // TODO : Camera selection
}

} // namespace
