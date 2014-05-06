#include "MVGMainWidget.h"

// Qt 
#include "QtDeclarative/qdeclarativecontext.h"
#include "QGraphicsObject"

// MayaMVG
#include "mayaMVG/qt/QmlInstantCoding.h"
#include <mayaMVG/qt/MVGProjectWrapper.h>


using namespace mayaMVG;

MVGMainWidget::MVGMainWidget(QWidget * parent)
	: QWidget(parent)
{

	_view = new QDeclarativeView(parent);
	
	// QtDesktop Components
	_view->engine()->addPluginPath("/usr/local/Trolltech/Qt-4.8.2/imports");
	_view->engine()->addImportPath("/usr/local/Trolltech/Qt-4.8.2/imports");

	// Qml source
	_view->setSource(QUrl::fromLocalFile("/datas/pra/openMVG-UI/src/mayaMVG/qt/qml/main.qml"));
	_view->setResizeMode(QDeclarativeView::SizeRootObjectToView);

	// Expose Project to QML
	_view->rootContext()->setContextProperty("_project", &(MVGProjectWrapper::instance()));

	// Instant coding
	QmlInstantCoding* qic = new QmlInstantCoding(_view, true);
	qic->addFilesFromDirectory("/datas/pra/openMVG-UI/src/mayaMVG/qt/qml", true);

}

MVGMainWidget::~MVGMainWidget()
{

}

QDeclarativeView* MVGMainWidget::view() const 
{
	return _view;
}