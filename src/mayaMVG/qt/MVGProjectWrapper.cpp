#include "mayaMVG/qt/MVGProjectWrapper.hpp"
#include "mayaMVG/version.hpp"
#include <QCoreApplication>
#include "MVGCameraSetWrapper.hpp"
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
#include <maya/MFnCamera.h>
#include <maya/MDagPath.h>
#include <maya/MNodeMessage.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MObjectSetMessage.h>
#include <maya/MDagModifier.h>

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

MVGProjectWrapper::MVGProjectWrapper(QObject* parent):
QObject(parent),
_currentCameraSetId(0),
_particleSelectionAccuracy(25),
_defaultCameraSet(new MVGCameraSetWrapper("- ALL -", this)),
_currentCameraSet(_defaultCameraSet),
_particleSelectionCameraSet(nullptr),
_cameraPointsLocatorCB(0)
{
    MVGPanelWrapper* leftPanel = new MVGPanelWrapper("mvgLPanel", "Left", MVGMayaUtil::fromMColor(MVGProject::_LEFT_PANEL_DEFAULT_COLOR), this);
    MVGPanelWrapper* rightPanel = new MVGPanelWrapper("mvgRPanel", "Right", MVGMayaUtil::fromMColor(MVGProject::_RIGHT_PANEL_DEFAULT_COLOR), this);
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

    // Force re-evaluation of current camera set index whenever the cameraSet model is modified
    connect(&_cameraSets, SIGNAL(countChanged()), this, SIGNAL(currentCameraSetIndexChanged()));
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

bool MVGProjectWrapper::useParticleSelection() const
{
    return _particleSelectionCameraSet && _currentCameraSet == _particleSelectionCameraSet;
}

void MVGProjectWrapper::setProjectDirectory(const QString& directory)
{
    if(_project.isValid())
        _project.setProjectDirectory(directory.toStdString());
    Q_EMIT projectDirectoryChanged();
}

int MVGProjectWrapper::getCameraPointsDisplayMode() const
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

int MVGProjectWrapper::getCurrentCameraSetIndex() const
{
    if(!_currentCameraSet)
        return -1;
    return _cameraSets.indexOf(_currentCameraSet);
}

void MVGProjectWrapper::setCurrentCameraSetIndex(int idx)
{
    // Exclude invalid indices
    if(idx < 0 || idx >= _cameraSets.count())
        return;
    setCurrentCameraSet(_cameraSets.asQList<MVGCameraSetWrapper>().at(idx));
}

void MVGProjectWrapper::setUseParticleSelection(bool value)
{
    if(value == useParticleSelection())
        return;

    static const QString particleSetName = "- PARTICLE SELECTION -";
    if(value)
    {
        // Create a temporary set for particle selection
        _particleSelectionCameraSet = new MVGCameraSetWrapper(particleSetName);
        _cameraSets.append(_particleSelectionCameraSet);
        // Use selection set as current set
        setCurrentCameraSet(_particleSelectionCameraSet);
        MString cmd;
        cmd.format("select \"^1s\"; selectType -pr true; selectMode -component", MVGProject::_CLOUD.c_str());
        MGlobal::executeCommand(cmd);
        updateCamerasFromParticleSelection(true);
    }
    else if(_particleSelectionCameraSet)
    {
        if(_currentCameraSet == _particleSelectionCameraSet)
        {
            _particleSelectionCameraSet->highlightLocators(false);
            _currentCameraSet = nullptr;
            setCurrentCameraSet(_defaultCameraSet);
        }
        _cameraSets.remove(_particleSelectionCameraSet);
        _particleSelectionCameraSet->deleteLater();
        _particleSelectionCameraSet = nullptr;
        // Restore Maya selection mode
        MGlobal::setSelectionMode(MGlobal::kSelectObjectMode);
        // Re-select the cloud to trigger an update in 3D viewport
        // (makes sure particles don't look selectable (blue) anymore)
        MGlobal::selectByName(MVGProject::_CLOUD.c_str(), MGlobal::kReplaceList);
    }

    Q_EMIT useParticleSelectionChanged();
}

void MVGProjectWrapper::updateParticleSelection(const std::set<int>& selection)
{
    if(!useParticleSelection())
        return;

    if(selection == _particleSelection)
        return;

    _particleSelection = selection;
    _selectionScorePerCamera.clear();

    for(const auto& pointId : _particleSelection)
    {
        if(!_camerasPerPoint.count(pointId))
            continue;
        for(auto* camWrapper : _camerasPerPoint[pointId])
        {
            if(!_selectionScorePerCamera.count(camWrapper))
                _selectionScorePerCamera[camWrapper] = 0;
            _selectionScorePerCamera[camWrapper]++;
        }
    }
    Q_EMIT particleSelectionCountChanged();
    updateCamerasFromParticleSelection(true);
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
            const auto idx = _currentCameraSet->getCameras()->indexOf(camera);
            if(idx >= 0)
                Q_EMIT centerCameraListByIndex(idx);
        }
    }
    Q_EMIT cameraSelectionCountChanged();
}

/**
 *
 * @param[in] cameraNames Names list of the cameras to add to IHM selection
 */
void MVGProjectWrapper::addCamerasToMayaSelection(const QStringList& cameraNames)
{
    if(!_project.isValid())
        return;
    std::vector<std::string> cameras;
    for(const auto& camName : cameraNames)
        cameras.push_back(camName.toStdString());

    if(useParticleSelection())
    {
        // Remove only cameras from the active selection list
        // to keep the particle selection
        MSelectionList currentSelection;
        MGlobal::getActiveSelectionList(currentSelection);
        MItSelectionList it(currentSelection, MFn::kCamera);

        MSelectionList camList;
        MDagPath p;
        for( ; !it.isDone(); it.next()) {
            it.getDagPath(p);
            camList.add(p);
        }
        currentSelection.merge(camList, MSelectionList::kRemoveFromList);
        MGlobal::setActiveSelectionList(currentSelection);
        // Select the cameras in the UI to avoid 'centerCameraListByIndex' emission
        addCamerasToIHMSelection(cameraNames, false);
        // Add cameras to the active selection
        _project.selectCameras(cameras, MGlobal::kAddToHeadOfList);
    }
    else
    {
        _project.selectCameras(cameras);
    }
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

void MVGProjectWrapper::deleteCameraSet(MVGCameraSetWrapper* setWrapper)
{
    // Use Maya delete command to make it undoable
    MString cmd;
    cmd.format("delete ^1s;", setWrapper->fnSet().name());
    MGlobal::executeCommand(cmd, false, true);
}

void MVGProjectWrapper::setCameraToView(MVGCameraWrapper* cameraWrapper, const QString& viewName)
{
    // Push command
    _project.pushLoadCurrentImagePlaneCommand(viewName.toStdString());
    // Set UI
    for(const auto& cam : _camerasByName)
    {
        MVGCameraWrapper* camWrapper = cam.second;
        if(camWrapper->isInView(viewName))
        {
            camWrapper->setInView(viewName, false);
            camWrapper->getCamera().setLocatorCustomColor(false);
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

void MVGProjectWrapper::setPerspFromCamera(MVGCameraWrapper *wrapper)
{
    // Retrieve "persp" cam transform
    MDagPath perspPath;
    CHECK_RETURN(MVGMayaUtil::getDagPathByName("persp", perspPath));
    MFnTransform perspTransform(perspPath);
    // Apply transform from the given camera
    perspTransform.set(wrapper->getCamera().getDagPath().inclusiveMatrix());
}

void MVGProjectWrapper::swapViews()
{
    MVGCameraWrapper* lPanelCam = cameraFromViewName("mvgLPanel");
    MVGCameraWrapper* rPanelCam = cameraFromViewName("mvgRPanel");
    setCameraToView(rPanelCam, "mvgLPanel");
    setCameraToView(lPanelCam, "mvgRPanel");
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
    _cameraPointsLocatorCB = MNodeMessage::addAttributeChangedCallback(cpLocator, onCameraPointsLocatorAttrChanged,
                                                                       static_cast<void*>(this));
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
        for(unsigned int i = 0; i < visibleIndexes.length(); ++i)
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

void MVGProjectWrapper::selectCamerasPoints()
{
    std::set<int> points;
    MIntArray array;
    for(const auto& camName : _selectedCameras)
    {
        _camerasByName[camName.toStdString()]->getCamera().getVisibleIndexes(array);
        for(unsigned int i=0; i<array.length(); ++i)
            points.insert(array[i]);
    }
    // Activate particle selection mode
    setUseParticleSelection(true);
    MVGMayaUtil::selectParticles(MVGProject::_CLOUD.c_str(), points);
}

void MVGProjectWrapper::createCameraSetFromSelection(const QString& name, bool makeCurrent)
{
    createCameraSetFromDagPaths(name, _selectedCameras, makeCurrent);
}

void MVGProjectWrapper::duplicateCameraSet(const QString& copyName, MVGCameraSetWrapper* sourceSet, bool makeCurrent)
{
    auto cameraList = sourceSet->getCameras()->asQList<MVGCameraWrapper>();
    QStringList dagPaths;
    for(auto* wrapper : cameraList)
        dagPaths.append(wrapper->getDagPathAsString());
    createCameraSetFromDagPaths(copyName, dagPaths, makeCurrent);
}

void MVGProjectWrapper::createCameraSetFromDagPaths(const QString& name, const QStringList& paths, bool makeCurrent)
{
    MSelectionList l;
    MFnSet set;

    for(const auto& dagPath : paths)
        l.add(dagPath.toStdString().c_str());

    // Create the set
    MObject setObj = set.create(l, MFnSet::kNone, false);
    set.setName((MVGProject::_CAMERASET_PREFIX + name.toStdString()).c_str());
    set.setAnnotation("MayaMVG Camera Set");

    // Store currently selected points ids as a dynamic attribute
    MFnTypedAttribute attr;
    MObject attrObj = attr.create("mvg_pointSelectionIds", "mvg_pids", MFnData::kPointArray);
    attr.setStorable(true);
    set.addAttribute(attrObj);
    MIntArray arr;
    for(const auto& idx : _particleSelection)
        arr.append(idx);
    MVGMayaUtil::setIntArrayAttribute(setObj, "mvg_pointSelectionIds", arr);

    // The set is created with a name by default
    // MVGMayaCallbacks::setAddedCB does not consider it as a mayaMVG camera set
    // So handle this manually here.
    addCameraSetToUI(setObj, makeCurrent);
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
    _currentCameraSet->highlightLocators(false);

    _camerasByName.clear();
    _activeCameraNameByView.clear();
    clearCameraSelection();

    _cameraSetsByName.clear();
    _cameraSets.clear();

    _meshesByName.clear();
    _meshesList.clear();
    _selectedMeshes.clear();

    if(_cameraPointsLocatorCB)
        MNodeMessage::removeCallback(_cameraPointsLocatorCB);

    for(auto& cb : _nodeCallbacks)
        MNodeMessage::removeCallbacks(cb.second);
    _nodeCallbacks.clear();
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
    Q_EMIT cameraSelectionCountChanged();
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

void MVGProjectWrapper::removeCameraFromUI(MObject& camera)
{
    MFnCamera fnCam(camera);
    MDagPath path;
    fnCam.getPath(path);
    const std::string camName = path.fullPathName().asChar();
    // Remove callbacks on this node
    MMessage::removeCallbacks(_nodeCallbacks[camName]);
    _nodeCallbacks.erase(camName);

    auto it = _camerasByName.find(camName);
    if(it == _camerasByName.end())
        return;
    _camerasByName.erase(camName);

    auto* wrapper = it->second;
    // Remove all occurences of the wrapper in the camera sets
    for (MVGCameraSetWrapper* setWrapper : _cameraSets.asQList<MVGCameraSetWrapper>())
    {
        const int idx = setWrapper->getCameras()->indexOf(wrapper);
        if(idx >= 0)
            setWrapper->getCameras()->removeAt(idx);
    }

    // Clear the views if needed
    MDagPath leftCameraPath, rightCameraPath;
    MVGMayaUtil::getCameraInView(leftCameraPath, "mvgLPanel");
    leftCameraPath.extendToShape();
    if(leftCameraPath.fullPathName() == camName.c_str())
        MVGMayaUtil::clearCameraInView("mvgLPanel");
    MVGMayaUtil::getCameraInView(rightCameraPath, "mvgRPanel");
    rightCameraPath.extendToShape();
    if(rightCameraPath.fullPathName() == camName.c_str())
        MVGMayaUtil::clearCameraInView("mvgRPanel");
}

void MVGProjectWrapper::addMeshToUI(const MDagPath& meshPath)
{
    MVGMesh mesh(meshPath);

    MVGMeshWrapper* meshWrapper = new MVGMeshWrapper(mesh);
    _meshesByName[meshWrapper->getMesh().getName()] = meshWrapper;
    _meshesList.append(meshWrapper); // model takes ownership
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
        _meshesByName.erase(mesh.getName());
    }
}

void MVGProjectWrapper::addCameraSetToUI(MObject& set, bool makeCurrent)
{
    MVGCameraSetWrapper* wrapper = new MVGCameraSetWrapper(set);
    _cameraSetsByName[wrapper->fnSet().name().asChar()] = wrapper;
    _cameraSets.append(wrapper); // model takes ownership
    updateCameraSetWrapperMembers(set);
    if(makeCurrent)
        setCurrentCameraSet(wrapper);

    MCallbackIdArray callbacks;
    // Update camera set wrapper's members when the underlying Maya set is modified
    MCallbackId cbId = MObjectSetMessage::addSetMembersModifiedCallback(set, [](MObject& node, void* projectWrapper) {
        auto* project = static_cast<MVGProjectWrapper*>(projectWrapper);
        project->updateCameraSetWrapperMembers(node);
    }, static_cast<void*>(this));
    callbacks.append(cbId);

    cbId = MNodeMessage::addNodePreRemovalCallback(set, [](MObject& node, void* projectWrapper) {
        auto* project = static_cast<MVGProjectWrapper*>(projectWrapper);
        project->removeCameraSetFromUI(node);
    }, static_cast<void*>(this));

    callbacks.append(cbId);
    _nodeCallbacks[wrapper->fnSet().name().asChar()] = callbacks;
}

void MVGProjectWrapper::removeCameraSetFromUI(MObject& set)
{
    MFnSet fnSet(set);
    // Remove callbacks on this node
    MMessage::removeCallbacks(_nodeCallbacks[fnSet.name().asChar()]);
    _nodeCallbacks.erase(fnSet.name().asChar());

    // Delete UI wrapper
    const std::string setName = fnSet.name().asChar();
    auto* wrapper = _cameraSetsByName[setName];
    if(wrapper == _currentCameraSet)
        setCurrentCameraSetIndex(getCurrentCameraSetIndex()-1);
    _cameraSetsByName.erase(setName);
    getCameraSets()->remove(wrapper);
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
    _camerasByName.clear();
    _activeCameraNameByView.clear();
    _camerasPerPoint.clear();
    _cameraSetsByName.clear();
    _cameraSets.clear();
    _selectionScorePerCamera.clear();

    const std::vector<MVGCamera>& cameraList = MVGCamera::getCameras();
    QObjectList camWrappers;
    for(const auto& camera : cameraList)
    {
        MVGCameraWrapper* cameraWrapper = new MVGCameraWrapper(camera);
        camWrappers.append(cameraWrapper);
        _camerasByName[camera.getDagPathAsString()] = cameraWrapper;
        MIntArray indices;
        camera.getVisibleIndexes(indices);
        for(unsigned int i = 0; i < indices.length(); ++i)
        {
            if(!_camerasPerPoint.count(indices[i]))
                _camerasPerPoint.emplace(indices[i], std::vector<MVGCameraWrapper*>());
            _camerasPerPoint[indices[i]].push_back(cameraWrapper);
        }
        MObject cam = camera.getObject();
        // Lock cam node to avoid manipulation errors
        MFnDagNode dagCam(cam);
        dagCam.setLocked(true);
        MCallbackId cbId = MNodeMessage::addNodePreRemovalCallback(cam, [](MObject& node, void* projectWrapper) {
            auto* project = static_cast<MVGProjectWrapper*>(projectWrapper);
            project->removeCameraFromUI(node);
        }, static_cast<void*>(this));
        _nodeCallbacks[camera.getName()].append(cbId);
    }
    // TODO : Camera selection

    // Camera Sets
    {
    // - default set with all cams
    _defaultCameraSet->setCameraWrappers(camWrappers); // model takes ownership of camera wrappers
    _cameraSets.append(_defaultCameraSet);
    setCurrentCameraSet(_defaultCameraSet);
    // - sets from maya scene
    std::vector<MObject> sets = MVGProject::getMVGCameraSets();
    for(auto& set : sets)
        addCameraSetToUI(set);
    }
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

void MVGProjectWrapper::updateCamerasFromParticleSelection(bool force)
{
    if(!useParticleSelection())
        return;

    QObjectList filteredCams;

    if(!_particleSelection.empty())
    {
        const auto maxIt = std::max_element(_selectionScorePerCamera.begin(), _selectionScorePerCamera.end(),
                [](const std::pair<MVGCameraWrapper*, int>& p1, const std::pair<MVGCameraWrapper*, int>& p2)
                {
                    return p1.second < p2.second;
                });
        setParticleMaxAccuracy(maxIt->second);
        const auto minAccuracy = getParticleMaxAccuracy() * (_particleSelectionAccuracy/100.0f);

        // Keep only cameras meeting the minimum score requirement
        for(const auto& elt : _selectionScorePerCamera)
        {
            if(elt.second >= minAccuracy)
                filteredCams.append(elt.first);
        }

        // Unless forced to update, same size here means no changes
        if(!force && filteredCams.size() == _particleSelectionCameraSet->getCameras()->size())
            return;

        // Sort model by score
        std::sort(filteredCams.begin(), filteredCams.end(),
                [this](QObject* a, QObject* b){
                   return _selectionScorePerCamera[static_cast<MVGCameraWrapper*>(a)] > _selectionScorePerCamera[static_cast<MVGCameraWrapper*>(b)];
                });
    }

    _particleSelectionCameraSet->highlightLocators(false);
    // Update particle selection set's camera wrappers
    _particleSelectionCameraSet->setCameraWrappers(filteredCams);
    _particleSelectionCameraSet->highlightLocators(true);
}

void MVGProjectWrapper::updateCameraSetWrapperMembers(const MObject &set)
{
    MFnSet fnSet(set);
    MVGCameraSetWrapper* wrapper = _cameraSetsByName[fnSet.name().asChar()];
    MSelectionList list;

    wrapper->fnSet().getMembers(list, false);
    MItSelectionList selectionIt(list);
    MDagPath path;
    QObjectList cams;
    for (; !selectionIt.isDone(); selectionIt.next())
    {
        selectionIt.getDagPath(path);
        path.extendToShape();
        if(path.apiType() == MFn::kCamera)
        {
            MVGCamera cam(path);
            if(cam.isValid() && _camerasByName.count(cam.getDagPathAsString()))
                cams.append(_camerasByName[cam.getDagPathAsString()]);
        }
    }
    wrapper->setCameraWrappers(cams);
}

void MVGProjectWrapper::setCurrentCameraSet(MVGCameraSetWrapper* setWrapper)
{
    if(_currentCameraSet == setWrapper)
        return;

    if(_currentCameraSet)
    {
        // Remove camera locators highlighting
        _currentCameraSet->highlightLocators(false);
        // Disable particle selection if needed before changing the current set
        if(_currentCameraSet == _particleSelectionCameraSet)
            setUseParticleSelection(false);
    }

    int previousIdx = getCurrentCameraSetIndex();
    _currentCameraSet = setWrapper;
    // Highlight camera locators of the new set (if not the default one)
    if(_currentCameraSet != _defaultCameraSet)
        _currentCameraSet->highlightLocators(true);
    Q_EMIT currentCameraSetChanged();
    if(previousIdx != getCurrentCameraSetIndex())
        Q_EMIT currentCameraSetIndexChanged();
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
