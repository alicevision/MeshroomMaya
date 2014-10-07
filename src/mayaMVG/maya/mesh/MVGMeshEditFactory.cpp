#include "mayaMVG/maya/mesh/MVGMeshEditFactory.hpp"
#include "mayaMVG/core/MVGMesh.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <cassert>

namespace mayaMVG
{

MVGMeshEditFactory::MVGMeshEditFactory()
{
    _componentIDs.clear();
}

void MVGMeshEditFactory::setMesh(const MObject& mesh)
{
    _meshObj = mesh;
}

void MVGMeshEditFactory::setComponentList(const MObject& componentList)
{
    _componentList = componentList;
}

void MVGMeshEditFactory::setComponentIDs(const MIntArray& componentIDs)
{
    _componentIDs = componentIDs;
}

void MVGMeshEditFactory::setWorldPositions(const MPointArray& worldPositions)
{
    _worldPositions = worldPositions;
}

void MVGMeshEditFactory::setCameraPositions(const MPointArray& cameraPositions)
{
    _cameraPositions = cameraPositions;
}

void MVGMeshEditFactory::setCameraID(const int cameraID)
{
    _cameraID = cameraID;
}

void MVGMeshEditFactory::setEditType(const EditType type)
{
    _editType = type;
}

MStatus MVGMeshEditFactory::doIt()
{
    MStatus status;
    MVGMesh mesh(_meshObj);
    if(!mesh.isValid())
        return MS::kFailure;

    switch(_editType)
    {
        case kCreate:
        {
            int index;
            mesh.addPolygon(_worldPositions, index);
            break;
        }
        case kMove:
        {
            assert(_componentIDs.length() == _cameraPositions.length());
            // set blind data
            for(size_t i = 0; i < _componentIDs.length(); ++i)
                CHECK(mesh.setBlindDataPerCamera(_componentIDs[i], _cameraID, _cameraPositions[i]))
            // move
            if(_componentIDs.length() == _worldPositions.length())
            {
                for(size_t i = 0; i < _componentIDs.length(); ++i)
                    CHECK(mesh.setPoint(_componentIDs[i], _worldPositions[i]))
            }
            break;
        }
    }
    return status;
}

} // namespace
