#pragma once

#include <QDeclarativeItem>
#include <QGraphicsSceneWheelEvent>
#include <assert.h>

class QWheelArea : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit QWheelArea(QDeclarativeItem* parent = 0)
        : QDeclarativeItem(parent)
        , _lastEvent(NULL)
    {
    }

protected:
    void wheelEvent(QGraphicsSceneWheelEvent* event)
    {
        _lastEvent = event;
        switch(event->orientation())
        {
            case Qt::Horizontal:
                Q_EMIT horizontalWheel(event->delta(), event->modifiers());
                break;
            case Qt::Vertical:
                Q_EMIT verticalWheel(event->delta(), event->modifiers());
                break;
            default:
                event->ignore();
                break;
        }
        _lastEvent = NULL;
    }

public:
    Q_INVOKABLE void eventAccept()
    {
        assert(_lastEvent);
        _lastEvent->accept();
    }
    Q_INVOKABLE void eventIgnore()
    {
        assert(_lastEvent);
        _lastEvent->ignore();
    }

Q_SIGNALS:
    void verticalWheel(int delta, Qt::KeyboardModifiers modifier);
    void horizontalWheel(int delta, Qt::KeyboardModifiers modifier);

private:
    QGraphicsSceneWheelEvent* _lastEvent; // Hack: only valid during the wheelEvent call
};
