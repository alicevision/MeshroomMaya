#pragma once

#include <QObject>
#include <QPointF>

namespace mayaMVG {

class MVGContext;

class MVGViewportEventFilter: public QObject {
	
	public:
		MVGViewportEventFilter(QObject* parent);
	
	protected:
		bool eventFilter(QObject *obj, QEvent *e);
	
	private:
		QPointF _clickPos;
		double _cameraHPan;
		double _cameraVPan;
		bool _tracking;
};

} // namespace
