#include "meshroomMaya/maya/mesh/MVGMeshEditFactory.hpp"
#include "meshroomMaya/core/MVGMesh.hpp"
#include "meshroomMaya/core/MVGLog.hpp"
#include <maya/MGlobal.h>
#include <maya/MIOStream.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <cassert>

namespace meshroomMaya
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

void MVGMeshEditFactory::setClearBlindData(const bool clear)
{
    _clearBD = clear;
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
        case kAddFace:
        {
            int index;
            mesh.addPolygon(_worldPositions, index);
            MIntArray facePointsIndexes = mesh.getFaceVertices(index);
            // Update componentIDs
            _componentIDs.clear();
            _componentIDs = facePointsIndexes;
            if(_cameraPositions.length() == 2)
            {
                _componentIDs.clear();
                _componentIDs.append(facePointsIndexes[2]);
                _componentIDs.append(facePointsIndexes[3]);
            }
            break;
        }
        case kMove:
        {
            // move
            if(_componentIDs.length() == _worldPositions.length())
            {
                for(size_t i = 0; i < _componentIDs.length(); ++i)
                {
                    CHECK(mesh.setPoint(_componentIDs[i], _worldPositions[i]))
                    if(_clearBD)
                        CHECK(mesh.unsetBlindData(_componentIDs[i]));
                }
            }
            if(!_clearBD)
            {
                // set blind data
                assert(_componentIDs.length() == _cameraPositions.length());
                for(size_t i = 0; i < _componentIDs.length(); ++i)
                    CHECK(mesh.setBlindDataPerCamera(_componentIDs[i], _cameraID,
                                                     _cameraPositions[i]))
            }
            break;
        }
        case kClearBD:
        {
            for(size_t i = 0; i < _componentIDs.length(); ++i)
                CHECK(mesh.unsetBlindData(_componentIDs[i]));
            break;
        }
    }

    return status;
}

} // namespace
