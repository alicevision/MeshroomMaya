#include "mayaMVG/qt/MVGMayaEvent.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWidget>
#include <maya/MFnCamera.h>

using namespace mayaMVG;

//
// MVGWindowEventFilter
//
MVGWindowEventFilter::MVGWindowEventFilter(const MCallbackIdArray& ids, MVGMayaViewportMouseEventFilter* mouseFilter, MVGMayaViewportKeyEventFilter* keyFilter, QObject* parent)
: QObject(parent)
, m_ids(ids)
, m_mouseFilter(mouseFilter)
, m_keyFilter(keyFilter)
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
		MVGMayaUtil::deleteMVGContext();
		// remove window event filter
		obj->removeEventFilter(this);
	}
	return QObject::eventFilter(obj, e);
}


//
// MVGContextEventFilter
//
MVGContextEventFilter::MVGContextEventFilter(MVGContext* ctx, QObject* parent)
	: QObject(parent)
	, m_context(ctx)
{
}

bool MVGContextEventFilter::eventFilter(QObject * obj, QEvent * e)
{
	if(!m_context)
		return QObject::eventFilter(obj, e);
	else if (e->type() == QEvent::Enter) {
		QVariant panelName = obj->property("mvg_panel");
		if(panelName.type()==QVariant::Invalid)
			return QObject::eventFilter(obj, e);
		// set focus
		if(panelName.toString()=="left")
			MVGMayaUtil::setFocusOnLeftView();
		else
			MVGMayaUtil::setFocusOnRightView();
	} else if (e->type() == QEvent::Leave) {
	}
	return QObject::eventFilter(obj, e);
}