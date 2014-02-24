#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QWidget>
#include <QApplication>

#include "mayaMVG/qt/MVGEventFilter.h"
#include "mayaMVG/util/MVGUtil.h"
#include "mayaMVG/util/MVGLog.h"
#include "mayaMVG/context/MVGContext.h"

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
    return (panelName.toString()=="left") ? MVGUtil::getMVGLeftCamera(path) : MVGUtil::getMVGRightCamera(path);
  }
}


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
  
  // Init pan and zoom
  if (e->type() == QEvent::MouseButtonPress) {
    Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
    // Camera Pan (Alt + Mid button)
    if((modifiers & Qt::AltModifier) && (mouseevent->button() & Qt::MidButton)) {
      MDagPath cameraPath;
      if(getCameraPathFromQbject(obj, cameraPath)) {
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
    MDagPath cameraPath;
    if(getCameraPathFromQbject(obj, cameraPath)) 
    {
      MFnCamera camera(cameraPath);
      // compute mouse offset
      QPointF offset = m_clickPos - mouseevent->pos();
      offset *= camera.zoom() / 190.f; // FIXME
      
      camera.setHorizontalPan( m_cameraHPan + offset.x( ) );
      camera.setVerticalPan( m_cameraVPan - offset.y( ) );
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
    QWheelEvent * wheelevent = static_cast<QWheelEvent *>(e);
    MDagPath cameraPath;
    if(getCameraPathFromQbject(obj, cameraPath)) 
    {
      MFnCamera camera(cameraPath);
      QPointF positionAfterZoom;
      
      QWidget* wid = (QWidget*)obj;
      QPointF center( wid->width( ) / 2, wid->height( ) / 2 );
      
      if( wheelevent->delta() > 0 )
      {
        //Zoom in
        camera.setZoom(camera.zoom() / 1.15 );
        positionAfterZoom.setX( ( mouseevent->pos().x() - center.x() ) / 1.15 + center.x() );
        positionAfterZoom.setY( ( mouseevent->pos().y() - center.y() ) / 1.15 + center.y() );
      }
      else
      {
        //Zooming out
        camera.setZoom(camera.zoom() * 1.15 );
        positionAfterZoom.setX( ( mouseevent->pos().x() - center.x() ) * 1.15 + center.x() );
        positionAfterZoom.setY( ( mouseevent->pos().y() - center.y() ) * 1.15 + center.y() );
      }
      // Apply pan to stay at the same point
      QPointF offset = mouseevent->pos() - positionAfterZoom;
      offset *= camera.zoom() / 190.f; // FIXME
      
      camera.setHorizontalPan( camera.horizontalPan() + offset.x( ) );
      camera.setVerticalPan( camera.verticalPan() - offset.y( ) );
    }
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
  else if (e->type() == QEvent::Enter) {
    QVariant panelName = obj->property("mvg_panel");
    if(panelName.type()==QVariant::Invalid)
      return QObject::eventFilter(obj, e);
    // set focus
    if(panelName.toString()=="left")
      MVGUtil::setFocusOnLeftView();
    else
      MVGUtil::setFocusOnRightView();
  } else if (e->type() == QEvent::Leave) {
  }
  return QObject::eventFilter(obj, e);
}
