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

    // QtDesktop Components
    _view->engine()->addPluginPath(importDirectory);
    _view->engine()->addImportPath(importDirectory);

    // Expose Project to QML
    _view->rootContext()->setContextProperty("_project", &_projectWrapper);

    // Qml source
    const char* instantCoding = std::getenv("MAYAMVG_USE_QMLINSTANTCODING");
    QString mainQml = importDirectory + "/mvg/main.qml";
    if(instantCoding)
    {
        QDir qmlFolder = QFileInfo(__FILE__).dir();
        qmlFolder.cd("qml");
        mainQml = QFileInfo(qmlFolder, "main.qml").absoluteFilePath();
        
        QmlInstantCoding* qic = new QmlInstantCoding(_view, true);
        qic->addFilesFromDirectory(qmlFolder.absolutePath(), true);
    }
    _view->setSource(QUrl::fromLocalFile(mainQml));
    _view->setResizeMode(QDeclarativeView::SizeRootObjectToView);
}

MVGMainWidget::~MVGMainWidget()
{
    _projectWrapper.clearAndUnloadImageCache();
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
