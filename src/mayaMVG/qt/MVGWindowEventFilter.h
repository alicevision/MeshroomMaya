#pragma once

#include "mayaMVG/qt/MVGViewportEventFilter.h"
#include <QObject>
#include <maya/MCallbackIdArray.h>


namespace mayaMVG {

class MVGWindowEventFilter: public QObject {

	public:
		MVGWindowEventFilter(const MCallbackIdArray& ids, 
			MVGViewportEventFilter* m, QObject* parent);

	protected:
		bool eventFilter(QObject *obj, QEvent *e);

	private:
		MCallbackIdArray _callbackIds;
		MVGViewportEventFilter* _viewportFilter;
};

} // namespace
