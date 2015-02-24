#pragma once

#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include <vector>
#include <list>
#include <map>

namespace mayaMVG
{
#define IMAGE_CACHE_SIZE 3

class MVGCamera;
class MVGPointCloud;

class MVGProject : public MVGNodeWrapper
{

public:
    MVGProject(const std::string& name = "mayaMVG");
    MVGProject(const MDagPath& dagPath);
    virtual ~MVGProject();

public:
    virtual bool isValid() const;

public:
    static MVGProject create(const std::string& name);
    static std::vector<MVGProject> list();

public:
    bool load(const std::string& projectDirectoryPath);
    bool loadCameras(const std::string& projectDirectoryPath);
    bool loadPointCloud(const std::string& projectDirectoryPath);
    bool scaleScene(const double scaleSize) const;
    void clear();

public:
    const std::string getProjectDirectory() const;
    void setProjectDirectory(const std::string&) const;
    const bool isProjectDirectoryValid(const std::string&) const;
    void selectCameras(std::vector<std::string> cameraNames) const;
    void unlockProject() const;
    void lockProject() const;
    // Image "cache"
    const std::string getLastLoadedCameraInView(const std::string& viewName) const;
    void setLastLoadedCameraInView(const std::string& viewName, const std::string& cameraName);
    void pushLoadCurrentImagePlaneCommand(const std::string& panelName) const;
    void pushImageInCache(const std::string& cameraName);
    void updateImageCache(const std::string& newCameraName, const std::string& oldCameraName);
    const std::list<std::string>& getImageCache() { return _cachedImagePlanes; };

public:
    // openMVG node names
    static std::string _CLOUD;
    static std::string _MESH;
    static std::string _PROJECT;
    // dynamic attributes
    static MString _PROJECTPATH;
    static std::string _cameraRelativeDirectory;
    static std::string _imageRelativeDirectory;
    static std::string _cameraRelativeFile;
    static std::string _pointCloudRelativeFile;

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
