#include "MVGPanelWrapper.h"

#include "mayaMVG/core/MVGLog.h"

namespace mayaMVG {

static void modelEditorChangedCB(void* userData)
{
	MVGPanelWrapper* panel = static_cast<MVGPanelWrapper*>(userData);
	if(!panel)
		return;

	panel->emitIsPointCloudDisplayedChanged();
}

MVGPanelWrapper::MVGPanelWrapper(QString name, QString label)
	: _name(name)
	, _label(label)
	, _isVisible(true)
{
	_mayaCallbackIds.append(MEventMessage::addEventCallback("modelEditorChanged", modelEditorChangedCB, this));
}

MVGPanelWrapper::MVGPanelWrapper(QString name)
{
	MVGPanelWrapper(name, name);	
}

MVGPanelWrapper::~MVGPanelWrapper()
{
	MMessage::removeCallbacks(_mayaCallbackIds);
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
	status = command.format("modelEditor -e -dynamics ^1s ^2s", displayString, MString(_name.toStdString().c_str()));
	CHECK(status)
	status = MGlobal::executeCommand(command, false, false);
	CHECK(status)

	Q_EMIT isPointCloudDisplayedChanged();
}

void MVGPanelWrapper::emitIsPointCloudDisplayedChanged()
{
	Q_EMIT isPointCloudDisplayedChanged();
}
} // namepspace

