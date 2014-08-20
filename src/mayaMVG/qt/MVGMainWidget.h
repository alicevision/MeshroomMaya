#pragma once

#include "mayaMVG/qt/MVGProjectWrapper.h"
#include <QtDeclarative/qdeclarativeview.h>
#include <QWidget>

namespace mayaMVG {

class MVGMainWidget : public QWidget {
	
	Q_OBJECT

	public:
		MVGMainWidget(QWidget * parent = 0);
		~MVGMainWidget();

	public:
		QDeclarativeView* view() const ;
		MVGProjectWrapper& getProjectWrapper() { return _projectWrapper;}
				
	private:
		QDeclarativeView* _view;
		MVGProjectWrapper _projectWrapper;
};

} // namespace
