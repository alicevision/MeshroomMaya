#ifndef MVGMANIPULATORKEYEVENTFILTER_H
#define	MVGMANIPULATORKEYEVENTFILTER_H

#include <QtCore/QObject>

namespace mayaMVG {
class MVGBuildFaceManipulator;
	
class MVGManipulatorKeyEventFilter: public QObject {
	
public:
	MVGManipulatorKeyEventFilter(QObject* mayaWindow, MVGBuildFaceManipulator* manipulator);
	
	~MVGManipulatorKeyEventFilter();
	
protected:
	bool eventFilter(QObject* obj, QEvent* e);
	
	QObject* _mayaWindow;
	MVGBuildFaceManipulator* _manipulator;

};

} // mayaMVG

#endif

