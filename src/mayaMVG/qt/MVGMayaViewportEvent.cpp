#include "MVGMayaViewportEvent.h"

#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QWidget>
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>

using namespace mayaMVG;


namespace {
	MStatus getCameraPathFromQbject(const QObject* obj, MDagPath& path) {
		if(!obj)
			return MS::kFailure;
		QVariant panelName = obj->property("mvg_panel");
		if(panelName.type()==QVariant::Invalid)
			return MS::kFailure;
		return (panelName.toString()=="left") ? 
        MVGMayaUtil::getMVGLeftCamera(path) : MVGMayaUtil::getMVGRightCamera(path);
	}
}


//
// MVGKeyEventFilter
//
MVGMayaViewportKeyEventFilter::MVGMayaViewportKeyEventFilter(QObject* parent)
: QObject(parent)
{
}

bool MVGMayaViewportKeyEventFilter::eventFilter(QObject * obj, QEvent * e)
{
  
  // TODO
  // Key Press "F" to fit image plane

	if ((e->type() == QEvent::KeyPress)) {
		QKeyEvent * keyevent = static_cast<QKeyEvent *>(e);
		if (keyevent->isAutoRepeat()) {
			return true;
		}
		switch (keyevent->key()) {
			case Qt::Key_A:
			case Qt::Key_B:
			case Qt::Key_Alt:
			case Qt::Key_Control:
			case Qt::Key_Shift:
			case Qt::Key_Meta:
				return true;
			default:
				break;
		}
	}
	return QObject::eventFilter(obj, e);
}


//
// MVGMouseEventFilter
//
MVGMayaViewportMouseEventFilter::MVGMayaViewportMouseEventFilter(QObject* parent)
: QObject(parent)
, m_tracking(false)
{
}

bool MVGMayaViewportMouseEventFilter::eventFilter(QObject * obj, QEvent * e)
{  
  // Image is fitted on width.
  // In maya, width is not 1 but math.sqrt(2).
  const double unit_maya = 1.4142135623730951;
  QMouseEvent * mouseevent = static_cast<QMouseEvent *>(e);
  
  // Init pan and zoom
  if (e->type() == QEvent::MouseButtonPress)
  {
    // Camera Pan (Alt + Mid button)
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    if((modifiers & Qt::AltModifier) && (mouseevent->button() & Qt::MidButton))
    {
      MDagPath cameraPath;
      if(getCameraPathFromQbject(obj, cameraPath))
      {
        MFnCamera camera(cameraPath);
        // register click position
        m_clickPos = mouseevent->pos();
        // register camera film offset
        m_cameraHPan = camera.horizontalPan();
        m_cameraVPan = camera.verticalPan();
        // set as tracking
        m_tracking = true;
      }
    }
  }
  // Apply Pan 
  else if(e->type() == QEvent::MouseMove) 
  {
    if(!m_tracking)
      return QObject::eventFilter(obj, e);

    QWidget* widget = qobject_cast<QWidget*>(obj);
    MDagPath cameraPath;
    if(getCameraPathFromQbject(obj, cameraPath)) 
    {
      MFnCamera camera(cameraPath);
      // compute mouse offset
      QPointF offset_screen = m_clickPos - mouseevent->pos();
      const double viewport_width = widget->width();
      QPointF offset = (offset_screen / viewport_width) * unit_maya * camera.zoom();
      
      camera.setHorizontalPan(m_cameraHPan + offset.x());
      camera.setVerticalPan(m_cameraVPan - offset.y());
    }
  }
  else if(e->type() == QEvent::MouseButtonRelease) 
  {
    // disable tracking
    m_tracking = false;
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
      QPointF mouse_maya_center = mouse_ratio_center * unit_maya * previousZoom;
      QPointF mouseAfterZoom_maya_center = mouse_maya_center * scaleRatio;
      QPointF offset = mouse_maya_center - mouseAfterZoom_maya_center;

      camera.setHorizontalPan( camera.horizontalPan() - offset.x() );
      camera.setVerticalPan( camera.verticalPan() + offset.y() );
    }
  }
  return QObject::eventFilter(obj, e);
}