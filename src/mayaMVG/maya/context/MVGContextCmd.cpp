#include "mayaMVG/maya/context/MVGContextCmd.hpp"
#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/core/MVGMesh.hpp"

namespace
{ // empty namespace

static const char* rebuildFlag = "-r";
static const char* rebuildFlagLong = "-rebuild";
static const char* meshFlag = "-m";
static const char* meshFlagLong = "-mesh";

} // empty namespace

namespace mayaMVG
{

MString MVGContextCmd::name("mayaMVGTool");

MVGContextCmd::MVGContextCmd()
    : _context(NULL)
{
}

MPxContext* MVGContextCmd::makeObj()
{
    _context = new MVGContext();
    return _context;
}

void* MVGContextCmd::creator()
{
    return new MVGContextCmd;
}

MStatus MVGContextCmd::doEditFlags()
{
    MArgParser argData = parser();
    // -rebuild: rebuild cache
    if(argData.isFlagSet(rebuildFlag))
    {
        MVGManipulatorCache& cache = _context->getCache();
        if(argData.isFlagSet(meshFlag))
        {
            MString meshName;
            argData.getFlagArgument(meshFlag, 0, meshName);
            MVGMesh mesh(meshName);
            cache.rebuildMeshCache(mesh.getDagPath());
            return MS::kSuccess;
        }
        cache.rebuildMeshesCache();
    }
    return MS::kSuccess;
}

MStatus MVGContextCmd::doQueryFlags()
{
    return MS::kSuccess;
}

MStatus MVGContextCmd::appendSyntax()
{
    MSyntax mySyntax = syntax();
    if(MS::kSuccess != mySyntax.addFlag(rebuildFlag, rebuildFlagLong))
    {
        return MS::kFailure;
    }
    if(MS::kSuccess != mySyntax.addFlag(meshFlag, meshFlagLong, MSyntax::kString))
    {
        return MS::kFailure;
    }
    return MS::kSuccess;
}

} // namespace
