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

    struct IntersectedComponent
    {
        IntersectedComponent()
            : type(MFn::kInvalid)
            , vertex(NULL)
            , edge(NULL)
        {
        }
        MFn::Type type; // kMeshEdgeComponent, kMeshVertComponent
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
    bool checkIntersection(double, const MPoint&);
    const IntersectedComponent& getIntersectedComponent();

    // mesh & view relative data
    const std::map<std::string, MeshData>& getMeshData() const;
    const MeshData& getMeshData(const std::string meshName);
    void rebuildMeshesCache();
    void rebuildMeshCache(const MDagPath&);

private:
    bool isIntersectingPoint(double, const MPoint&);
    bool isIntersectingEdge(double, const MPoint&);

private:
    M3dView _activeView;
    MVGCamera _activeCamera;
    IntersectedComponent _intersectedComponent;
    std::map<std::string, MeshData> _meshData; // per mesh
};

} // namespace
