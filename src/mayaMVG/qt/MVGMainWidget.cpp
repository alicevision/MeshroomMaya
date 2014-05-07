#include "mayaMVG/qt/MVGMainWidget.h"
#include "mayaMVG/qt/QmlInstantCoding.h"
#include "mayaMVG/qt/MVGProjectWrapper.h"
#include <QtDeclarative/qdeclarativecontext.h>
#include <QGraphicsObject>

using namespace mayaMVG;

MVGMainWidget::MVGMainWidget(QWidget * parent)
	: QWidget(parent)
{
	_view = new QDeclarativeView(parent);
	
	// QtDesktop Components
	_view->engine()->addPluginPath("/usr/lib64/qt4/plugins/imports");
	_view->engine()->addImportPath("/usr/lib64/qt4/plugins/imports");

	// Qml source
	_view->setSource(QUrl::fromLocalFile("/datas/nro/dev/openMVGUI/src/mayaMVG/qt/qml/main.qml"));
	_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

	// Expose Project to QML
	_view->rootContext()->setContextProperty("_project", &(MVGProjectWrapper::instance()));

	// Instant coding
	QmlInstantCoding* qic = new QmlInstantCoding(_view, true);
	qic->addFilesFromDirectory("/datas/nro/dev/openMVGUI/src/mayaMVG/qt/qml", true);

}

MVGMainWidget::~MVGMainWidget()
{
}

QDeclarativeView* MVGMainWidget::view() const 
{
	return _view;
}
