#pragma once

#include <maya/MString.h>
#include <maya/MStatus.h>

class QWidget;
class MDagPath;

namespace mayaMVG {

class MVGMenu;

struct MVGUtil {
	// window
	static QWidget* createMVGWindow();
	static MStatus deleteMVGWindow();
	// window menu
	static void populateMVGMenu(MVGMenu* menu);
	// context
	static MStatus createMVGContext();
	static MStatus deleteMVGContext();
	// cameras
	static MStatus getMVGLeftCamera(MDagPath& path);
	static MStatus setMVGLeftCamera(MString camera);
	static MStatus getMVGRightCamera(MDagPath& path);
	static MStatus setMVGRightCamera(MString camera);
	static MStatus addToMayaSelection(MString camera);
	// maya selection
	static MStatus clearMayaSelection();
};

} // mayaMVG
