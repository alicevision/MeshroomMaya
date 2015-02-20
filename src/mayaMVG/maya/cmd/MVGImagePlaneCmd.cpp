#include "MVGImagePlaneCmd.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>

namespace
{ // empty namespace

static const char* nameFlag = "-n";
static const char* nameFlagLong = "-name";
static const char* imageFlag = "-i";
static const char* imageFlagLong = "-image";

} // empty namespace
namespace mayaMVG
{

MString MVGImagePlaneCmd::_name("MVGImagePlaneCmd");

void* MVGImagePlaneCmd::creator()
{
    return new MVGImagePlaneCmd();
}

MSyntax MVGImagePlaneCmd::newSyntax()
{
    MSyntax s;
    s.addFlag(nameFlag, nameFlagLong, MSyntax::kString);
    s.addFlag(imageFlag, imageFlagLong, MSyntax::kString);
    s.enableEdit(false);
    s.enableQuery(false);
    return s;
}

MStatus MVGImagePlaneCmd::doIt(const MArgList& args)
{
    MStatus status;
    MSyntax syntax = MVGImagePlaneCmd::newSyntax();
    MArgDatabase argData(syntax, args);

    // Check for image plane name
    if(!argData.isFlagSet(nameFlag))
    {

        LOG_ERROR("Image plane name is compulsory")
        return MS::kFailure;
    }
    MString imagePlaneName;
    argData.getFlagArgument(nameFlag, 0, imagePlaneName);

    // Load image plane
    if(argData.isFlagSet(imageFlag))
    {
        MString imageName;
        argData.getFlagArgument(imageFlag, 0, imageName);

        // Retrieve imagePlaneNode
        MSelectionList list;
        list.add(imagePlaneName);
        MDagPath dagPath;
        list.getDagPath(0, dagPath);
        MFnDagNode fnImagePlane(dagPath);
        MPlug imageNamePlug = fnImagePlane.findPlug("imageName", &status);
        CHECK_RETURN_STATUS(status)
        imageNamePlug.setValue(imageName);
    }

    return status;
}

bool MVGImagePlaneCmd::isUndoable() const
{
    return false;
}

} // namespace
