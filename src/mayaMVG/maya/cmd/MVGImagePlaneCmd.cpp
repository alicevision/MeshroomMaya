#include "MVGImagePlaneCmd.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>
#include <maya/MPlugArray.h>

namespace
{ // empty namespace

static const char* panelFlag = "-p";
static const char* panelFlagLong = "-panel";
static const char* loadFlag = "-l";
static const char* loadFlagLong = "-load";
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
    s.addFlag(panelFlag, panelFlagLong, MSyntax::kString);
    s.addFlag(loadFlag, loadFlagLong);
    s.enableEdit(false);
    s.enableQuery(false);
    return s;
}

MStatus MVGImagePlaneCmd::doIt(const MArgList& args)
{
    MStatus status;
    MSyntax syntax = MVGImagePlaneCmd::newSyntax();
    MArgDatabase argData(syntax, args);

    if(!argData.isFlagSet(panelFlag))
    {
        LOG_ERROR("Need panel name to load image")
        return MS::kFailure;
    }

    MString panel;
    argData.getFlagArgument(panelFlag, 0, panel);

    if(argData.isFlagSet(loadFlag))
    {
        // Retrieve current camera in the specified Panel
        MString cmd;
        cmd.format("modelPanel -q -camera \"^1s\"", panel);
        MString currentCamera;
        MGlobal::executeCommand(cmd, currentCamera);

        // Retrieve image plane attached to camera
        MSelectionList list;
        list.add(currentCamera);
        MDagPath dagPath;
        list.getDagPath(0, dagPath);
        dagPath.extendToShape();

        MFnDagNode fnCamera(dagPath, &status);
        MPlug imagePlanePlug = fnCamera.findPlug("imagePlane", status);
        CHECK_RETURN_STATUS(status)
        MPlug imagePlug = imagePlanePlug.elementByLogicalIndex(0, &status);
        MPlugArray connectedPlugs;
        imagePlug.connectedTo(connectedPlugs, true, true, &status);
        CHECK_RETURN_STATUS(status)
        if(connectedPlugs.length() == 0)
        {
            LOG_ERROR("No plug connected to the plug")
            return MS::kFailure;
        }
        MDagPath imagePlaneShapeDagPath;
        status = MDagPath::getAPathTo(connectedPlugs[0].node(), imagePlaneShapeDagPath);
        CHECK(status)

        MFnDagNode fnImagePlane(imagePlaneShapeDagPath, &status);
        MPlug imageNamePlug = fnImagePlane.findPlug("imageName", &status);
        CHECK_RETURN_STATUS(status)
        MString imageNameValue;
        imageNamePlug.getValue(imageNameValue);

        // Set "imageName" attribute on image plane
        MString imagePath = fnCamera.findPlug(MVGCamera::_MVG_IMAGE_PATH, &status).asString();
        CHECK_RETURN_STATUS(status)
        if(imageNameValue != imagePath)
        {
            imageNamePlug.setValue(imagePath);

            // Update cache
            MVGProject project(MVGProject::_PROJECT);
            const std::string lastLoadedCam = project.getLastLoadedCameraInView(panel.asChar());
            project.updateImageCache(currentCamera.asChar(), lastLoadedCam);
            project.setLastLoadedCameraInView(panel.asChar(), currentCamera.asChar());
        }
    }

    return status;
}

bool MVGImagePlaneCmd::isUndoable() const
{
    return false;
}

} // namespace
