#pragma once

#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include "mayaMVG/core/MVGPointCloudItem.hpp"
#include <vector>

class MIntArray;
class MPointArray;
class M3dView;
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
    const std::vector<MVGPointCloudItem> getAllItems() const;
    const std::vector<MVGPointCloudItem> getItems(const MIntArray& indexes) const;
    bool projectPoints(M3dView& view, std::vector<MVGPointCloudItem>& visibleItems,
                       const MPointArray& cameraSpacePoints, MPointArray& worldSpacePoints,
                       const int index = -1);

private:
    static MString _RGBPP;
};

} // namespace
