#pragma once

#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include <maya/M3dView.h>
#include <maya/MPointArray.h>
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
    bool projectPolygon(M3dView& view, const MPointArray& cameraSpacePoints,
                        MPointArray& worldSpacePoints);

private:
    static MString _RGBPP;
};

} // namespace
