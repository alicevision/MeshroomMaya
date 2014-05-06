#ifndef MVGMAYAVIEWPORTEVENT_H
#define	MVGMAYAVIEWPORTEVENT_H

#include <QObject>
#include <QPointF>

namespace mayaMVG {
	
class MVGContext;

class MVGMayaViewportKeyEventFilter: public QObject {
    public:
        MVGMayaViewportKeyEventFilter(QObject* parent);
    protected:
        bool eventFilter(QObject *obj, QEvent *e);
};

class MVGMayaViewportMouseEventFilter: public QObject {
    public:
        MVGMayaViewportMouseEventFilter(QObject* parent);
    protected:
        bool eventFilter(QObject *obj, QEvent *e);
  private:
    QPointF m_clickPos;
    double m_cameraHPan;
    double m_cameraVPan;
    bool m_tracking;
};
}

#endif	/* MVGMAYAVIEWPORTEVENT_H */

