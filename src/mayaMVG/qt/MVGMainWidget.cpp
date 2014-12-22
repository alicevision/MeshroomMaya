#include "mayaMVG/qt/MVGMainWidget.hpp"
#include "mayaMVG/qt/QmlInstantCoding.hpp"
#include "mayaMVG/qt/QWheelArea.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include <QtDeclarative/QDeclarativeView>
#include <QtGui/QFocusEvent>

namespace mayaMVG
{

MVGMainWidget::MVGMainWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("mvgMainWidget");

    qmlRegisterType<MVGCameraWrapper>();
    qmlRegisterType<QObjectListModel>();
    qmlRegisterType<QWheelArea>("MyTools", 1, 0, "CustomWheelArea");

    _view = new QDeclarativeView(parent);

    _projectWrapper.loadExistingProject();

    QString importDirectory = QString(MVGMayaUtil::getModulePath().asChar()) + "/qml";
    QString sourceDirectory = importDirectory + "/mvg/main.qml";

    // QtDesktop Components
    _view->engine()->addPluginPath(importDirectory);
    _view->engine()->addImportPath(importDirectory);

    // Expose Project to QML
    _view->rootContext()->setContextProperty("_project", &_projectWrapper);

    // Qml source
    _view->setSource(QUrl::fromLocalFile(sourceDirectory));
    _view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

    // Instant coding
    // QmlInstantCoding* qic = new QmlInstantCoding(_view, true);
    // qic->addFilesFromDirectory(importDirectory, true);
}

MVGMainWidget::~MVGMainWidget()
{
}

void MVGMainWidget::focusOutEvent(QFocusEvent* event)
{
    event->accept();
    _view->clearFocus();
}

QDeclarativeView* MVGMainWidget::getView() const
{
    return _view;
}

} // namespace
