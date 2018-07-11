#pragma once

#include "meshroomMaya/core/MVGNodeWrapper.hpp"
#include <vector>
#include <map>

class MPoint;
class MPointArray;
class MIntArray;

namespace meshroomMaya
{

class MVGMesh : public MVGNodeWrapper
{
public:
    struct ClickedCSPosition
    {
        unsigned int cameraId;
        double x;
        double y;
    };

public:
    MVGMesh(const std::string& dagPathAsString);
    MVGMesh(const MString& dagPathAsString);
    MVGMesh(const MDagPath& dagPath);
    MVGMesh(const MObject& object);
    virtual ~MVGMesh() {}

public:
    virtual bool isValid() const;

public:
    static MVGMesh create(const std::string& name);
    static std::vector<MVGMesh> listActiveMeshes();
    static std::vector<MVGMesh> listAllMeshes();

public:
    void setIsActive(const bool value) const;
    bool isActive() const;
    bool addPolygon(const MPointArray& pointArray, int& index) const;
    bool deletePolygon(const int index) const;
    MStatus getPoints(MPointArray& pointArray) const;
    int getPolygonsCount() const;
    int getVerticesCount() const;
    MStatus getPolygonVertices(const int polygonId, MIntArray& vertexList) const;
    const MIntArray getConnectedFacesToVertex(const int vertexId);
    const MIntArray getConnectedFacesToEdge(const int edgeId);
    const MIntArray getFaceVertices(const int faceId);
    MStatus getPoint(const int vertexId, MPoint& point) const;
    MStatus setPoint(const int vertexId, const MPoint& point) const;
    MStatus setPoints(const MIntArray& verticesIds, const MPointArray& points) const;
    MStatus setBlindData(const int vertexId, std::vector<ClickedCSPosition>& data) const;
    MStatus getBlindData(const int vertexId, std::vector<ClickedCSPosition>& data) const;
    MStatus getBlindData(const int vertexId, std::map<int, MPoint>& cameraToClickedCSPoints) const;
    MStatus unsetAllBlindData() const;
    MStatus unsetBlindData(const int vertexId) const;
    MStatus getBlindDataPerCamera(const int vertexId, const int cameraId, MPoint& point2D) const;
    MStatus setBlindDataPerCamera(const int vertexId, const int cameraId,
                                  const MPoint& point2D) const;
    MStatus unsetBlindDataPerCamera(const int vertexId, const int cameraId) const;

private:
    static int _blindDataID;
    static MString _MVG;
};

} // namespace
