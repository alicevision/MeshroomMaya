#include "mayaMVG/maya/context/MVGManipulatorCache.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>

namespace mayaMVG
{

namespace
{ // empty namespace

float minimumDistanceToEdge(MPoint A, MPoint B, MPoint P)
{
    if(A == B)
        return P.distanceTo(A);
    // consider the line extending the segment, parameterized as A + t (B - A).
    // we find projection of point p onto the line.
    // it falls where t = [(P-A) . (B-A)] / |B-A|^2
    MVector vector(B - A);
    const float t = (MVector(P - A) * vector) / pow(vector.length(), 2);
    if(t < 0.0)
        return P.distanceTo(A); // beyond the 'A' end of the segment
    else if(t > 1.0)
        return P.distanceTo(B);               // beyond the 'B' end of the segment
    const MPoint projection = A + t * vector; // projection falls on the segment
    return P.distanceTo(projection);
}

} // empty namespace

MVGManipulatorCache::MVGManipulatorCache()
{
}

void MVGManipulatorCache::setActiveView(const M3dView& view)
{
    _activeView = view;
    MDagPath cameraPath;
    _activeView.getCamera(cameraPath);
    _activeCamera = MVGCamera(cameraPath);
}

M3dView& MVGManipulatorCache::getActiveView()
{
    return _activeView;
}

const MVGCamera& MVGManipulatorCache::getActiveCamera() const
{
    return _activeCamera;
}

bool MVGManipulatorCache::checkIntersection(double tolerance, const MPoint& mouseCSPosition)
{
    if(isIntersectingPoint(tolerance, mouseCSPosition))
        return true;
    if(isIntersectingEdge(tolerance, mouseCSPosition))
        return true;
    // clear intersected component
    _intersectedComponent = MVGManipulatorCache::IntersectedComponent();
    return false;
}

const MVGManipulatorCache::IntersectedComponent& MVGManipulatorCache::getIntersectedComponent()
{
    return _intersectedComponent;
}
const std::map<std::string, MVGManipulatorCache::MeshData>& MVGManipulatorCache::getMeshData() const
{
    return _meshData;
}
const MVGManipulatorCache::MeshData& MVGManipulatorCache::getMeshData(const std::string meshName)
{
    return _meshData[meshName];
}
void MVGManipulatorCache::rebuildMeshesCache()
{
    // clear cache & other associated data
    _intersectedComponent = MVGManipulatorCache::IntersectedComponent();
    _meshData.clear();
    // rebuild cache
    std::vector<MVGMesh> meshes = MVGMesh::list();
    std::vector<MVGMesh>::const_iterator it = meshes.begin();
    for(; it != meshes.end(); ++it)
        rebuildMeshCache(it->getDagPath());
}

void MVGManipulatorCache::rebuildMeshCache(const MDagPath& path)
{
    MVGMesh mesh(path);
    if(!mesh.isValid())
        return;
    // prepare vertices & edges iterators
    MStatus status;
    MItMeshVertex vIt(path, MObject::kNullObj, &status);
    vIt.updateSurface();
    vIt.geomChanged();
    CHECK_RETURN(status)
    MItMeshEdge eIt(path, MObject::kNullObj, &status);
    eIt.updateSurface();
    eIt.geomChanged();
    CHECK_RETURN(status)
    // prepare & add an empty mesh data object
    _meshData[path.fullPathName().asChar()] = MeshData();
    MeshData& newMeshData = _meshData[path.fullPathName().asChar()];
    newMeshData.vertices.resize(vIt.count());
    newMeshData.edges.resize(eIt.count());
    // fill it with vertices data
    while(!vIt.isDone())
    {
        int index = vIt.index(&status);
        CHECK(status)
        int numConnectedEdges = -1;
        CHECK(vIt.numConnectedEdges(numConnectedEdges))
        // blind data
        VertexData& vertex = newMeshData.vertices[index];
        vertex.index = index;
        vertex.numConnectedEdges = numConnectedEdges;
        vertex.worldPosition = vIt.position(MSpace::kWorld, &status);
        mesh.getBlindData(index, vertex.blindData);
        vIt.next();
    }
    // fill it w/ edges data
    while(!eIt.isDone())
    {
        assert(eIt.index(0) < newMeshData.vertices.size());
        assert(eIt.index(1) < newMeshData.vertices.size());
        std::vector<VertexData>::iterator v1It = newMeshData.vertices.begin();
        std::vector<VertexData>::iterator v2It = newMeshData.vertices.begin();
        v1It += eIt.index(0);
        v2It += eIt.index(1);
        EdgeData& edge = newMeshData.edges[eIt.index()];
        edge.index = eIt.index();
        edge.vertex1 = &(*v1It);
        edge.vertex2 = &(*v2It);
        eIt.next();
    }
}

bool MVGManipulatorCache::isIntersectingPoint(double tolerance, const MPoint& mouseCSPosition)
{
    if(_meshData.empty())
        return false;
    // compute tolerance
    double threshold = (tolerance * _activeCamera.getZoom()) / (double)_activeView.portWidth();
    // check each mesh vertices
    std::map<std::string, MeshData>::iterator meshIt = _meshData.begin();
    for(; meshIt != _meshData.end(); ++meshIt)
    {
        std::vector<VertexData>& vertices = meshIt->second.vertices;
        std::vector<VertexData>::iterator vertexIt = vertices.begin();
        for(; vertexIt < vertices.end(); ++vertexIt)
        {
            // check if we intersect w/ the real vertex position projection
            MPoint realCSVertexPosition;
            MVGGeometryUtil::worldToCameraSpace(_activeView, vertexIt->worldPosition,
                                                realCSVertexPosition);
            if(mouseCSPosition.x <= realCSVertexPosition.x + threshold &&
               mouseCSPosition.x >= realCSVertexPosition.x - threshold &&
               mouseCSPosition.y <= realCSVertexPosition.y + threshold &&
               mouseCSPosition.y >= realCSVertexPosition.y - threshold)
            {
                MDagPath meshPath;
                MVGMayaUtil::getDagPathByName(meshIt->first.c_str(), meshPath);
                _intersectedComponent.type = MFn::kMeshVertComponent;
                _intersectedComponent.meshPath = meshPath;
                _intersectedComponent.vertex = &(*vertexIt);
                _intersectedComponent.edge = NULL;
                return true;
            }
        }
    }
    return false;
}

bool MVGManipulatorCache::isIntersectingEdge(double tolerance, const MPoint& mouseCSPosition)
{
    if(_meshData.empty())
        return false;
    // compute tolerance
    double threshold = (tolerance * _activeCamera.getZoom()) / (double)_activeView.portWidth();
    // check each mesh edges
    std::map<std::string, MeshData>::iterator meshIt = _meshData.begin();
    for(; meshIt != _meshData.end(); ++meshIt)
    {
        std::vector<EdgeData>& edges = meshIt->second.edges;
        std::vector<EdgeData>::iterator edgeIt = edges.begin();
        for(; edgeIt < edges.end(); ++edgeIt)
        {
            MPoint vertex1CSPosition;
            MPoint vertex2CSPosition;
            MVGGeometryUtil::worldToCameraSpace(_activeView, edgeIt->vertex1->worldPosition,
                                                vertex1CSPosition);
            MVGGeometryUtil::worldToCameraSpace(_activeView, edgeIt->vertex2->worldPosition,
                                                vertex2CSPosition);
            if(minimumDistanceToEdge(vertex1CSPosition, vertex2CSPosition, mouseCSPosition) <
               threshold)
            {
                MDagPath meshPath;
                MVGMayaUtil::getDagPathByName(meshIt->first.c_str(), meshPath);
                _intersectedComponent.type = MFn::kMeshEdgeComponent;
                _intersectedComponent.meshPath = meshPath;
                _intersectedComponent.vertex = NULL;
                _intersectedComponent.edge = &(*edgeIt);
                return true;
            }
        }
    }
    return false;
}

} // namespace
