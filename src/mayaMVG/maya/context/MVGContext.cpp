#include "mayaMVG/maya/context/MVGContext.h"
#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/maya/context/MVGMoveManipulator.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/qt/MVGEventFilter.h"
#include "mayaMVG/qt/MVGQt.h"
#include <maya/MQtUtil.h>
#include <maya/MFnCamera.h>

using namespace mayaMVG;

MVGContext::MVGContext() 
	: _filter((QObject*)MVGMayaUtil::getMVGWindow(), this, &_eventData)
	, _filterLV((QObject*)MVGMayaUtil::getMVGViewportLayout("mvgLPanel"), this, &_eventData)
	, _filterRV((QObject*)MVGMayaUtil::getMVGViewportLayout("mvgRPanel"), this, &_eventData)
	, _editMode(eModeCreate)
{
	setTitleString("MVG tool");
}

void MVGContext::toolOnSetup(MEvent& event)
{
	updateManipulators();
}

void MVGContext::toolOffCleanup()
{
	deleteManipulators();
	MPxContext::toolOffCleanup();
}

void MVGContext::getClassName(MString & name) const
{
	name.set("mayaMVGTool");
}

void MVGContext::updateManipulators()
{
	// delete all manipulators
	deleteManipulators();
	// then add a new one, depending on edit mode
	MStatus status;
	MObject manipObject;
	switch(_editMode) {
		case eModeCreate: {
			MVGCreateManipulator* manip = static_cast<MVGCreateManipulator*>(
				MPxManipulatorNode::newManipulator("MVGCreateManipulator", manipObject, &status));
			if(status && !manip)
				return;
			manip->setContext(this);
			break;
		}
		case eModeMove: {
			MVGMoveManipulator* manip = static_cast<MVGMoveManipulator*>(
				MPxManipulatorNode::newManipulator("MVGMoveManipulator", manipObject, &status));
			if(status && !manip)
				return;
			manip->setContext(this);
			break;
		}
		default:
			return;
	}
	if(status)
		addManipulator(manipObject);
}

bool MVGContext::eventFilter(QObject *obj, QEvent *e, void* eventData)
{
	// key pressed
	if(e->type() == QEvent::KeyPress) {
		QKeyEvent* keyevent = static_cast<QKeyEvent*>(e);
		if (keyevent->isAutoRepeat())
			return false;
		switch(keyevent->key()) {
			case Qt::Key_F:
				LOG_INFO("FIT")
				break;
			case Qt::Key_M:	
				LOG_INFO("MOVE MODE")
				_editMode = eModeMove;
				updateManipulators();
				break;
			case Qt::Key_C:
				LOG_INFO("CREATE MODE")
				_editMode = eModeCreate;
				updateManipulators();
				break;
			default:
				return false;
		}
	}
	// key released
	else if(e->type() == QEvent::KeyRelease) {
		// QKeyEvent * keyevent = static_cast<QKeyEvent*>(e);
		// if (keyevent->isAutoRepeat())
		// 	return false;
	}
	// mouse button pressed
	else if (e->type() == QEvent::MouseButtonPress) {
		QMouseEvent * mouseevent = static_cast<QMouseEvent*>(e);
		// middle button: initialize camera pan
		if((mouseevent->button() & Qt::MidButton)) {
			EventData* data = static_cast<EventData*>(eventData);
			if(!data->cameraPath.isValid())
				return false;
			MFnCamera fnCamera(data->cameraPath);
			data->onPressMousePos = mouseevent->pos();
			data->onPressCameraHPan = fnCamera.horizontalPan();
			data->onPressCameraVPan = fnCamera.verticalPan();
			data->isDragging = true;
		}
	}
	// mouse button moved
	else if(e->type() == QEvent::MouseMove) {
		EventData* data = static_cast<EventData*>(eventData);
		if(!data->cameraPath.isValid())
			return false;
		if(!data->isDragging)
			return false;
		MFnCamera camera(data->cameraPath);
		// compute pan offset
		QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
		QPointF offset_screen = data->onPressMousePos - mouseevent->posF();
		const double viewport_width = qobject_cast<QWidget*>(obj)->width();
		QPointF offset = (offset_screen / viewport_width) * camera.horizontalFilmAperture() * camera.zoom();
		camera.setHorizontalPan(data->onPressCameraHPan + offset.x());
		camera.setVerticalPan(data->onPressCameraVPan - offset.y());
	}
	// mouse button released
	else if(e->type() == QEvent::MouseButtonRelease) {
		EventData* data = static_cast<EventData*>(eventData);
		data->isDragging = false;
	}
	// mouse wheel rolled
	else if (e->type() == QEvent::Wheel) {
		EventData* data = static_cast<EventData*>(eventData);
		if(!data->cameraPath.isValid())
			return false;
		// compute & set zoom value
		QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
		QWheelEvent* wheelevent = static_cast<QWheelEvent*>(e);
		MFnCamera camera(data->cameraPath);
		QWidget* widget = qobject_cast<QWidget*>(obj);
		const double viewportWidth = widget->width();
		const double viewportHeight = widget->height();
		static const double wheelStep = 1.15;
		const double previousZoom = camera.zoom();
		double newZoom = wheelevent->delta() > 0 ? previousZoom / wheelStep : previousZoom * wheelStep;
		camera.setZoom(std::max(newZoom, 0.0001));
		const double scaleRatio = newZoom / previousZoom;
		// compute & set pan offset
		QPointF center_ratio(0.5, 0.5 * viewportHeight / viewportWidth);
		QPointF mouse_ratio_center = (center_ratio - (mouseevent->posF() / viewportWidth));
		QPointF mouse_maya_center = mouse_ratio_center * camera.horizontalFilmAperture() * previousZoom;
		QPointF mouseAfterZoo_maya_center = mouse_maya_center * scaleRatio;
		QPointF offset = mouse_maya_center - mouseAfterZoo_maya_center;
		camera.setHorizontalPan(camera.horizontalPan() - offset.x());
		camera.setVerticalPan(camera.verticalPan() + offset.y());
	}
	// mouse enters widget's boundaries
	else if (e->type() == QEvent::Enter) {
		// check if we are entering an MVG panel
		QVariant panelName = obj->property("mvg_panel");
		if(panelName.type()==QVariant::Invalid)
			return false;
		// find & register the associated camera path
		EventData* data = static_cast<EventData*>(eventData);
		MVGMayaUtil::getCameraInView(data->cameraPath, MQtUtil::toMString(panelName.toString()));
		if(!data->cameraPath.isValid())
			return false;
		// automagically set focus on this MVG panel
		MVGMayaUtil::setFocusOnView(MQtUtil::toMString(panelName.toString()));
	} 
	return false;
}

MVGEditCmd* MVGContext::newCmd() 
{
	return (MVGEditCmd *)newToolCommand();
}
