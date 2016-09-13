#pragma once

#undef Success // needed by eigen
#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include "openMVG/cameras/PinholeCamera.hpp"
#include <vector>
#include <map>

class MString;
class MPoint;
class MIntArray;

namespace mayaMVG
{

class MVGPointCloudItem;

class MVGCamera : public MVGNodeWrapper
{

public:
    MVGCamera();
    MVGCamera(const std::string& dagPathAsString);
    MVGCamera(const MDagPath& dagPath);
    MVGCamera(const int& id);
    virtual ~MVGCamera();

public:
    bool operator<(const MVGCamera&) const;

public:
    virtual bool isValid() const;

public:
    static MVGCamera create(MDagPath& cameraDagPath, std::map<int, MIntArray>& itemsPerCamera);
    static std::vector<MVGCamera> getCameras();

public:
    int getId() const;
    void setId(const int&) const;
    MDagPath getImagePlaneShapeDagPath() const;
    std::string getThumbnailPath() const;
    void setImagePlane() const;
    void unloadImagePlane() const;
    MPoint getCenter(MSpace::Space space = MSpace::kWorld) const;
    void getSensorSize(MIntArray& sensorSize) const;
    void getVisibleIndexes(MIntArray& visibleIndexes) const;
    void getVisibleItems(std::vector<MVGPointCloudItem>& visibleItems) const;
    void setVisibleItems(const std::vector<MVGPointCloudItem>& item) const;
    double getZoom() const;
    void setZoom(const double zoom) const;
    double getHorizontalPan() const;
    void setHorizontalPan(const double pan) const;
    double getVerticalPan() const;
    void setVerticalPan(const double pan) const;
    void setPan(const double hpan, const double vpan) const;
    void setAspectRatio(const double ratio) const;
    double getHorizontalFilmAperture() const;
    void resetZoomAndPan() const;
    void setInView(const std::string& viewName) const;
    void setNear(const double near) const;
    void setFar(const double far) const;
    void setImagePlaneDepth(const double depth) const;
    void setLocatorScale(const double scale) const;

    const std::pair<double, double> getImageSize() const;

public:
    static MString _MVG_IMAGE_PATH;
    static MString _MVG_IMAGE_SOURCE_PATH;
    static MString _MVG_THUMBNAIL_PATH;
    static MString _MVG_ITEMS;
    static MString _MVG_VIEW_ID;

private:
    static MString _MVG_INTRINSIC_ID;
    static MString _MVG_INTRINSIC_TYPE;
    static MString _MVG_INTRINSICS_PARAMS;
    static MString _MVG_SENSOR_SIZE;
};

} // namespace
