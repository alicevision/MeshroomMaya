#include "mayaMVG/maya/cmd/MVGCmd.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/qt/MVGMainWidget.hpp"
#include <QtDeclarative/QDeclarativeView>
#include <maya/MQtUtil.h>
#include <maya/MArgDatabase.h>

namespace
{ // empty namespace

static const char* projectPathFlag = "-p";
static const char* projectPathFlagLong = "-project";

} // empty namespace

namespace mayaMVG
{

MVGCmd::MVGCmd()
{
}

MVGCmd::~MVGCmd()
{
}

void* MVGCmd::creator()
{
    return new MVGCmd();
}

MSyntax MVGCmd::newSyntax()
{
    MSyntax s;
    s.addFlag(projectPathFlag, projectPathFlagLong, MSyntax::kString);
    s.enableEdit(false);
    s.enableQuery(false);
    return s;
}

MStatus MVGCmd::doIt(const MArgList& args)
{
    MStatus status = MS::kSuccess;

    // create maya window
    status = MVGMayaUtil::createMVGWindow();
    if(!status)
    {
        LOG_ERROR("Unable to create MVGWindow.")
        return status;
    }

    // get layout from
    QWidget* menuLayout = MVGMayaUtil::getMVGMenuLayout();
    if(!menuLayout)
    {
        LOG_ERROR("Unable to retrieve MVGMenuLayout.")
        return MS::kFailure;
    }

    // create maya MVGContext
    status = MVGMayaUtil::createMVGContext();
    if(!status)
    {
        LOG_ERROR("Unable to create MVGContext.")
        return status;
    }

    // set window properties
    QWidget* mayaWindow = MVGMayaUtil::getMVGWindow();
    if(!mayaWindow)
    {
        LOG_ERROR("Unable to retrieve MVGWindow.")
        return MS::kFailure;
    }
    mayaWindow->setProperty("mvg_window", QString("MVGWindow"));

    // set layout properties
    const QString& leftPanelName = "mvgLPanel";
    const QString& rightPanelName = "mvgRPanel";
    QWidget* leftViewport = MVGMayaUtil::getMVGViewportLayout(leftPanelName.toStdString().c_str());
    QWidget* rightViewport =
        MVGMayaUtil::getMVGViewportLayout(rightPanelName.toStdString().c_str());
    if(!leftViewport || !rightViewport)
    {
        LOG_ERROR("Unable to retrieve MVG panels.");
        return MS::kFailure;
    }
    leftViewport->setProperty("mvg_panel", leftPanelName);
    rightViewport->setProperty("mvg_panel", rightPanelName);

    // create MVG menu
    MVGMainWidget* mainWidget = new MVGMainWidget(menuLayout);
    MQtUtil::addWidgetToMayaLayout(mainWidget->getView(), menuLayout);

    return status;
}

} // namespace
