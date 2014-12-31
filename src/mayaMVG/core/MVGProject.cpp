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

std::string MVGProject::_cameraRelativeDirectory =
    stlplus::folder_append_separator("outIncremental") +
    stlplus::folder_append_separator("SfM_output") + stlplus::folder_append_separator("cameras");
std::string MVGProject::_imageRelativeDirectory =
    stlplus::folder_append_separator("outIncremental") +
    stlplus::folder_append_separator("SfM_output") + stlplus::folder_append_separator("images");
std::string MVGProject::_cameraRelativeFile = stlplus::folder_append_separator("outIncremental") +
                                              stlplus::folder_append_separator("SfM_output") +
                                              "views.txt";
std::string MVGProject::_pointCloudRelativeFile =
    stlplus::folder_append_separator("outIncremental") +
    stlplus::folder_append_separator("SfM_output") + stlplus::folder_append_separator("clouds") +
    "calib.ply";

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
    // Check for mesh node
    MVGMesh mesh(_MESH);
    if(!mesh.isValid())
        return false;
    // Get references
    // Y axis is described by the two first points
    // X axis is described by the first and last points
    MPoint A, B, C;
    mesh.getPoint(0, A);
    mesh.getPoint(1, B);
    mesh.getPoint(3, C);
    MVector AB = B - A;
    double scaleFactor = scaleSize / AB.length();
    AB.normalize();
    MVector y(0, 1, 0);
    double yAngle = acos(AB * y);
    MVector yRotAxe = (AB ^ y);
    yRotAxe.normalize();
    MQuaternion quat(yAngle, yRotAxe);
    MEulerRotation euler = quat.asEulerRotation();

    // Translate : center object in (0, 0, 0)
    MMatrix translationMatrix;
    translationMatrix[3][0] = -A.x;
    translationMatrix[3][1] = -A.y;
    translationMatrix[3][2] = -A.z;
    translationMatrix[3][3] = 1;
    // Rotate : fit Y axis
    MMatrix rotationXMatrix;
    rotationXMatrix[1][1] = cos(euler.x);
    rotationXMatrix[1][2] = sin(euler.x);
    rotationXMatrix[2][1] = -sin(euler.x);
    rotationXMatrix[2][2] = cos(euler.x);
    MMatrix rotationYMatrix;
    rotationYMatrix[0][0] = cos(euler.y);
    rotationYMatrix[0][2] = -sin(euler.y);
    rotationYMatrix[2][0] = sin(euler.y);
    rotationYMatrix[2][2] = cos(euler.y);
    MMatrix rotationZMatrix;
    rotationZMatrix[0][0] = cos(euler.z);
    rotationZMatrix[0][1] = sin(euler.z);
    rotationZMatrix[1][0] = -sin(euler.z);
    rotationZMatrix[1][1] = cos(euler.z);
    // Scale
    MMatrix scaleMatrix;
    scaleMatrix[0][0] = scaleFactor;
    scaleMatrix[1][1] = scaleFactor;
    scaleMatrix[2][2] = scaleFactor;
    MMatrix transformMatrix =
        translationMatrix * rotationXMatrix * rotationYMatrix * rotationZMatrix * scaleMatrix;
    // Rotate : fit X axis
    MPoint transformedA = A * transformMatrix;
    MPoint transformedC = C * transformMatrix;
    MVector AC = transformedC - transformedA;
    AC.normalize();
    MVector x(1, 0, 0);
    double xAngle = acos(AC * x);
    MVector xRotAxe = (AC ^ x);
    xRotAxe.normalize();
    MQuaternion xQuat(xAngle, xRotAxe);
    euler = xQuat.asEulerRotation();
    MMatrix rotationYMatrix2;
    rotationYMatrix2[0][0] = cos(euler.y);
    rotationYMatrix2[0][2] = -sin(euler.y);
    rotationYMatrix2[2][0] = sin(euler.y);
    rotationYMatrix2[2][2] = cos(euler.y);
    MMatrix rotationXMatrix2;
    rotationXMatrix2[1][1] = cos(euler.x);
    rotationXMatrix2[1][2] = sin(euler.x);
    rotationXMatrix2[2][1] = -sin(euler.x);
    rotationXMatrix2[2][2] = cos(euler.x);
    MMatrix rotationZMatrix2;
    rotationZMatrix2[0][0] = cos(euler.z);
    rotationZMatrix2[0][1] = sin(euler.z);
    rotationZMatrix2[1][0] = -sin(euler.z);
    rotationZMatrix2[1][1] = cos(euler.z);
    transformMatrix *= rotationXMatrix2 * rotationYMatrix2 * rotationZMatrix2;

    // Apply transformation
    MString matrix;
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 4; ++j)
        {
            matrix += transformMatrix[i][j];
            matrix += " ";
        }
    }
    MString cmd;
    MString meshName(_MESH.c_str());
    MString projectName(_PROJECT.c_str());
    unlockProject();

    // MakeIdentity/Freeze transform to reset transformation and use directly world coordinates
    cmd.format("xform -r -m ^1s ^2s; makeIdentity -a true ^2s; xform -r -m ^1s ^3s; makeIdentity "
               "-a true ^3s",
               matrix, projectName, meshName);
    MGlobal::executeCommand(cmd, false, true);
    lockProject();

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

void MVGProject::unlockProject() const
{
    unlockNode(getObject());
}

void MVGProject::lockProject() const
{
    lockNode(getObject());
}
} // namespace
