#include "MVGManipulatorKeyEventFilter.h"

#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"

namespace mayaMVG {
	
MVGManipulatorKeyEventFilter::MVGManipulatorKeyEventFilter(QObject* mayaWindow, MVGBuildFaceManipulator* manipulator)
: QObject(mayaWindow)
, _mayaWindow(mayaWindow)
, _manipulator(manipulator)
{
	// Install key event filter on maya viewports
	_mayaWindow->installEventFilter(this);
//	_mayaWindow->setProperty("mvg_keyFiltered", true);
}

MVGManipulatorKeyEventFilter::~MVGManipulatorKeyEventFilter()
{
	_mayaWindow->removeEventFilter(this);
//	_mayaWindow->setProperty("mvg_keyFiltered", false);
}

bool MVGManipulatorKeyEventFilter::eventFilter(QObject* obj, QEvent* e)
{
	if(_manipulator-> eventFilter(obj, e))
		return true;
	return QObject::eventFilter(obj, e);
}

} // mayaMVG