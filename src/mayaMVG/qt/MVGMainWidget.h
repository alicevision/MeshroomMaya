#ifndef MVGMAINWIDGET_H
#define	MVGMAINWIDGET_H

#include <QWidget>
//#include "mayaMVG/qt/QmlInstantCoding.h"
#include "QtDeclarative/qdeclarativeview.h"

namespace mayaMVG {

	class MVGMainWidget : public QWidget {
		
		Q_OBJECT

	public:
		MVGMainWidget(QWidget * parent = 0);
		
		~MVGMainWidget();
		
		QDeclarativeView* view() const ;
				
	private:
		QDeclarativeView*	_view;		
		
	};
}

#endif	/* MVGMAINWIDGET_H */

