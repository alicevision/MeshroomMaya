#pragma once

#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include <vector>

namespace mayaMVG
{

class MVGCamera;
class MVGPointCloudItem;

class MVGPointCloud : public MVGNodeWrapper
{

public:
    MVGPointCloud(const std::string& name);
    MVGPointCloud(const MDagPath& dagPath);
    virtual ~MVGPointCloud();

public:
    virtual bool isValid() const;

public:
    static MVGPointCloud create(const std::string& name);

public:
    void setItems(const std::vector<MVGPointCloudItem>& items);
    std::vector<MVGPointCloudItem> getItems() const;

private:
    static MString _RGBPP;
};

} // namespace
