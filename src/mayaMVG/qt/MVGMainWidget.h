#pragma once

#include <QWidget>
#include <QtDeclarative/qdeclarativeview.h>

namespace mayaMVG {

class MVGMainWidget : public QWidget {
	
	Q_OBJECT

	public:
		MVGMainWidget(QWidget * parent = 0);
		~MVGMainWidget();

	public:
		QDeclarativeView* view() const ;
				
	private:
		QDeclarativeView* _view;
};

} // mayaMVG
