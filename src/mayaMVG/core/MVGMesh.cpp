#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/maya/cmd/MVGEditCmd.hpp"
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MSelectionList.h>
#include <maya/MDagModifier.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItDag.h>
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MPlug.h>
#include <maya/MArgList.h>
#include <cassert>
#include <cstring>

namespace mayaMVG
{

namespace
{ // empty namespace

void binaryToVectorData(const char* binaryData, const int binarySize,
                        std::vector<MVGMesh::ClickedCSPosition>& vectorData)
{
    const size_t nbElements = size_t(binarySize / sizeof(MVGMesh::ClickedCSPosition));
    vectorData.resize(nbElements);
    memcpy(vectorData.data(), binaryData, binarySize);
}

} // empty namespace

int MVGMesh::_blindDataID = 0; // FIXME
MString MVGMesh::_MVG = "mvg";

MVGMesh::MVGMesh(const std::string& dagPathAsString)
    : MVGNodeWrapper(dagPathAsString)
{
}

MVGMesh::MVGMesh(const MString& dagPathAsString)
    : MVGNodeWrapper(dagPathAsString)
{
}

MVGMesh::MVGMesh(const MDagPath& dagPath)
    : MVGNodeWrapper(dagPath)
{
}

MVGMesh::MVGMesh(const MObject& object)
    : MVGNodeWrapper(object)
{
}

bool MVGMesh::isValid() const
{
    if(_object == MObject::kNullObj ||
       !(_object.apiType() == MFn::kMesh || _object.apiType() == MFn::kMeshData))
        return false;
    return true;
}

MVGMesh MVGMesh::create(const std::string& name)
{
    MStatus status;
    MFnMesh fnMesh;

    // create empty mesh
    MPointArray vertexArray;
    MIntArray polygonCounts, polygonConnects;
    MObject transform = fnMesh.create(0, 0, vertexArray, polygonCounts, polygonConnects,
                                      MObject::kNullObj, &status);

    // Create blindData
    MStringArray longNames, shortNames, formatNames;
    longNames.append("binarySize");
    shortNames.append("size");
    formatNames.append("int");
    longNames.append("binaryData");
    shortNames.append("data");
    formatNames.append("binary");
    if(!fnMesh.isBlindDataTypeUsed(_blindDataID, &status))
        CHECK(fnMesh.createBlindDataType(_blindDataID, longNames, shortNames, formatNames))

    // register dag path
    MDagPath path;
    MDagPath::getAPathTo(transform, path);
    path.extendToShape();

    // connect to initialShadingGroup
    MObject sgObj;
    MSelectionList list;
    status = MGlobal::getSelectionListByName("initialShadingGroup", list);
    status = list.getDependNode(0, sgObj);
    MFnSet fnSet(sgObj, &status);
    status = fnSet.addMember(path);
    CHECK(status)

    // rename and return
    MVGMesh mesh(path);
    mesh.setName(name);
    return mesh;
}

// static
std::vector<MVGMesh> MVGMesh::listActiveMeshes()
{
    std::vector<MVGMesh> list;
    MDagPath path;
    MItDag it(MItDag::kDepthFirst, MFn::kMesh);
    for(; !it.isDone(); it.next())
    {
        MFnDagNode fn(it.currentItem());
        if(fn.isIntermediateObject())
            continue;
        fn.getPath(path);
        MVGMesh mesh(path);
        if(mesh.isValid() && mesh.isActive())
            list.push_back(mesh);
    }
    return list;
}

// static
std::vector<MVGMesh> MVGMesh::listAllMeshes()
{
    std::vector<MVGMesh> list;
    MDagPath path;
    MItDag it(MItDag::kDepthFirst, MFn::kMesh);
    for(; !it.isDone(); it.next())
    {
        MFnDagNode fn(it.currentItem());
        if(fn.isIntermediateObject())
            continue;
        fn.getPath(path);
        MVGMesh mesh(path);
        if(mesh.isValid())
            list.push_back(mesh);
    }
    return list;
}

void MVGMesh::setIsActive(const bool isActive) const
{
    MStatus status;

    // Check is flag exists
    MFnMesh fn(_dagpath);
    MPlug mvgPlug = fn.findPlug(_MVG, false, &status);
    if(!status && !isActive)
        return;
    if(!status && isActive)
    {
        // Create MayaMVG attribute
        MDagModifier dagModifier;
        MFnNumericAttribute nAttr;
        MObject mvgAttr = nAttr.create(_MVG, "mvg", MFnNumericData::kBoolean);
        status = dagModifier.addAttribute(_object, mvgAttr);
        CHECK(status)
        dagModifier.doIt();
        mvgPlug = fn.findPlug(_MVG, false, &status);
    }
    status = mvgPlug.setValue(isActive);
    CHECK(status)
    if(isActive)
    {
        status = MGlobal::executePythonCommand("from mayaMVG import scale");
        CHECK(status)
        MString cmd;
        // Retrieve transform node
        cmd.format("scale.getParent(\"^1s\")", _dagpath.fullPathName());
        MString transform;
        status = MGlobal::executePythonCommand(cmd, transform);
        // Freeze transform mesh
        cmd.format("makeIdentity -apply true \"^1s\"", transform);
        status = MGlobal::executeCommand(cmd);
        CHECK(status)
        // Lock node
        cmd.format("scale.lockNode(\"^1s\", True)", transform);
        status = MGlobal::executePythonCommand(cmd);
        CHECK(status)
    }
    else
    {
        status = MGlobal::executePythonCommand("from mayaMVG import scale");
        CHECK(status)
        MString cmd;
        // Retrieve transform node
        cmd.format("scale.getParent(\"^1s\")", _dagpath.fullPathName());
        MString transform;
        status = MGlobal::executePythonCommand(cmd, transform);
        // Unlock node
        status = MGlobal::executePythonCommand("from mayaMVG import scale");
        cmd.format("scale.lockNode(\"^1s\", False)", transform);
        status = MGlobal::executePythonCommand(cmd);
        CHECK(status)
    }
    // Rebuild cache
    MString cmd;
    cmd.format("^1s -e -rebuild -mesh \"^2s\" ^3s", MVGContextCmd::name, _dagpath.fullPathName(),
               MVGContextCmd::instanceName);
    status = MGlobal::executeCommand(cmd);
    CHECK(status)
}

bool MVGMesh::isActive() const
{
    MStatus status;
    // Check if the specific plug exists
    MFnMesh fn(_dagpath);
    MPlug mvgPlug = fn.findPlug(_MVG, false, &status);
    if(!status)
        return false;
    // Retrieve value
    bool value;
    status = mvgPlug.getValue(value);
    return value;
}

bool MVGMesh::addPolygon(const MPointArray& pointArray, int& index) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    if(pointArray.length() < 3)
        return false;
    fnMesh.addPolygon(pointArray, index, true, kMFnMeshPointTolerance, MObject::kNullObj, &status);
    CHECK_RETURN_VARIABLE(status, false)
    return true;
}

bool MVGMesh::deletePolygon(const int index) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK_RETURN_VARIABLE(status, false)
    status = fnMesh.deleteFace(index);
    CHECK_RETURN_VARIABLE(status, false)
    return true;
}

MStatus MVGMesh::getPoints(MPointArray& pointArray) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK_RETURN_STATUS(status);
    status = fnMesh.getPoints(pointArray, MSpace::kWorld);
    CHECK_RETURN_STATUS(status)
    return status;
}

int MVGMesh::getPolygonsCount() const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK(status);
    int count = fnMesh.numPolygons(&status);
    CHECK(status);
    return count;
}

int MVGMesh::getVerticesCount() const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK(status);
    int count = fnMesh.numVertices(&status);
    CHECK(status);
    return count;
}

MStatus MVGMesh::getPolygonVertices(const int polygonId, MIntArray& vertexList) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK_RETURN_STATUS(status);
    status = fnMesh.getPolygonVertices(polygonId, vertexList);
    CHECK_RETURN_STATUS(status)
    return status;
}

const MIntArray MVGMesh::getConnectedFacesToVertex(const int vertexId)
{
    MIntArray connectedFacesId;
    MStatus status;
    MItMeshVertex verticesIter(_object, &status);
    CHECK(status);
    int prev;
    status = verticesIter.setIndex(vertexId, prev);
    CHECK(status);
    status = verticesIter.getConnectedFaces(connectedFacesId);
    CHECK(status);
    return connectedFacesId;
}

const MIntArray MVGMesh::getConnectedFacesToEdge(const int edgeId)
{
    MIntArray connectedFacesId;
    MStatus status;
    MItMeshEdge edgesIter(_object, &status);
    CHECK(status);
    int prev;
    status = edgesIter.setIndex(edgeId, prev);
    CHECK(status);
    edgesIter.getConnectedFaces(connectedFacesId, &status);
    CHECK(status);
    return connectedFacesId;
}

const MIntArray MVGMesh::getFaceVertices(const int faceId)
{
    MStatus status;
    MItMeshPolygon faceIter(_object);
    int prev;
    status = faceIter.setIndex(faceId, prev);
    CHECK(status);
    MIntArray vertices;
    status = faceIter.getVertices(vertices);
    CHECK(status);
    return vertices;
}

MStatus MVGMesh::setPoint(const int vertexId, const MPoint& point) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    status = fnMesh.setPoint(vertexId, point, MSpace::kWorld);
    CHECK_RETURN_STATUS(status);
    return status;
}

MStatus MVGMesh::setPoints(const MIntArray& verticesIds, const MPointArray& points) const
{
    MStatus status;
    assert(verticesIds.length() == points.length());
    assert(_dagpath.isValid());
    MFnMesh fnMesh(_dagpath, &status);
    for(int i = 0; i < verticesIds.length(); ++i)
    {
        status = fnMesh.setPoint(verticesIds[i], points[i], MSpace::kWorld);
        CHECK_RETURN_STATUS(status);
    }
    fnMesh.syncObject();
    return status;
}

MStatus MVGMesh::getPoint(int vertexId, MPoint& point) const
{
    MStatus status;
    assert(_dagpath.isValid());
    MFnMesh fnMesh(_dagpath, &status);
    status = fnMesh.getPoint(vertexId, point, MSpace::kWorld);
    CHECK_RETURN_STATUS(status)
    return status;
}

MStatus MVGMesh::setBlindData(const int vertexId,
                              std::vector<ClickedCSPosition>& clickedCSPositions) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK_RETURN_STATUS(status);
    char* charData = reinterpret_cast<char*>(clickedCSPositions.data());
    const int binarySize = clickedCSPositions.size() * sizeof(ClickedCSPosition);
    CHECK_RETURN_STATUS(
        fnMesh.setIntBlindData(vertexId, MFn::kMeshVertComponent, _blindDataID, "size", binarySize))
    CHECK_RETURN_STATUS(fnMesh.setBinaryBlindData(vertexId, MFn::kMeshVertComponent, _blindDataID,
                                                  "data", charData, binarySize))
    return status;
}

MStatus MVGMesh::getBlindData(const int vertexId,
                              std::vector<ClickedCSPosition>& clickedCSPositions) const
{
    MStatus status;
    MFnMesh fnMesh(_object, &status);
    CHECK_RETURN_STATUS(status);
    if(!fnMesh.hasBlindData(MFn::kMeshVertComponent))
        return MS::kFailure;
    if(!fnMesh.hasBlindDataComponentId(vertexId, MFn::kMeshVertComponent, _blindDataID))
        return MS::kFailure;
    int binarySize;
    CHECK_RETURN_STATUS(
        fnMesh.getIntBlindData(vertexId, MFn::kMeshVertComponent, _blindDataID, "size", binarySize))
    MString stringData;
    CHECK_RETURN_STATUS(fnMesh.getBinaryBlindData(vertexId, MFn::kMeshVertComponent, _blindDataID,
                                                  "data", stringData))
    const char* binData = stringData.asChar(binarySize);
    binaryToVectorData(binData, binarySize, clickedCSPositions);
    return status;
}

MStatus MVGMesh::getBlindData(const int vertexId,
                              std::map<int, MPoint>& cameraToClickedCSPoints) const
{
    MStatus status;
    std::vector<ClickedCSPosition> data;
    if(!getBlindData(vertexId, data))
        return MS::kFailure;
    std::vector<ClickedCSPosition>::const_iterator it = data.begin();
    for(; it != data.end(); ++it)
        cameraToClickedCSPoints[it->cameraId] = MPoint(it->x, it->y);
    return status;
}

MStatus MVGMesh::unsetAllBlindData() const
{
    MStatus status;

    // Get all vertices
    MItMeshVertex vIt(_dagpath, MObject::kNullObj, &status);
    vIt.updateSurface();
    vIt.geomChanged();
    MIntArray componentId;
    while(!vIt.isDone())
    {
        const int index = vIt.index(&status);
        componentId.append(index);
        vIt.next();
    }
    MVGEditCmd* cmd = new MVGEditCmd();
    if(cmd)
    {
        cmd->clearBD(_dagpath, componentId);
        MArgList args;
        if(cmd->doIt(args))
            cmd->finalize();
    }
    delete cmd;

    return status;
}
MStatus MVGMesh::unsetBlindData(const int vertexId) const
{
    MStatus status;
    // TODO : use clearBlindData function in MFnMesh
    std::vector<ClickedCSPosition> vector;
    status = setBlindData(vertexId, vector);
    CHECK(status)
    return status;
}

MStatus MVGMesh::getBlindDataPerCamera(const int vertexId, const int cameraId,
                                       MPoint& point2D) const
{
    MStatus status;
    std::vector<ClickedCSPosition> data;
    status = getBlindData(vertexId, data);
    CHECK_RETURN_STATUS(status)
    for(std::vector<ClickedCSPosition>::iterator it = data.begin(); it != data.end(); ++it)
    {
        if(it->cameraId == cameraId)
        {
            point2D.x = it->x;
            point2D.y = it->y;
            return MS::kSuccess;
        }
    }
    return MS::kFailure;
}

MStatus MVGMesh::setBlindDataPerCamera(const int vertexId, const int cameraId,
                                       const MPoint& point2D) const
{
    MStatus status;
    std::vector<ClickedCSPosition> data;
    if(getBlindData(vertexId, data))
    {
        for(std::vector<ClickedCSPosition>::iterator it = data.begin(); it != data.end(); ++it)
        {
            if(it->cameraId == cameraId)
            {
                it->x = point2D.x;
                it->y = point2D.y;
                status = setBlindData(vertexId, data);
                CHECK(status)
                return status;
            }
        }
    }
    ClickedCSPosition newData;
    newData.cameraId = cameraId;
    newData.x = point2D.x;
    newData.y = point2D.y;
    data.push_back(newData);
    status = setBlindData(vertexId, data);
    CHECK(status)
    return status;
}

MStatus MVGMesh::unsetBlindDataPerCamera(const int vertexId, const int cameraId) const
{
    MStatus status;
    std::vector<ClickedCSPosition> data;
    status = getBlindData(vertexId, data);
    for(std::vector<ClickedCSPosition>::iterator it = data.begin(); it != data.end(); ++it)
    {
        if(it->cameraId == cameraId)
            data.erase(it);
    }
    status = setBlindData(vertexId, data);
    CHECK(status)
    return status;
}

} // namespace
