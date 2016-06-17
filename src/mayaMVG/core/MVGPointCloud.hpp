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
    MStatus getItems(std::vector<MVGPointCloudItem>& items, const MIntArray& indexes) const;
    bool projectPoints(M3dView& view, const std::vector<MVGPointCloudItem>& visibleItems,
                       const MPointArray& faceCSPoints, MPointArray& faceWSPoints);
    bool projectPointsWithLineConstraint(M3dView& view,
                                         const std::vector<MVGPointCloudItem>& visibleItems,
                                         const MPointArray& faceCSPoints,
                                         const MPointArray& constraintedWSPoints,
                                         const MPoint& mouseCSPoint, MPoint& projectedWSMouse);
};

} // namespace
