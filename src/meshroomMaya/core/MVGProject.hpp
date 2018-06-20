#pragma once

#include "meshroomMaya/core/MVGNodeWrapper.hpp"
#include "maya/MColor.h"
#include <maya/MGlobal.h>
#include <vector>
#include <list>
#include <map>


namespace meshroomMaya
{
#define IMAGE_CACHE_SIZE 3

class MVGCamera;
class MVGPointCloud;

class MVGProject : public MVGNodeWrapper
{

public:
    MVGProject(const std::string& name = MVGProject::_PROJECT);
    MVGProject(const MDagPath& dagPath);
    virtual ~MVGProject();

public:
    virtual bool isValid() const;

public:
    static std::vector<MVGProject> list();
    /// Get all the camera sets created by MeshroomMaya in the current Maya scene
    static std::vector<MObject> getMVGCameraSets();
    /// Return whether the given set is a camera set created by MeshroomMaya
    static bool isMVGCameraSet(const MObject& obj);

public:
    bool applySceneTransformation() const;
    void clear();

public:
    const std::string getProjectDirectory() const;
    void setProjectDirectory(const std::string&) const;
    void selectCameras(const std::vector<std::string>& cameraNames, MGlobal::ListAdjustment=MGlobal::kReplaceList) const;
    void selectMeshes(const std::vector<std::string>& meshNames) const;
    void unlockProject() const;
    void lockProject() const;
    // Image "cache"
    const std::string getLastLoadedCameraInView(const std::string& viewName) const;
    void setLastLoadedCameraInView(const std::string& viewName, const std::string& cameraName);
    void pushLoadCurrentImagePlaneCommand(const std::string& panelName) const;
    void pushImageInCache(const std::string& cameraName);
    void updateImageCache(const std::string& newCameraName, const std::string& oldCameraName);
    const std::list<std::string>& getImageCache() { return _cachedImagePlanes; };
    void clearImageCache();

public:
    // aliceVision node names
    static std::string _CAMERAS_GROUP;
    static std::string _CLOUD_GROUP;
    static std::string _CLOUD;
    static std::string _MESH;
    static std::string _PROJECT;
    static std::string _LOCATOR;
    static std::string _CAMERA_POINTS_LOCATOR;
    static MColor _LEFT_PANEL_DEFAULT_COLOR;
    static MColor _RIGHT_PANEL_DEFAULT_COLOR;
    static MColor _COMMON_POINTS_DEFAULT_COLOR;
    static MString _MVG_PROJECTPATH;
    static std::string _CAMERASET_PREFIX;

    /// FIFO queue indicating the list of images/cameras keept in memory
    /// Cameras corresponding to current images seen in panels are not stored in this list.
    static std::list<std::string> _cachedImagePlanes;
    /// Stores the camera name of the last image plane loaded in each view.
    /// The user can change the camera of the view faster than what Maya is
    /// able to do with the loading time of image planes.
    /// So the current camera in the view is not always the same
    /// than the "last loaded image plane".
    static std::map<std::string, std::string> _lastLoadedCameraByView;
};

} // namespace
