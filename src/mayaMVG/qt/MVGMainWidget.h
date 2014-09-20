#pragma once

#include "mayaMVG/qt/MVGProjectWrapper.h"

#include <QtGui/QWidget>

class QWidget;
class QDeclarativeView;

namespace mayaMVG
{

class MVGMainWidget : public QWidget
{

    Q_OBJECT

public:
    MVGMainWidget(QWidget* parent = 0);
    ~MVGMainWidget();

public:
    QDeclarativeView* getView() const;
    MVGProjectWrapper& getProjectWrapper() { return _projectWrapper; }

    // Needed to pass the focusOutEvent to the view
    void focusOutEvent(QFocusEvent* event);

private:
    QDeclarativeView* _view;
    MVGProjectWrapper _projectWrapper;
};

} // namespace
