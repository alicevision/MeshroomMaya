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
	: _filter((QObject*)MVGMayaUtil::getMVGWindow(), this)
	, _filterLV((QObject*)MVGMayaUtil::getMVGViewportLayout(MVGProjectWrapper::instance().panelModel().at(0).toStdString().c_str()), this)
	, _filterRV((QObject*)MVGMayaUtil::getMVGViewportLayout(MVGProjectWrapper::instance().panelModel().at(1).toStdString().c_str()), this)
	, _editMode(eModeCreate)
	, _keyPressed(eKeyNone)
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

bool MVGContext::eventFilter(QObject *obj, QEvent *e)
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
			case Qt::Key_Control:
			{
				_keyPressed = eKeyCtrl;
				M3dView view = M3dView::active3dView();
				view.refresh(true, true);
				break;
			}
			case Qt::Key_Shift:
			{
				_keyPressed = eKeyShift;
				M3dView view = M3dView::active3dView();
				view.refresh(true, true);
				break;
			}
			case Qt::Key_Escape:
			{
				M3dView view = M3dView::active3dView();

				DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
				data->buildPoints2D.clear();
				view.refresh(true, true);
				break;
			}
			default:
				return false;
		}
	}
	// key released
	else if(e->type() == QEvent::KeyRelease) {
		Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
		 QKeyEvent * keyevent = static_cast<QKeyEvent*>(e);
		 if (keyevent->isAutoRepeat())
		 	return false;
		 
		switch(keyevent->key()) {
			case Qt::Key_Control:
			{
				if(!(modifiers & Qt::ShiftModifier))
					_keyPressed = eKeyNone;
				else
					_keyPressed = eKeyShift;
				M3dView view = M3dView::active3dView();
				view.refresh(true, true);
				break;
			}
			case Qt::Key_Shift:
			{
				if(!(modifiers & Qt::ControlModifier))
					_keyPressed = eKeyNone;
				else
					_keyPressed = eKeyCtrl;
				M3dView view = M3dView::active3dView();
				view.refresh(true, true);
				break;
			}
		}
	}
	// mouse button pressed
	else if (e->type() == QEvent::MouseButtonPress) {
		QMouseEvent * mouseevent = static_cast<QMouseEvent*>(e);
		// middle button: initialize camera pan
		if((mouseevent->button() & Qt::MidButton)) {
			if(!_eventData.cameraPath.isValid())
				return false;
			MFnCamera fnCamera(_eventData.cameraPath);
			_eventData.onPressMousePos = mouseevent->pos();
			_eventData.onPressCameraHPan = fnCamera.horizontalPan();
			_eventData.onPressCameraVPan = fnCamera.verticalPan();
			_eventData.isDragging = true;
		}
	}
	// mouse button moved
	else if(e->type() == QEvent::MouseMove) {
		if(!_eventData.cameraPath.isValid())
			return false;
		if(!_eventData.isDragging)
			return false;
		MFnCamera camera(_eventData.cameraPath);
		// compute pan offset
		QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
		QPointF offset_screen = _eventData.onPressMousePos - mouseevent->posF();
		const double viewport_width = qobject_cast<QWidget*>(obj)->width();
		QPointF offset = (offset_screen / viewport_width) * camera.horizontalFilmAperture() * camera.zoom();
		camera.setHorizontalPan(_eventData.onPressCameraHPan + offset.x());
		camera.setVerticalPan(_eventData.onPressCameraVPan - offset.y());
	}
	// mouse button released
	else if(e->type() == QEvent::MouseButtonRelease) {
		_eventData.isDragging = false;
	}
	// mouse wheel rolled
	else if (e->type() == QEvent::Wheel) {
		if(!_eventData.cameraPath.isValid())
			return false;
		// compute & set zoom value
		QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
		QWheelEvent* wheelevent = static_cast<QWheelEvent*>(e);
		MFnCamera camera(_eventData.cameraPath);
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
		// Check if we are entering the MVGWindow
//		QVariant window = obj->property("mvg_window");
//		if(window.type() != QVariant::Invalid)
//		{
//			MVGProjectWrapper::instance().rebuildAllMeshesCacheFromMaya();
//			MVGProjectWrapper::instance().rebuildCacheFromMaya();
//			return false;
//		}
		// check if we are entering an MVG panel
		QVariant panelName = obj->property("mvg_panel");
		if(panelName.type()== QVariant::Invalid)
			return false;
		// find & register the associated camera path
		MVGMayaUtil::getCameraInView(_eventData.cameraPath, MQtUtil::toMString(panelName.toString()));
		if(!_eventData.cameraPath.isValid())
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
