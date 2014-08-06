#include "mayaMVG/qt/MVGMainWidget.h"
#include "mayaMVG/qt/QmlInstantCoding.h"
#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/qt/QWheelArea.h"
#include <QtDeclarative/qdeclarativecontext.h>
#include <QGraphicsObject>

using namespace mayaMVG;

MVGMainWidget::MVGMainWidget(QWidget * parent)
	: QWidget(parent)
{
    qmlRegisterType<MVGCameraWrapper>();
    qmlRegisterType<QObjectListModel>();
    qmlRegisterType<QWheelArea>("MyTools", 1, 0, "CustomWheelArea");

	_view = new QDeclarativeView(parent);
	
	MVGProjectWrapper& project = MVGProjectWrapper::instance();
	QString importDirectory = project.moduleDirectory()+"/qml";
	QString sourceDirectory = importDirectory+"/mvg/main.qml";

	// QtDesktop Components
	_view->engine()->addPluginPath(importDirectory);
	_view->engine()->addImportPath(importDirectory);

	// Expose Project to QML
	_view->rootContext()->setContextProperty("_project", &project);

	// Qml source
	_view->setSource(QUrl::fromLocalFile(sourceDirectory));
	_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

	// Instant coding
	QmlInstantCoding* qic = new QmlInstantCoding(_view, true);
	qic->addFilesFromDirectory(importDirectory, true);
}

MVGMainWidget::~MVGMainWidget()
{
}

QDeclarativeView* MVGMainWidget::view() const 
{
	return _view;
}
