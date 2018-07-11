#include "MVGSelectClosestCamCmd.hpp"
#include "meshroomMaya/core/MVGProject.hpp"
#include "meshroomMaya/core/MVGCamera.hpp"
#include "meshroomMaya/maya/MVGMayaUtil.hpp"

#include <maya/MMatrix.h>

#include <cmath>


namespace meshroomMaya
{   

MString MVGSelectClosestCamCmd::_name("MVGSelectClosestCamCmd");

void* MVGSelectClosestCamCmd::creator() 
{
    return new MVGSelectClosestCamCmd();
}

MStatus MVGSelectClosestCamCmd::doIt(const MArgList& args)
{
    // Retrieve perspective matrix
    MDagPath perspDagPath;
    MVGMayaUtil::getDagPathByName("perspShape", perspDagPath);

    // Browse all MVG cameras
    std::multimap<float, std::string> camMap;
    std::vector<MVGCamera> mvgCameras = MVGCamera::getCameras();
    for(std::vector<MVGCamera>::iterator it = mvgCameras.begin(); it != mvgCameras.end(); ++it)
    {
        // Compute Frobenius norm
        MDagPath camDagPath = it->getDagPath();
        MMatrix matrix = perspDagPath.inclusiveMatrix() - camDagPath.inclusiveMatrix();
        float norm = 0;
        for(size_t i = 0; i < 4; ++i)
            for(size_t j = 0; j < 4; ++j)
                norm += std::pow(matrix[i][j], 2.0);
        camMap.insert(std::make_pair(norm, camDagPath.partialPathName().asChar()));
    }
    if(!camMap.empty())
    {
        MVGProject project(MVGProject::_PROJECT);
        std::vector<std::string> cams(1, camMap.begin()->second);
        project.selectCameras(cams);
    }
    return MS::kSuccess;
}

}