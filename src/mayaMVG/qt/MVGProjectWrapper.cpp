#include "mayaMVG/qt/MVGProjectWrapper.hpp"
#include "mayaMVG/version.hpp"
#include <QCoreApplication>
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include "mayaMVG/qt/MVGMeshWrapper.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/maya/MVGDummyLocator.h"
#include "mayaMVG/maya/MVGCameraPointsLocator.hpp"
#include "mayaMVG/maya/cmd/MVGSelectClosestCamCmd.hpp"
#include "Eigen/src/StlSupport/StdVector.h"
#include <maya/MQtUtil.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MDagPath.h>
#include <maya/MNodeMessage.h>
#include <set>

namespace mayaMVG
{

namespace  // Utility functions
{

/**
 * Give the number of occurrences of each element in the given sets.
 *
 * @param sets the sets to consider
 * @return a map element => count
 */
template <typename T>
std::map<T, int> countElements(const std::vector< std::set<T> >& sets)
{
    std::map<T, int> idWeights;
    for(const auto& set : sets)
    {
        for(const auto& elt : set)
        {
            if(idWeights.find(elt) != idWeights.end())
                idWeights[elt]++;
            else
                idWeights[elt] = 1;
        }
    }
    return idWeights;
}

/**
 * Returns the intersection (common elements) of the given sets.
 *
 * @param sets the sets to consider
 * @return the intersection of all the sets
 */
template <typename T>
std::set<T> setsIntersection(const std::vector< std::set<T> >& sets)
{
    std::set<T> intersection;
    const auto weights = countElements(sets);
    for(const auto& elt : weights)
    {
        if(elt.second > 1)
            intersection.insert(elt.first);
    }
    return intersection;
}

}

MVGProjectWrapper::MVGProjectWrapper():
_cameraPointsLocatorCB(0)
{
    MVGPanelWrapper* leftPanel = new MVGPanelWrapper("mvgLPanel", "Left", MVGMayaUtil::fromMColor(MVGProject::_LEFT_PANEL_DEFAULT_COLOR));
    MVGPanelWrapper* rightPanel = new MVGPanelWrapper("mvgRPanel", "Right", MVGMayaUtil::fromMColor(MVGProject::_RIGHT_PANEL_DEFAULT_COLOR));
    _panelList.append(leftPanel);
    _panelList.append(rightPanel);
    // Initialize currentContext
    MString context;
    MVGMayaUtil::getCurrentContext(context);
    _currentContext = context.asChar();
    _editMode = MVGContext::eEditModeCreate;
    _moveMode = MVGMoveManipulator::eMoveModeNViewTriangulation;

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

    // Init _isProjectLoading
    _isProjectLoading = false;
    _activeSynchro = true;
}

MVGProjectWrapper::~MVGProjectWrapper()
{
    clear();
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

const QString MVGProjectWrapper::getPluginVersion() const
{
    return MAYAMVG_VERSION;
}

void MVGProjectWrapper::setIsProjectLoading(const bool value)
{
    if(value == _isProjectLoading)
        return;
    _isProjectLoading = value;
    Q_EMIT isProjectLoadingChanged();
}
void MVGProjectWrapper::setActiveSynchro(const bool value)
{
    if(value == _activeSynchro)
        return;
    _activeSynchro = value;
    Q_EMIT activeSynchroChanged();
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
    if(_project.isValid())
        _project.setProjectDirectory(directory.toStdString());
    Q_EMIT projectDirectoryChanged();
}

const int MVGProjectWrapper::getCameraPointsDisplayMode() const 
{
    MObject locator;
    MVGMayaUtil::getObjectByName(MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), locator);
    if(locator.isNull())
        return 0;
    int displayMode;
    MVGMayaUtil::getIntAttribute(locator, "mvgDisplayMode", displayMode);
    return displayMode;
}

void MVGProjectWrapper::setCameraPointsDisplayMode(int displayMode)
{
    if(getCameraPointsDisplayMode() == displayMode)
        return;
    MObject locator;
    MVGMayaUtil::getObjectByName(MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), locator);
    MVGMayaUtil::setIntAttribute(locator, "mvgDisplayMode", displayMode, false);
    // Notify signal emitted in onCameraPointsLocatorAttrChanged
}

void onCameraPointsLocatorAttrChanged(MNodeMessage::AttributeMessage msg, MPlug& plug, MPlug& other, void* data)
{
    MVGProjectWrapper* wrapper = static_cast<MVGProjectWrapper*>(data);
    if(msg & MNodeMessage::kAttributeSet)
    {
        if(plug.partialName() == "mvgdm")
            wrapper->emitCameraPointsDisplayModeChanged();
        // Handle left/right color attributes changes
        if(plug.partialName() == "mvglc" || plug.partialName() == "mvgrc")
        {
            const QString viewName = plug.partialName() == "mvglc" ? "mvgLPanel" : "mvgRPanel";
            wrapper->updatePanelColor(viewName);
        }
    }
}

QString MVGProjectWrapper::openFileDialog() const
{
    MString directoryPath;
    MVGMayaUtil::openFileDialog(directoryPath);

    return MQtUtil::toQString(directoryPath);
}

void MVGProjectWrapper::applySceneTransformation() const
{
    _project.applySceneTransformation();
}

void MVGProjectWrapper::activeSelectionContext() const
{
    MVGMayaUtil::activeMayaContext();
}

void MVGProjectWrapper::setCreationMode()
{
    MVGMayaUtil::setCreationMode();
}

void MVGProjectWrapper::setTriangulationMode()
{
    MVGMayaUtil::setTriangulationMode();
}

void MVGProjectWrapper::setPointCloudMode()
{
    MVGMayaUtil::setPointCloudMode();
}

void MVGProjectWrapper::setAdjacentPlaneMode()
{
    MVGMayaUtil::setAdjacentPlaneMode();
}

void MVGProjectWrapper::setLocatorMode()
{
    MVGMayaUtil::setLocatorMode();
}

void MVGProjectWrapper::loadExistingProject()
{
    clear();
    std::vector<MVGProject> projects = MVGProject::list();
    if(projects.empty())
        return;
    _project = projects.front();
    
    initCameraPointsLocator();
    reloadMVGCamerasFromMaya();
    reloadMVGMeshesFromMaya();

    // Retrieve selection
    MDagPath leftCameraPath;
    MVGMayaUtil::getCameraInView(leftCameraPath, "mvgLPanel");
    MString leftCameraXFormName = leftCameraPath.partialPathName();
    leftCameraPath.extendToShape();
    _activeCameraNameByView["mvgLPanel"] = leftCameraPath.fullPathName().asChar();
    _project.setLastLoadedCameraInView("mvgLPanel", leftCameraXFormName.asChar());

    MDagPath rightCameraPath;
    MVGMayaUtil::getCameraInView(rightCameraPath, "mvgRPanel");
    MString rightCameraXFormName = rightCameraPath.partialPathName();
    rightCameraPath.extendToShape();
    _activeCameraNameByView["mvgRPanel"] = rightCameraPath.fullPathName().asChar();
    _project.setLastLoadedCameraInView("mvgRPanel", rightCameraXFormName.asChar());

    // Clear cache
    clearAndUnloadImageCache();

    Q_EMIT projectDirectoryChanged();
}

void MVGProjectWrapper::loadABC(const QString& abcFilePath)
{
    MStatus status;

    // Cancel load
    if(abcFilePath.isEmpty())
        return;

    // Load abc
    MString cmd;
    cmd.format("AbcImport -mode import \"^1s\"", abcFilePath.toStdString().c_str());
    status = MGlobal::executeCommand(cmd);
    CHECK_RETURN(status)

    // Retrieve root node
    MDagPath rootDagPath;
    status = MVGMayaUtil::getDagPathByName(MVGProject::_PROJECT.c_str(), rootDagPath);
    if(!status)
        status = MVGMayaUtil::getDagPathByName(("*:" + MVGProject::_PROJECT).c_str(), rootDagPath);
    CHECK_RETURN(status)

    _project = MVGProject(rootDagPath);
    // Add MVG attribute
    MDagModifier dagModifier;
    MFnTypedAttribute tAttr;
    MObject pathAttr = tAttr.create(MVGProject::_MVG_PROJECTPATH, "mp", MFnData::kString);
    dagModifier.addAttribute(rootDagPath.node(), pathAttr);
    dagModifier.doIt();
    setProjectDirectory(abcFilePath);

    // Cameras group node
    MObject cameraParent = rootDagPath.child(0);
    MDagPath cameraGroupPath;
    MDagPath::getAPathTo(cameraParent, cameraGroupPath);
    // Cloud group node
    MObject cloudParent = rootDagPath.child(1);
    MDagPath cloudGroupPath;
    MDagPath::getAPathTo(cloudParent, cloudGroupPath);

    // Camera points locator
    initCameraPointsLocator();

    // Point cloud
    if(cloudGroupPath.childCount() == 0)
    {
        LOG_ERROR("Can't find point cloud in MVG hierarchy")
        return;
    }
    MDagPath pointCloudDagPath;
    MDagPath::getAPathTo(cloudGroupPath.child(0), pointCloudDagPath);
    pointCloudDagPath.extendToShape();
    MObject pointCloud = pointCloudDagPath.node();

    // Visibility
    MIntArray visibilitySizeArray;
    status =
        MVGMayaUtil::getIntArrayAttribute(pointCloud, "mvg_visibilitySize", visibilitySizeArray);
    CHECK_RETURN(status)
    MIntArray visibilityIDsArray;
    status = MVGMayaUtil::getIntArrayAttribute(pointCloud, "mvg_visibilityIds", visibilityIDsArray);
    CHECK_RETURN(status)

    int k = 0;
    std::map<int, MIntArray> itemsPerCam;
    // Browse 3D points
    for(int j = 0; j < visibilitySizeArray.length(); ++j)
    {
        int nbView = visibilitySizeArray[j];
        int lastPosition = k + (nbView - 1) * 2;
        // Browse visibility
        for(; k < lastPosition + 1; k += 2)
        {
            int viewID = visibilityIDsArray[k];
            itemsPerCam[viewID].append(j);
        }
    }

    // Cameras
    if(cameraGroupPath.childCount() == 0)
    {
        LOG_ERROR("Can't find cameras in MVG hierarchy")
        return;
    }
    for(int i = 0; i < cameraGroupPath.childCount(); ++i)
    {
        MDagPath cameraDagPath;
        MDagPath::getAPathTo(cameraGroupPath.child(i), cameraDagPath);
        MVGCamera::create(cameraDagPath, itemsPerCam);
    }

    // Set images paths
    cmd.format("from mayaMVG import camera;\n"
               "camera.setImagesPaths('^1s', '^2s', '^3s', '^4s', '^5s')",
               abcFilePath.toStdString().c_str(), MVGCamera::_MVG_IMAGE_PATH.asChar(),
               MVGCamera::_MVG_IMAGE_SOURCE_PATH.asChar(), MVGCamera::_MVG_THUMBNAIL_PATH.asChar(),
               MVGCamera::_MVG_VIEW_ID.asChar());
    MGlobal::executePythonCommand(cmd);

    _project.lockProject();

    // Update view
    reloadMVGCamerasFromMaya();
}

void MVGProjectWrapper::remapPaths(const QString& abcFilePath)
{
    MStatus status;
    MString cmd;
    cmd.format("camera.mapImagesPaths('^1s', '^2s', '^3s')", MVGCamera::_MVG_IMAGE_PATH.asChar(),
               MVGCamera::_MVG_THUMBNAIL_PATH.asChar(), abcFilePath.toStdString().c_str());
    MGlobal::executePythonCommand("from mayaMVG import camera");
    status = MGlobal::executePythonCommand(cmd);
    setProjectDirectory(abcFilePath);
    CHECK(status)
}

/**
 *
 * @param[in] selectedCameraNames Names list of the cameras to add to IHM selection
 * @param[in] center Boolean indicating if we center the camera list on the new selection
 */
void MVGProjectWrapper::addCamerasToIHMSelection(const QStringList& selectedCameraNames,
                                                 bool center)
{
    // Reset old selection to false
    clearCameraSelection();
    // Set new selection to true
    for(QStringList::const_iterator it = selectedCameraNames.begin();
        it != selectedCameraNames.end(); ++it)
    {
        if(_camerasByName.count(it->toStdString()) == 0)
            continue;
        MVGCameraWrapper* camera = _camerasByName[it->toStdString()];
        camera->setIsSelected(true);
        _selectedCameras.append(*it);
        // Replace listView and set image in first viewort
        // TODO : let the user define in which viewport he wants to display the selected camera
        if(center && camera->getDagPathAsString() == selectedCameraNames[0])
        {
            setCameraToView(camera, static_cast<MVGPanelWrapper*>(_panelList.get(0))->getName());
            Q_EMIT centerCameraListByIndex(_cameraList.indexOf(camera));
        }
    }
}

/**
 *
 * @param[in] cameraNames Names list of the cameras to add to IHM selection
 */
void MVGProjectWrapper::addCamerasToMayaSelection(const QStringList& cameraNames) const
{
    if(!_project.isValid())
        return;
    std::vector<std::string> cameras;
    for(QStringList::const_iterator it = cameraNames.begin(); it != cameraNames.end(); ++it)
        cameras.push_back(it->toStdString());
    _project.selectCameras(cameras);
}

void MVGProjectWrapper::addMeshesToIHMSelection(const QStringList& selectedMeshPaths, bool center)
{
    // Reset old selection to false
    clearMeshSelection();
    // Set new selection to true
    for(QStringList::const_iterator it = selectedMeshPaths.begin(); it != selectedMeshPaths.end();
        ++it)
    {
        if(_meshesByName.count(it->toStdString()) == 0)
            continue;
        MVGMeshWrapper* mesh = _meshesByName[it->toStdString()];
        mesh->setIsSelected(true);
        _selectedMeshes.append(mesh->getDagPathAsString());
        // Replace listView
        // TODO : let the user define in which viewport he wants to display the selected camera
        if(center && mesh->getDagPathAsString() == selectedMeshPaths[0])
            Q_EMIT centerMeshListByIndex(_meshesList.indexOf(mesh));
    }
}

void MVGProjectWrapper::addMeshesToMayaSelection(const QStringList& meshesPath) const
{
    if(!_project.isValid())
        return;
    std::vector<std::string> meshes;
    for(QStringList::const_iterator it = meshesPath.begin(); it != meshesPath.end(); ++it)
        meshes.push_back(it->toStdString());
    _project.selectMeshes(meshes);
}

void MVGProjectWrapper::selectClosestCam() const
{
    MGlobal::executeCommand(MVGSelectClosestCamCmd::_name);
}

void MVGProjectWrapper::setCameraToView(QObject* camera, const QString& viewName)
{
    MVGCameraWrapper* cameraWrapper = static_cast<MVGCameraWrapper*>(camera);

    // Push command
    _project.pushLoadCurrentImagePlaneCommand(viewName.toStdString());
    // Set UI
    foreach(MVGCameraWrapper* c, _cameraList.asQList<MVGCameraWrapper>())
    {
        if(c->isInView(viewName))
        {
            c->setInView(viewName, false);
            c->getCamera().setLocatorCustomColor(false);
        }
    }
    // Update active camera
    _activeCameraNameByView[viewName.toStdString()] = cameraWrapper->getDagPathAsString().toStdString();

    // Update data from new configuration
    for(const auto& camByView : _activeCameraNameByView)
    {
        const QString view = QString::fromStdString(camByView.first);
        MVGCameraWrapper* camWrapper = cameraFromViewName(view);
        if(!camWrapper)
            return;
        camWrapper->setInView(view, true);
        const MColor color = MVGMayaUtil::fromQColor(panelFromViewName(view)->getColor());
        camWrapper->getCamera().setLocatorCustomColor(true, color);
    }

    updatePointsVisibility();
}

void MVGProjectWrapper::initCameraPointsLocator() 
{
    MObject cpLocator;
    MStatus status;
    MVGMayaUtil::getObjectByName(MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), cpLocator);
    // If the locator does not exist, create it
    if(cpLocator.isNull())
    {
        status = MVGMayaUtil::addLocator("MVGCameraPointsLocator", MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), _project.getObject(), cpLocator);
        CHECK_RETURN(status);
    }
    _cameraPointsLocatorCB = MNodeMessage::addAttributeChangedCallback(cpLocator, onCameraPointsLocatorAttrChanged, (void*)this);
}

void MVGProjectWrapper::updatePointsVisibility()
{
    std::vector< std::set<int> > pointsSets;
    pointsSets.reserve(_activeCameraNameByView.size());
    std::map< std::string, std::set<int>* > pointsPerCamera;
    for(const auto& camByView : _activeCameraNameByView)
    {
        MVGCameraWrapper* camWrapper = cameraFromViewName(QString::fromStdString(camByView.first));
        if(!camWrapper)
            return;
        std::set<int> visibility;
        MIntArray visibleIndexes;
        camWrapper->getCamera().getVisibleIndexes(visibleIndexes);
        for(int i = 0; i < visibleIndexes.length(); ++i)
            visibility.insert(visibleIndexes[i]);
        pointsSets.push_back(visibility);
        // pointsSets won't be resized (because reserved);
        // we can use pointers to avoid data duplication
        pointsPerCamera[camByView.second] = &pointsSets.back();
    }

    // Remove common points from individual camera points lists
    // to avoid z-fighting when drawing them
    const std::set<int> intersection = setsIntersection(pointsSets);
    for(const auto& cameraPoints : pointsPerCamera)
        for(const auto& point : intersection)
            cameraPoints.second->erase(point);

    MObject locator;
    MStatus status;
    status = MVGMayaUtil::getObjectByName(MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), locator);
    CHECK_RETURN(status)
    MDagPath locatorPath;
    status = MDagPath::getAPathTo(locator, locatorPath);
    CHECK_RETURN(status)

    std::vector<MVGPointCloudItem> allPoints;
    MVGPointCloud pointCloud(MVGProject::_CLOUD);
    pointCloud.getItems(allPoints);

    // PointCloudItem positions are in world space;
    // multiply them by the locator inverse matrix to be independent from the locator transform
    const MMatrix locatorInverseMatrix = locatorPath.inclusiveMatrixInverse().transpose();

    // Fill locator points attributes (based on panel name)
    // TODO: make it more generic
    for(const auto& camByView : _activeCameraNameByView)
    {
        const std::string& camName = camByView.second;
        if(camName.empty())
            return;
        const std::string& attrName = camByView.first + "Points";
        const auto& cameraPoints = *(pointsPerCamera[camName]);
        MPointArray array;
        for(const auto& point : cameraPoints)
            array.append(locatorInverseMatrix * allPoints[point]._position);
        MVGMayaUtil::setPointArrayAttribute(locator, attrName.c_str(), array);
    }

    { // Common points
        MPointArray array;
        for(const auto& point : intersection)
            array.append(locatorInverseMatrix * allPoints[point]._position);
        MVGMayaUtil::setPointArrayAttribute(locator, "mvgCommonPoints", array);
    }
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

void MVGProjectWrapper::setCamerasDepth(const double depth)
{
    for(std::map<std::string, MVGCameraWrapper*>::const_iterator it = _camerasByName.begin();
        it != _camerasByName.end(); ++it)
        it->second->getCamera().setImagePlaneDepth(depth);
}

void MVGProjectWrapper::setCameraLocatorScale(const double scale)
{
    // TODO : undoable ?
    for(std::map<std::string, MVGCameraWrapper*>::const_iterator it = _camerasByName.begin();
        it != _camerasByName.end(); ++it)
        it->second->getCamera().setLocatorScale(scale);
}

void MVGProjectWrapper::clearAllBlindData()
{
    // Retrieve all meshes
    std::vector<MVGMesh> meshes = MVGMesh::listAllMeshes();
    for(std::vector<MVGMesh>::iterator it = meshes.begin(); it != meshes.end(); ++it)
        it->unsetAllBlindData();

    // Update cache
    MString cmd;
    cmd.format("^1s -e -rebuild ^2s", MVGContextCmd::name, MVGContextCmd::instanceName);
    MGlobal::executeCommand(cmd);
}

void MVGProjectWrapper::clear()
{
    _cameraList.clear();
    _camerasByName.clear();
    _activeCameraNameByView.clear();
    _selectedCameras.clear();
    _meshesList.clear();
    _meshesByName.clear();
    _selectedMeshes.clear();
    
    if(_cameraPointsLocatorCB)
        MNodeMessage::removeCallback(_cameraPointsLocatorCB);
}

void MVGProjectWrapper::clearAndUnloadImageCache()
{
    std::vector<std::string> activeCameras;
    std::map<std::string, std::string>::iterator it = _activeCameraNameByView.begin();
    for(it; it != _activeCameraNameByView.end(); ++it)
        activeCameras.push_back(it->second);

    const std::vector<MVGCamera> cameras = MVGCamera::getCameras();
    for(std::vector<MVGCamera>::const_iterator cameraIt = cameras.begin();
        cameraIt != cameras.end(); ++cameraIt)
    {
        std::string cameraName = cameraIt->getDagPath().fullPathName().asChar();
        std::vector<std::string>::iterator cameraFound =
            std::find(activeCameras.begin(), activeCameras.end(), cameraName);
        if(cameraFound == activeCameras.end())
            cameraIt->unloadImagePlane();
    }

    _project.clearImageCache();
}

void MVGProjectWrapper::clearCameraSelection()
{
    for(QStringList::const_iterator it = _selectedCameras.begin(); it != _selectedCameras.end();
        ++it)
    {
        std::map<std::string, MVGCameraWrapper*>::const_iterator foundIt =
            _camerasByName.find(it->toStdString());
        if(foundIt != _camerasByName.end())
            foundIt->second->setIsSelected(false);
    }
    _selectedCameras.clear();
}

void MVGProjectWrapper::clearMeshSelection()
{
    for(QStringList::const_iterator it = _selectedMeshes.begin(); it != _selectedMeshes.end(); ++it)
    {
        std::map<std::string, MVGMeshWrapper*>::const_iterator foundIt =
            _meshesByName.find(it->toStdString());
        if(foundIt != _meshesByName.end())
            foundIt->second->setIsSelected(false);
    }
    _selectedMeshes.clear();
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
void MVGProjectWrapper::addMeshToUI(const MDagPath& meshPath)
{
    MVGMesh mesh(meshPath);

    MVGMeshWrapper* meshWrapper = new MVGMeshWrapper(mesh);
    _meshesList.append(meshWrapper);
}

void MVGProjectWrapper::removeMeshFromUI(const MDagPath& meshPath)
{
    MVGMesh mesh(meshPath);
    if(!meshPath.isValid())
        return;

    for(int i = 0; i < _meshesList.count(); ++i)
    {
        MVGMeshWrapper* meshWraper = static_cast<MVGMeshWrapper*>(_meshesList.at(i));
        if(!meshWraper)
            continue;
        if(meshWraper->getMesh().getName() != mesh.getName())
            continue;
        _meshesList.removeAt(i);
    }
}

void MVGProjectWrapper::emitCurrentUnitChanged()
{
    Q_EMIT currentUnitChanged();
}

void MVGProjectWrapper::emitCameraPointsDisplayModeChanged()
{
    Q_EMIT cameraPointsDisplayModeChanged();
}

void MVGProjectWrapper::setEditMode(const int mode)
{
    if(_editMode == mode)
        return;
    _editMode = mode;
    Q_EMIT editModeChanged();
}

void MVGProjectWrapper::setMoveMode(const int mode)
{
    if(_moveMode == mode)
        return;
    _moveMode = mode;
    Q_EMIT moveModeChanged();
}

void MVGProjectWrapper::reloadMVGCamerasFromMaya()
{
    _cameraList.clear();
    _camerasByName.clear();
    _activeCameraNameByView.clear();

    const std::vector<MVGCamera>& cameraList = MVGCamera::getCameras();
    std::vector<MVGCamera>::const_iterator it = cameraList.begin();
    for(; it != cameraList.end(); ++it)
    {
        MVGCameraWrapper* cameraWrapper = new MVGCameraWrapper(*it);
        _cameraList.append(cameraWrapper);
        _camerasByName[it->getDagPathAsString()] = cameraWrapper;
    }
    // TODO : Camera selection
}

void MVGProjectWrapper::updatePanelColor(const QString& viewName)
{
    // Update panel's color
    MVGPanelWrapper* panel = panelFromViewName(viewName);
    panel->onColorAttributeChanged();
    // Update camera locator's color if any
    MVGCameraWrapper* camWrapper = cameraFromViewName(viewName);
    if(!camWrapper)
        return;
    const MColor color = MVGMayaUtil::fromQColor(panelFromViewName(viewName)->getColor());
    camWrapper->getCamera().setLocatorCustomColor(true, color);
}

MVGCameraWrapper* MVGProjectWrapper::cameraFromViewName(const QString& viewName)
{
    const std::string camName = _activeCameraNameByView[viewName.toStdString()];
    if(camName.empty())
        return NULL;
    return _camerasByName[camName];
}

MVGPanelWrapper* MVGProjectWrapper::panelFromViewName(const QString& viewName)
{
    foreach(MVGPanelWrapper* p, _panelList.asQList<MVGPanelWrapper>())
        if(p->getName() == viewName)
            return p;
    return NULL;
}

void MVGProjectWrapper::reloadMVGMeshesFromMaya()
{
    _meshesList.clear();
    const std::vector<MVGMesh>& meshList = MVGMesh::listAllMeshes();
    std::vector<MVGMesh>::const_iterator it = meshList.begin();
    for(; it != meshList.end(); ++it)
    {
        MVGMeshWrapper* meshWrapper = new MVGMeshWrapper(*it);
        _meshesList.append(meshWrapper);
        _meshesByName[it->getDagPathAsString()] = meshWrapper;
    }
}

} // namespace
