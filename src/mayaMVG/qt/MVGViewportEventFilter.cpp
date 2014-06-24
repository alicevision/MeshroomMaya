#include "mayaMVG/qt/MVGViewportEventFilter.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <QMouseEvent>
#include <QWidget>
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>
#include <maya/MQtUtil.h>

using namespace mayaMVG;

namespace { // empty namespace
	MStatus getCameraPathFromQbject(const QObject* obj, MDagPath& path) {
		if(!obj)
			return MS::kFailure;
		QVariant panelName = obj->property("mvg_panel");
		if(panelName == QVariant::Invalid)
			return MS::kFailure;
		return MVGMayaUtil::getCameraInView(path, MQtUtil::toMString(panelName.toString()));
	}
} // empty namespace


MVGViewportEventFilter::MVGViewportEventFilter(QObject* parent)
	: QObject(parent)
	, _tracking(false)
{
}

bool MVGViewportEventFilter::eventFilter(QObject * obj, QEvent * e)
{  
	// Image is fitted on width.
	QMouseEvent * mouseevent = static_cast<QMouseEvent *>(e);
	
	// Init pan and zoom
	if (e->type() == QEvent::MouseButtonPress)
	{
		// Camera Pan (Mid button)
		if((mouseevent->button() & Qt::MidButton))
		{
		  MDagPath cameraPath;
		  if(getCameraPathFromQbject(obj, cameraPath))
		  {
			MFnCamera camera(cameraPath);
			// register click position
			_clickPos = mouseevent->pos();
			// register camera film offset
			_cameraHPan = camera.horizontalPan();
			_cameraVPan = camera.verticalPan();
			// set as tracking
			_tracking = true;
		  }
		}
	}
	// Apply Pan 
	else if(e->type() == QEvent::MouseMove) 
	{
		if(!_tracking)
			return QObject::eventFilter(obj, e);

		QWidget* widget = qobject_cast<QWidget*>(obj);
		MDagPath cameraPath;
		if(getCameraPathFromQbject(obj, cameraPath)) 
		{
			MFnCamera camera(cameraPath);
			// compute mouse offset
			QPointF offset_screen = _clickPos - mouseevent->pos();
			const double viewport_width = widget->width();
			QPointF offset = (offset_screen / viewport_width) * camera.horizontalFilmAperture() * camera.zoom();

			camera.setHorizontalPan(_cameraHPan + offset.x());
			camera.setVerticalPan(_cameraVPan - offset.y());
		}
	}
	else if(e->type() == QEvent::MouseButtonRelease) 
	{
		// disable tracking
		_tracking = false;
	}
	// Apply zoom
	else if (e->type() == QEvent::Wheel)
	{
		// compute wheel offset
		QWheelEvent* wheelevent = static_cast<QWheelEvent*>(e);
		MDagPath cameraPath;
		if(getCameraPathFromQbject(obj, cameraPath))
		{
			MFnCamera camera(cameraPath);
			QWidget* widget = qobject_cast<QWidget*>(obj);

			const double viewport_width = widget->width();
			const double viewport_height = widget->height();
			const QPointF pan_maya(camera.horizontalPan(), camera.verticalPan());

			static const double wheelStep = 1.15;
			const double previousZoom = camera.zoom();
			double newZoom = wheelevent->delta() > 0 ? previousZoom / wheelStep : previousZoom * wheelStep;
			newZoom = std::max(newZoom, 0.0001);  // zoom max
			camera.setZoom( newZoom );
			const double scaleRatio = newZoom / previousZoom;

			// compute mouse offset
			QPointF center_ratio(0.5, 0.5 * viewport_height / viewport_width);
			QPointF mouse_ratio_center = (center_ratio - (mouseevent->posF() / viewport_width));
			QPointF mouse_maya_center = mouse_ratio_center * camera.horizontalFilmAperture() * previousZoom;
			QPointF mouseAfterZoo_maya_center = mouse_maya_center * scaleRatio;
			QPointF offset = mouse_maya_center - mouseAfterZoo_maya_center;

			camera.setHorizontalPan( camera.horizontalPan() - offset.x() );
			camera.setVerticalPan( camera.verticalPan() + offset.y() );
		}
	}
	else if (e->type() == QEvent::Enter) {
		// automagically set focus on mvg panel
		QVariant panelName = obj->property("mvg_panel");
		if(panelName.type()==QVariant::Invalid)
			return QObject::eventFilter(obj, e);
		MVGMayaUtil::setFocusOnView(MQtUtil::toMString(panelName.toString()));
	} else if ((e->type() == QEvent::Close)) {
		LOG_ERROR("PANEL CLOSED")
	}
	return QObject::eventFilter(obj, e);
}
