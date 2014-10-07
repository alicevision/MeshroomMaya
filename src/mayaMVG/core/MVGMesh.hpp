#pragma once

#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include <vector>
#include <map>

class MPoint;
class MPointArray;
class MIntArray;

namespace mayaMVG
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
    MVGMesh(const std::string& name);
    MVGMesh(const MString& name);
    MVGMesh(const MDagPath& dagPath);
    MVGMesh(const MObject& object);
    virtual ~MVGMesh() {}

public:
    virtual bool isValid() const;

public:
    static MVGMesh create(const std::string& name);
    static std::vector<MVGMesh> list();

public:
    bool addPolygon(const MPointArray& pointArray, int& index) const;
    bool deletePolygon(const int index) const;
    MStatus getPoints(MPointArray& pointArray) const;
    int getPolygonsCount() const;
    MStatus getPolygonVertices(const int polygonId, MIntArray& vertexList) const;
    const MIntArray getConnectedFacesToVertex(int vertexId);
    const MIntArray getConnectedFacesToEdge(int edgeId);
    const MIntArray getFaceVertices(int faceId);
    MStatus getPoint(int vertexId, MPoint& point) const;
    MStatus setPoint(int vertexId, MPoint& point) const;
    MStatus setBlindData(const int vertexId, std::vector<ClickedCSPosition>& data) const;
    MStatus getBlindData(const int vertexId, std::vector<ClickedCSPosition>& data) const;
    MStatus getBlindData(const int vertexId, std::map<int, MPoint>& cameraToClickedCSPoints) const;
    MStatus unsetBlindData(const int vertexId) const;
    MStatus getBlindDataPerCamera(const int vertexId, const int cameraId, MPoint& point2D) const;
    MStatus setBlindDataPerCamera(const int vertexId, const int cameraId,
                                  const MPoint& point2D) const;
    MStatus unsetBlindDataPerCamera(const int vertexId, const int cameraId) const;

private:
    static int _blindDataID;
};

} // namespace
