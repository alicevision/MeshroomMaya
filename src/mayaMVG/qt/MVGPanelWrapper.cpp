#include "mayaMVG/qt/MVGPanelWrapper.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/MColor.h>
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include <QWidget>

namespace mayaMVG
{

MVGPanelWrapper::MVGPanelWrapper(const QString& name, const QString& label, const QColor& color)
    : _name(name)
    , _label(label)
    , _isVisible(true)
    , _defaultColor(color)
{
    updateStylesheet();
}

MVGPanelWrapper::MVGPanelWrapper(const QString& name)
    : _name(name)
    , _label(name)
    , _isVisible(true)
{
}

MVGPanelWrapper::~MVGPanelWrapper()
{
}

void MVGPanelWrapper::setLabel(QString label)
{
    _label = label;
    Q_EMIT labelChanged();
}

bool MVGPanelWrapper::isPointCloudDisplayed() const
{
    int result;
    MString command;
    command.format("modelEditor -q -dynamics ^1s", MString(_name.toStdString().c_str()));
    MStatus status = MGlobal::executeCommand(command, result, false, false);
    CHECK(status)
    return result;
}

void MVGPanelWrapper::displayPointCloud(const bool display)
{
    MStatus status;
    MString displayString;
    displayString = display ? "true" : "false";
    MString command;
    status = command.format("modelEditor -e -dynamics ^1s ^2s", displayString,
                            MString(_name.toStdString().c_str()));
    CHECK(status)
    status = MGlobal::executeCommand(command, false, false);
    CHECK(status)

    Q_EMIT isPointCloudDisplayedChanged();
}

void MVGPanelWrapper::emitIsPointCloudDisplayedChanged()
{
    Q_EMIT isPointCloudDisplayedChanged();
}

QColor MVGPanelWrapper::getColor() const 
{
    // Retrieve color from the MVGCameraPointsLocator of the project 
    // or the default color if this locator does not exist (yet)
    MObject locator;
    MVGMayaUtil::getObjectByName(MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), locator);
    if(locator.isNull())
        return _defaultColor;
    MColor color;
    // Use panel name to find the color attribute
    MVGMayaUtil::getColorAttribute(locator, (_name.toStdString() + "Color").c_str(), color);
    return MVGMayaUtil::fromMColor(color);
}

void MVGPanelWrapper::setColor(const QColor& color)
{
    if(getColor() == color)
        return;
    MObject locator;
    MStatus status = MVGMayaUtil::getObjectByName(MVGProject::_CAMERA_POINTS_LOCATOR.c_str(), locator);
    CHECK_RETURN(status);
    // Use panel name to find the color attribute
    MVGMayaUtil::setColorAttribute(locator, (_name.toStdString() + "Color").c_str(), MVGMayaUtil::fromQColor(color), false);
}

void MVGPanelWrapper::updateStylesheet()
{
    // Get the panel parent's widget and update its style
    QWidget* view = MVGMayaUtil::getMVGViewportLayout(_name.toStdString().c_str())->parentWidget();
    const QString style = "border: none; border-bottom: 5px solid " + getColor().name();
    view->setStyleSheet(style);
    view->ensurePolished();  // Force Qt to update this widget
}

void MVGPanelWrapper::onColorAttributeChanged()
{
    updateStylesheet();
    Q_EMIT colorChanged();
}

} // namepspace
