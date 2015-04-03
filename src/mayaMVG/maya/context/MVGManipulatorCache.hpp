#pragma once

#include "mayaMVG/core/MVGCamera.hpp"
#include <maya/MDagPath.h>
#include <maya/MIntArray.h>
#include <maya/MPointArray.h>
#include <maya/M3dView.h>
#include <map>
#include <vector>

namespace mayaMVG
{

class MVGContext;

class MVGManipulatorCache
{
public:
    struct VertexData
    {
        VertexData()
            : index(-1)
            , numConnectedEdges(-1)
        {
        }
        int index;
        int numConnectedEdges;
        MPoint worldPosition;
        std::map<int, MPoint> blindData; // map from cameraIDs to clickedCSPositions
        /// Map from cameraIDs to cameraSpacePositions
        /// Only store the point for the current cameras
        std::map<int, MPoint> cameraSpacePoints;
    };

    struct EdgeData
    {
        EdgeData()
            : index(-1)
            , vertex1(NULL)
            , vertex2(NULL)
        {
        }
        int index;
        VertexData* vertex1;
        VertexData* vertex2;
    };

    struct MeshData
    {
        std::vector<VertexData> vertices;
        std::vector<EdgeData> edges;
    };

    struct MVGComponent
    {
        MVGComponent()
            : type(MFn::kInvalid)
            , vertex(NULL)
            , edge(NULL)
        {
        }
        MFn::Type type; // kMeshEdgeComponent, kMeshVertComponent, kBlindData
        MDagPath meshPath;
        VertexData* vertex;
        EdgeData* edge;
    };

public:
    MVGManipulatorCache();

public:
    // views
    void setActiveView(const M3dView&);
    M3dView& getActiveView();
    const MVGCamera& getActiveCamera() const;

    // intersections tests
    bool checkIntersection(const double, const MPoint&, const bool checkBlindData = false);
    const MVGComponent& getIntersectedComponent() const;
    void clearIntersectedComponent() { _intersectedComponent = MVGComponent(); }
    const MFn::Type getIntersectionType() const;

    // mesh & view relative data
    const std::map<std::string, MeshData>& getMeshData() const;
    const MeshData& getMeshData(const std::string meshName);
    void rebuildMeshesCache();
    void rebuildMeshCache(const MDagPath&);
    void computeMeshCacheForCameraID(M3dView& view, const int cameraID);
    void removeMeshCacheForCameraID(const int cameraID);

    const MVGComponent& getSelectedComponent() const { return _selectedComponent; }
    void setSelectedComponent(const MVGComponent& selectedComponent);
    void clearSelectedComponent() { _selectedComponent = MVGComponent(); }
    void updateSelectedComponent(const MDagPath& meshPath, const MFn::Type type, const int index);

private:
    bool isIntersectingBlindData(const double, const MPoint&);
    bool isIntersectingPoint(const double, const MPoint&);
    bool isIntersectingEdge(const double, const MPoint&);

private:
    M3dView _activeView;
    MVGCamera _activeCamera;
    MVGComponent _intersectedComponent;
    MVGComponent _selectedComponent;
    std::map<std::string, MeshData> _meshData; // per mesh
};

} // namespace
