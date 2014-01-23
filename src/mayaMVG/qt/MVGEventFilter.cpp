#include <QtCore/QEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include "qt/MVGEventFilter.h"
#include "util/MVGUtil.h"
#include "util/MVGLog.h"
#include "context/MVGContext.h"
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>


using namespace mayaMVG;

//
// MVGKeyEventFilter
//
MVGKeyEventFilter::MVGKeyEventFilter()
{
}

bool MVGKeyEventFilter::eventFilter(QObject * obj, QEvent * e)
{
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
MVGMouseEventFilter::MVGMouseEventFilter()
{
}

bool MVGMouseEventFilter::eventFilter(QObject * obj, QEvent * e)
{
	QMouseEvent * mouseevent = static_cast<QMouseEvent *>(e);
	if (e->type() == QEvent::MouseButtonPress) {
		Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
		// Camera Pan (Alt + Mid button)
		if((modifiers & Qt::AltModifier) && (mouseevent->button() & Qt::MidButton)) {
			// get panel name
			QVariant panelName = obj->property("mvg_panel");
			if(panelName.type()==QVariant::Invalid)
				return QObject::eventFilter(obj, e);
			// get camera dagpath
			if(panelName.toString()=="left") {
				if(!MVGUtil::getMVGLeftCamera(m_camPath))
					return QObject::eventFilter(obj, e);
			} else {
				if(!MVGUtil::getMVGRightCamera(m_camPath))
					return QObject::eventFilter(obj, e);
			}
			// register camera dagpath
			MFnCamera camera(m_camPath);
			// enable camera pan/zoom
			camera.setPanZoomEnabled(true);
			// register click position
			m_clickPos = mouseevent->pos();
			// register camera film offset
			m_cameraHPan = camera.horizontalPan();
			m_cameraVPan = camera.verticalPan();
			// set as tracking
			m_tracking = true;
		}
	} else if(e->type() == QEvent::MouseMove) {
		if(!m_tracking)
			return QObject::eventFilter(obj, e);
		MFnCamera camera(m_camPath);
		// compute mouse offset
		QPointF offset = m_clickPos - mouseevent->pos();
		offset /= camera.zoom()*500.f; // FIXME
		// update camera pan
		camera.setHorizontalPan(m_cameraHPan+offset.x());
		camera.setVerticalPan(m_cameraVPan-offset.y());

	} else if(e->type() == QEvent::MouseButtonRelease) {
		// disable tracking
		m_tracking = false;

	} else if (e->type() == QEvent::Wheel) {
		// compute wheel offset
		QWheelEvent * wheelevent = static_cast<QWheelEvent *>(e);
		float numDegrees = wheelevent->delta() / 8.f;
		float numSteps = numDegrees / 15.f;
		numSteps /= 5.f; // FIXME
		// update camera zoom
		MFnCamera camera(m_camPath);
		camera.setZoom(camera.zoom()-numSteps);
		
	}
	return QObject::eventFilter(obj, e);
}


//
// MVGWindowEventFilter
//
MVGWindowEventFilter::MVGWindowEventFilter(const MCallbackIdArray& ids, MVGMouseEventFilter* mouseFilter, MVGKeyEventFilter* keyFilter)
	: m_ids(ids), m_mouseFilter(mouseFilter), m_keyFilter(keyFilter)
{
}

bool MVGWindowEventFilter::eventFilter(QObject * obj, QEvent * e)
{
	if ((e->type() == QEvent::Close)) {
		// remove mouse and key filters on tagged objects
		QList<QWidget *> children = obj->findChildren<QWidget *>();
		for (int i = 0; i < children.size(); ++i) {
			if(m_mouseFilter && children[i]->property("mvg_mouseFiltered").type()!=QVariant::Invalid)
				children[i]->removeEventFilter(m_mouseFilter);
			if(m_keyFilter && children[i]->property("mvg_keyFiltered").type()!=QVariant::Invalid)
				children[i]->removeEventFilter(m_keyFilter);
		}
		// remove maya callbacks
		if(m_ids.length()>0)
			MMessage::removeCallbacks(m_ids);
		// delete maya context
		MVGUtil::deleteMVGContext();
 		// remove window event filter
		obj->removeEventFilter(this);
	}
	return QObject::eventFilter(obj, e);
}


//
// MVGContextEventFilter
//
MVGContextEventFilter::MVGContextEventFilter(MVGContext* ctx)
	: m_context(ctx)
{
}

bool MVGContextEventFilter::eventFilter(QObject * obj, QEvent * e)
{
	if(!m_context)
		return QObject::eventFilter(obj, e);
	if (e->type() == QEvent::MouseMove) {
		QMouseEvent * mouseevent = static_cast<QMouseEvent *>(e);
		m_context->setMousePos(mouseevent->pos().x(), mouseevent->pos().y());
	} 
	return QObject::eventFilter(obj, e);
}
