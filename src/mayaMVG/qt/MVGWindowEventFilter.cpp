#include "mayaMVG/qt/MVGWindowEventFilter.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include <QEvent>
#include <QVariant>
#include <QWidget>
#include <QLayout>
#include <maya/MQtUtil.h>

using namespace mayaMVG;

MVGWindowEventFilter::MVGWindowEventFilter(const MCallbackIdArray& ids
			, MVGViewportEventFilter* filter, QObject* parent)
	: QObject(parent)
	, _callbackIds(ids)
	, _viewportFilter(filter)
{
}

bool MVGWindowEventFilter::eventFilter(QObject * obj, QEvent * e)
{
	if ((e->type() == QEvent::Close)) {
		// remove event filters
		// FIXME
		// remove maya callbacks
		if(_callbackIds.length()>0)
			MMessage::removeCallbacks(_callbackIds);
		// delete maya context
		MVGMayaUtil::deleteMVGContext();
		// remove window event filter
		obj->removeEventFilter(this);
	}
	return QObject::eventFilter(obj, e);
}
