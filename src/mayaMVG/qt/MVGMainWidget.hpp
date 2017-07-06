#pragma once

#include "mayaMVG/qt/MVGProjectWrapper.hpp"
#include <QWidget>

class QWidget;
class QQuickWidget;

namespace mayaMVG
{

class MVGMainWidget : public QWidget
{

    Q_OBJECT

public:
    MVGMainWidget(QWidget* parent = 0);
    ~MVGMainWidget();

public:
    QWidget* getView() const;
    MVGProjectWrapper& getProjectWrapper() { return _projectWrapper; }

    // Needed to pass the focusOutEvent to the view
    void focusOutEvent(QFocusEvent* event);

private:
    QQuickWidget* _view;
    MVGProjectWrapper _projectWrapper;
};

} // namespace
