#pragma once

#include <QObject>
#include <QPointF>
#include <maya/MDagPath.h>
#include <maya/MCallbackIdArray.h>

namespace mayaMVG {

class MVGContext;

class MVGKeyEventFilter: public QObject {
    public:
        MVGKeyEventFilter();
    protected:
        bool eventFilter(QObject *obj, QEvent *e);
};

class MVGMouseEventFilter: public QObject {
    public:
        MVGMouseEventFilter();
    protected:
        bool eventFilter(QObject *obj, QEvent *e);
  private:
    QPointF m_clickPos;
    double m_cameraHPan;
    double m_cameraVPan;
    bool m_tracking;
};

class MVGWindowEventFilter: public QObject {
    public:
        MVGWindowEventFilter(const MCallbackIdArray& ids, MVGMouseEventFilter*m=NULL, MVGKeyEventFilter*k=NULL);
    protected:
        bool eventFilter(QObject *obj, QEvent *e);
    private:
    MCallbackIdArray m_ids;
      MVGMouseEventFilter* m_mouseFilter;
      MVGKeyEventFilter* m_keyFilter;
};

class MVGContextEventFilter: public QObject {
    public:
        MVGContextEventFilter(MVGContext* ctx);
    protected:
        bool eventFilter(QObject *obj, QEvent *e);
    private:
        MVGContext* m_context;
};

} // mayaMVG
