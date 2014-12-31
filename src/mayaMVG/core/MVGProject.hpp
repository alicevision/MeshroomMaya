#pragma once

#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include <vector>

namespace mayaMVG
{

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
};

} // namespace
