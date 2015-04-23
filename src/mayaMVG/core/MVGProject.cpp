#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/io/cameraIO.hpp"
#include "mayaMVG/io/pointCloudIO.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include <maya/MFnTransform.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MDagModifier.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MQuaternion.h>
#include <maya/MEulerRotation.h>
#include <maya/MMatrix.h>
#include <maya/MItSelectionList.h>
#include <maya/MItMeshPolygon.h>

namespace
{ // empty namespace

void lockNode(MObject obj)
{
    if(obj.apiType() != MFn::kTransform)
        return;
    MStatus status;
    MFnDagNode fn(obj, &status);
    CHECK_RETURN(status)
    fn.findPlug("translateX").setLocked(true);
    fn.findPlug("translateY").setLocked(true);
    fn.findPlug("translateZ").setLocked(true);
    fn.findPlug("rotateX").setLocked(true);
    fn.findPlug("rotateY").setLocked(true);
    fn.findPlug("rotateZ").setLocked(true);

    for(int i = 0; i < fn.childCount(); ++i)
        lockNode(fn.child(i));
}

void unlockNode(MObject obj)
{
    if(obj.apiType() != MFn::kTransform)
        return;
    MFnDagNode fn(obj);
    fn.findPlug("translateX").setLocked(false);
    fn.findPlug("translateY").setLocked(false);
    fn.findPlug("translateZ").setLocked(false);
    fn.findPlug("rotateX").setLocked(false);
    fn.findPlug("rotateY").setLocked(false);
    fn.findPlug("rotateZ").setLocked(false);

    for(int i = 0; i < fn.childCount(); ++i)
        unlockNode(fn.child(i));
}

} // empty namespace

namespace mayaMVG
{

std::string MVGProject::_CLOUD = "mvgPointCloud";
std::string MVGProject::_MESH = "mvgMesh";
std::string MVGProject::_PROJECT = "mayaMVG";
MString MVGProject::_PROJECTPATH = "projectPath";

// Paths
std::string MVGProject::_cameraRelativeDirectory =
    stlplus::folder_append_separator("SfM_output") + stlplus::folder_append_separator("cameras");
std::string MVGProject::_imageRelativeDirectory =
    stlplus::folder_append_separator("SfM_output") + stlplus::folder_append_separator("images");
std::string MVGProject::_cameraRelativeFile =
    stlplus::folder_append_separator("SfM_output") + "views.txt";
std::string MVGProject::_pointCloudRelativeFile = stlplus::folder_append_separator("SfM_output") +
                                                  stlplus::folder_append_separator("clouds") +
                                                  "calib.ply";
// Image cache
std::list<std::string> MVGProject::_cachedImagePlanes;
std::map<std::string, std::string> MVGProject::_lastLoadedCameraByView;

MVGProject::MVGProject(const std::string& name)
    : MVGNodeWrapper(name)
{
}

MVGProject::MVGProject(const MDagPath& dagPath)
    : MVGNodeWrapper(dagPath)
{
}

MVGProject::~MVGProject()
{
}

// virtual
bool MVGProject::isValid() const
{
    if(!_dagpath.isValid() || (_dagpath.apiType() != MFn::kTransform))
        return false;
    MFnTransform fn(_dagpath);
    MStatus status;
    fn.findPlug(_PROJECTPATH, false, &status);
    if(!status)
        return false;
    return true;
}

// static
MVGProject MVGProject::create(const std::string& name)
{
    MStatus status;
    MFnTransform fn;
    MDagPath path;

    // root node
    MObject transform = fn.create(MObject::kNullObj, &status);
    CHECK(status)
    // root/cameras
    MObject cameras = fn.create(transform, &status);
    fn.setName("cameras");
    lockNode(cameras);
    // root/clouds
    MObject clouds = fn.create(transform, &status);
    fn.setName("clouds");
    lockNode(clouds);

    MDagPath::getAPathTo(transform, path);
    MVGProject project(path);
    project.setName(name);

    // Add root attributes
    MDagModifier dagModifier;
    MFnTypedAttribute tAttr;
    MObject projectAttr = tAttr.create(_PROJECTPATH, "prp", MFnData::kString);
    dagModifier.addAttribute(path.node(), projectAttr);
    dagModifier.doIt();

    return project;
}

// static
std::vector<MVGProject> MVGProject::list()
{
    std::vector<MVGProject> list;
    MDagPath path;
    MItDependencyNodes it(MFn::kTransform);
    for(; !it.isDone(); it.next())
    {
        MFnDependencyNode fn(it.thisNode());
        MDagPath::getAPathTo(fn.object(), path);
        MVGProject project(path);
        if(project.isValid())
            list.push_back(project);
    }
    return list;
}

bool MVGProject::load(const std::string& projectDirectoryPath)
{
    // TODO: remove when multiproject
    clear();

    if(!isProjectDirectoryValid(projectDirectoryPath))
        return false;
    // Load new elements
    if(!loadCameras(projectDirectoryPath))
        return false;
    if(!loadPointCloud(projectDirectoryPath))
        return false;
    return true;
}

bool MVGProject::loadCameras(const std::string& projectDirectoryPath)
{
    std::string cameraFile =
        stlplus::folder_append_separator(projectDirectoryPath) + _cameraRelativeFile;
    std::string imageDirectory =
        stlplus::folder_append_separator(projectDirectoryPath) + _imageRelativeDirectory;
    std::string cameraDirectory =
        stlplus::folder_append_separator(projectDirectoryPath) + _cameraRelativeDirectory;
    return readCameras(cameraFile, imageDirectory, cameraDirectory);
}

bool MVGProject::loadPointCloud(const std::string& projectDirectoryPath)
{
    std::string pointCloudFile =
        stlplus::folder_append_separator(projectDirectoryPath) + _pointCloudRelativeFile;
    return readPointCloud(pointCloudFile);
}

bool MVGProject::scaleScene(const double scaleSize) const
{
    // Check for root node
    if(!isValid())
        return false;
    // Retrieve reference face
    std::string meshName;
    MPoint A, B, C;
    // Use Maya selection if a mesh face is selected
    MSelectionList selectionList;
    MGlobal::getActiveSelectionList(selectionList);
    bool selectedFace = false;
    MItSelectionList selectionIt(selectionList);
    for(; !selectionIt.isDone(); selectionIt.next())
    {
        MDagPath item;
        MObject component;
        selectionIt.getDagPath(item, component);
        if(component.apiType() != MFn::kMeshPolygonComponent)
            continue;
        LOG_INFO("Mesh path : " << item.fullPathName())
        MVGMesh mesh(item);
        if(!mesh.isActive())
        {
            LOG_ERROR("Mesh is not active in MayaMVG : " << mesh.getName())
            return false;
        }
        selectedFace = true;
        MItMeshPolygon polyIt(item, component);
        A = polyIt.point(0);
        B = polyIt.point(1);
        C = polyIt.point(3);

        MObject transform = item.transform();
        MDagPath::getAPathTo(transform, item);
        meshName = item.partialPathName().asChar();
        break;
    }
    // Else use the first face of the first created mesh
    if(!selectedFace)
    {
        // Check for mesh node
        MVGMesh mesh(_MESH);
        if(!mesh.isValid())
            return false;
        if(!mesh.isActive())
        {
            LOG_ERROR("Mesh is not active in MayaMVG : " << mesh.getName())
            return false;
        }
        mesh.getPoint(0, A);
        mesh.getPoint(1, B);
        mesh.getPoint(3, C);
        meshName = _MESH;
    }
    MVector AB = B - A;
    double scaleFactor = scaleSize / AB.length();
    AB.normalize();
    MVector AC = C - A;
    AC.normalize();
    MVector RZ = (AC ^ AB);
    RZ.normalize();
    MVector RX = (AB ^ RZ);
    RX.normalize();
    MVector RY = (RZ ^ RX);
    RY.normalize();
    // Compute rotateScale matrix
    MMatrix rotateScaleMatrix;
    rotateScaleMatrix[0][0] = scaleFactor * RX[0];
    rotateScaleMatrix[0][1] = scaleFactor * RY[0];
    rotateScaleMatrix[0][2] = scaleFactor * RZ[0];
    rotateScaleMatrix[1][0] = scaleFactor * RX[1];
    rotateScaleMatrix[1][1] = scaleFactor * RY[1];
    rotateScaleMatrix[1][2] = scaleFactor * RZ[1];
    rotateScaleMatrix[2][0] = scaleFactor * RX[2];
    rotateScaleMatrix[2][1] = scaleFactor * RY[2];
    rotateScaleMatrix[2][2] = scaleFactor * RZ[2];
    // Compute translation matrix
    MMatrix translateMatrix;
    translateMatrix[3][0] = -A.x;
    translateMatrix[3][1] = -A.y;
    translateMatrix[3][2] = -A.z;
    // Apply transformation
    MMatrix transformMatrix = translateMatrix * rotateScaleMatrix;
    MString matrix;
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            matrix += transformMatrix[i][j];
            if(!(i == 3 && j == 3))
                matrix += " ";
        }
    }
    MGlobal::executePythonCommand("from mayaMVG import scale");
    MGlobal::executePythonCommand("scale.scaleScene('" + matrix + "', '" + _PROJECT.c_str() + "')",
                                  false, true);

    // Update cache
    MGlobal::executeCommand("mayaMVGTool -e -rebuild mayaMVGTool1");
    return true;
}

void MVGProject::clear()
{
    MDagPath camerasDagPath;
    MVGMayaUtil::getDagPathByName("cameras", camerasDagPath);
    for(int i = camerasDagPath.childCount(); i > 0; --i)
    {
        MObject child = camerasDagPath.child(i - 1);
        MGlobal::deleteNode(child);
    }
    MDagPath pointCloudDagPath;
    MVGMayaUtil::getDagPathByName("clouds", pointCloudDagPath);
    for(int i = pointCloudDagPath.childCount(); i > 0; --i)
    {
        MObject child = pointCloudDagPath.child(i - 1);
        MGlobal::deleteNode(child);
    }
}

const std::string MVGProject::getProjectDirectory() const
{
    MString directory;
    // Retrive it by its name
    MDagPath path;
    MVGMayaUtil::getDagPathByName(_PROJECT.c_str(), path);
    MVGMayaUtil::getStringAttribute(path.node(), _PROJECTPATH, directory);
    return (directory.length() > 0) ? directory.asChar() : "";
}

void MVGProject::setProjectDirectory(const std::string& directory) const
{
    // Retrive it by its name
    MDagPath path;
    MVGMayaUtil::getDagPathByName(_PROJECT.c_str(), path);
    MVGMayaUtil::setStringAttribute(path.node(), _PROJECTPATH, directory.c_str());
}

const bool MVGProject::isProjectDirectoryValid(const std::string& projectDirectoryPath) const
{
    // Camera file
    std::string cameraFile =
        stlplus::folder_append_separator(projectDirectoryPath) + _cameraRelativeFile;
    std::ifstream infile(cameraFile.c_str());
    if(!infile.is_open())
    {
        LOG_ERROR("Camera file not found (" << cameraFile << ")")
        return false;
    }
    // Cloud file
    std::string pointCloudFile =
        stlplus::folder_append_separator(projectDirectoryPath) + _pointCloudRelativeFile;
    Ply ply;
    if(!ply.open(pointCloudFile))
    {
        LOG_ERROR("Point cloud file not found (" << pointCloudFile << ")")
        ply.close();
        return false;
    }
    return true;
}

void MVGProject::selectCameras(std::vector<std::string> cameraNames) const
{
    MSelectionList list;
    for(std::vector<std::string>::iterator it = cameraNames.begin(); it != cameraNames.end(); ++it)
    {
        MVGCamera camera(*it);
        list.add(camera.getDagPath());
    }
    MGlobal::setActiveSelectionList(list);
}

void MVGProject::selectMeshes(std::vector<std::string> meshes) const
{
    MSelectionList list;
    for(std::vector<std::string>::iterator it = meshes.begin(); it != meshes.end(); ++it)
    {
        MVGMesh mesh(*it);
        list.add(mesh.getDagPath());
    }
    MGlobal::setActiveSelectionList(list);
}

void MVGProject::unlockProject() const
{
    unlockNode(getObject());
}

void MVGProject::lockProject() const
{
    lockNode(getObject());
}

void MVGProject::pushImageInCache(const std::string& cameraName)
{
    if(cameraName.empty())
        return;

    std::list<std::string>::iterator camera =
        std::find(_cachedImagePlanes.begin(), _cachedImagePlanes.end(), cameraName);
    if(camera != _cachedImagePlanes.end()) // Camera is already in the list
        return;

    if(_cachedImagePlanes.size() == IMAGE_CACHE_SIZE)
    {
        std::string frontCamera = _cachedImagePlanes.front();
        MVGCamera camera(frontCamera);
        camera.unloadImagePlane();
        _cachedImagePlanes.pop_front();
    }
    _cachedImagePlanes.push_back(cameraName);
}

const std::string MVGProject::getLastLoadedCameraInView(const std::string& viewName) const
{
    std::map<std::string, std::string>::const_iterator findIt =
        _lastLoadedCameraByView.find(viewName);
    if(findIt == _lastLoadedCameraByView.end())
        return "";
    return findIt->second;
}

void MVGProject::setLastLoadedCameraInView(const std::string& viewName,
                                           const std::string& cameraName)
{
    _lastLoadedCameraByView[viewName] = cameraName;
}

/**
 * Update the image cache :
 *     - If current camera is in cache : remove it
 *     - Push last current camera in cache
 * @param newCameraName current camera
 * @param oldCameraName last current camera that we need to push in cache
 */
void MVGProject::updateImageCache(const std::string& newCameraName,
                                  const std::string& oldCameraName)
{
    // If new camera is in cache remove from cacheList
    std::list<std::string>::iterator cameraIt =
        std::find(_cachedImagePlanes.begin(), _cachedImagePlanes.end(), newCameraName);
    if(cameraIt != _cachedImagePlanes.end())
        _cachedImagePlanes.remove(newCameraName);

    if(oldCameraName.length() > 0)
        pushImageInCache(oldCameraName);
}

/**
 * Create a command to load the current image plane corresponding to the camera in the panel.
 * Push the command to the idle queue
 * @param panelName view in which the image plane will be updated
 */
void MVGProject::pushLoadCurrentImagePlaneCommand(const std::string& panelName) const
{
    MStatus status;
    MString cmd;
    cmd.format("MVGImagePlaneCmd -panel \"^1s\" -load ", panelName.c_str());
    status = MGlobal::executeCommandOnIdle(cmd);
    CHECK_RETURN(status)
}

} // namespace
