#pragma once

#include <maya/MString.h>
#include <maya/MStatus.h>

class QWidget;
class MDagPath;

namespace mayaMVG {

class MVGMenu;

struct MVGUtil {
	static QWidget* createMVGWindow();
	static void populateMenu(MVGMenu* menu);
	static MStatus getMVGLeftCamera(MDagPath& path);
	static MStatus setMVGLeftCamera(MString camera);
	static MStatus getMVGRightCamera(MDagPath& path);
	static MStatus setMVGRightCamera(MString camera);
	static MStatus addToMayaSelection(MString camera);
	static MStatus clearMayaSelection();
};

} // mayaMVG
