#include "mayaMVG/maya/context/MVGManipulatorCache.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <list>

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

bool MVGManipulatorCache::checkIntersection(const double tolerance, const MPoint& mouseCSPosition,
                                            const bool checkBlindData)
{
    // Check
    if(checkBlindData)
    {
        if(isIntersectingBlindData(tolerance, mouseCSPosition))
            return true;
    }
    if(isIntersectingPoint(tolerance, mouseCSPosition))
        return true;
    if(isIntersectingEdge(tolerance, mouseCSPosition))
        return true;
    clearIntersectedComponent();
    return false;
}

const MVGManipulatorCache::MVGComponent& MVGManipulatorCache::getIntersectedComponent() const
{
    return _intersectedComponent;
}

const MFn::Type MVGManipulatorCache::getIntersectionType() const
{
    return _intersectedComponent.type;
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
    // List all meshes currently stored in meshData
    std::list<std::string> meshesList;
    for(std::map<std::string, MeshData>::iterator it = _meshData.begin(); it != _meshData.end();
        ++it)
        meshesList.push_back(it->first);

    // Update all meshes
    std::vector<MVGMesh> meshes = MVGMesh::listAllMeshes();
    std::vector<MVGMesh>::const_iterator it = meshes.begin();
    for(; it != meshes.end(); ++it)
    {
        // Remove mesh updated to only clear meshes that have been removed
        meshesList.remove(it->getDagPath().fullPathName().asChar());
        rebuildMeshCache(it->getDagPath());
    }

    // Remove data for meshes that does not exist anymore
    for(std::list<std::string>::iterator meshIt = meshesList.begin(); meshIt != meshesList.end();
        ++meshIt)
        _meshData.erase(*meshIt);
}

void MVGManipulatorCache::rebuildMeshCache(const MDagPath& path)
{
    if(!path.isValid())
        return;
    MVGMesh mesh(path);
    // Remove non active mesh
    if(!mesh.isActive())
    {
        std::map<std::string, MeshData>::iterator foundIt =
            _meshData.find(path.fullPathName().asChar());
        if(foundIt != _meshData.end())
            _meshData.erase(foundIt);
        return;
    }
    // Retrieve selectedComponent info
    MDagPath meshPath = _selectedComponent.meshPath;
    MFn::Type type = MFn::kInvalid;
    int index = -1;
    if(meshPath == path)
    {
        type = _selectedComponent.type;
        if(type == MFn::kBlindData || type == MFn::kMeshVertComponent)
            index = _selectedComponent.vertex->index;
        if(type == MFn::kMeshEdgeComponent)
            index = _selectedComponent.edge->index;
    }

    // Clear component associated to meshData
    if(path == _intersectedComponent.meshPath)
        clearIntersectedComponent();
    if(path == _selectedComponent.meshPath)
        clearSelectedComponent();

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
    const std::string pathsString = path.fullPathName().asChar();
    _meshData[pathsString] = MeshData();
    MeshData& newMeshData = _meshData[pathsString];
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

    if(meshPath == path)
        updateSelectedComponent(meshPath, type, index);
}

void MVGManipulatorCache::setSelectedComponent(const MVGComponent& selectedComponent)
{
    _selectedComponent = selectedComponent;
}

/**
 * Retrieve the selected component after a rebuild cache
 * The pointers in the Component are not valid anymore
 *
 * @param meshPath : path of the former selected component
 * @param type : type of the former selected component
 * @param index : index of the former selected component (edge or vertex)
 */
void MVGManipulatorCache::updateSelectedComponent(const MDagPath& meshPath, const MFn::Type type,
                                                  const int index)
{
    MVGComponent component;
    component.type = type;
    component.meshPath = meshPath;

    const std::string meshPathString = meshPath.fullPathName().asChar();
    if(type == MFn::kMeshVertComponent || type == MFn::kBlindData)
    {
        std::map<std::string, MeshData>::iterator it = _meshData.find(meshPathString);
        if(it == _meshData.end())
            return;
        std::vector<VertexData>& verticesArray = _meshData[meshPathString].vertices;
        if(verticesArray.size() <= index)
            return;
        component.vertex = &(verticesArray[index]);
    }
    _selectedComponent = component;
}

bool MVGManipulatorCache::isIntersectingBlindData(const double tolerance,
                                                  const MPoint& mouseCSPosition)
{
    if(_meshData.empty())
        return false;
    // compute tolerance
    const double threshold =
        (tolerance * _activeCamera.getZoom()) / (double)_activeView.portWidth();
    const int cameraID = _activeCamera.getId();
    // check each mesh vertices
    std::map<std::string, MeshData>::iterator meshIt = _meshData.begin();
    for(; meshIt != _meshData.end(); ++meshIt)
    {
        std::vector<VertexData>& vertices = meshIt->second.vertices;
        std::vector<VertexData>::iterator vertexIt = vertices.begin();
        for(; vertexIt < vertices.end(); ++vertexIt)
        {
            std::map<int, MPoint>::const_iterator blindDataIt = vertexIt->blindData.find(cameraID);
            if(blindDataIt == vertexIt->blindData.end())
                continue;
            MPoint pointCSPosition = blindDataIt->second;
            // check if we intersect w/ the vertex position
            if(mouseCSPosition.x <= pointCSPosition.x + threshold &&
               mouseCSPosition.x >= pointCSPosition.x - threshold &&
               mouseCSPosition.y <= pointCSPosition.y + threshold &&
               mouseCSPosition.y >= pointCSPosition.y - threshold)
            {
                MDagPath meshPath;
                MVGMayaUtil::getDagPathByName(meshIt->first.c_str(), meshPath);
                _intersectedComponent.type = MFn::kBlindData;
                _intersectedComponent.meshPath = meshPath;
                _intersectedComponent.vertex = &(*vertexIt);
                _intersectedComponent.edge = NULL;
                return true;
            }
        }
    }
    return false;
}

bool MVGManipulatorCache::isIntersectingPoint(const double tolerance, const MPoint& mouseCSPosition)
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

bool MVGManipulatorCache::isIntersectingEdge(const double tolerance, const MPoint& mouseCSPosition)
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
