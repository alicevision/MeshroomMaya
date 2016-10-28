#pragma once

#include <maya/MString.h>
#include <maya/MStatus.h>
#include <vector>
#include <set>

class MObject;
class MPlug;
class MIntArray;
class MDoubleArray;
class MPointArray;
class MVectorArray;
class MPoint;
class MColor;
class MDagPath;
class M3dView;
class QWidget;
class MColor;
class QColor;

namespace mayaMVG
{

class MVGCamera;

struct MVGMayaUtil
{
    // window
    static MStatus createMVGWindow();
    static MStatus deleteMVGWindow();
    static QWidget* getMVGWindow();
    // window menu
    static QWidget* getMVGMenuLayout();
    // viewports
    static QWidget* getMVGViewportLayout(const MString& viewName);
    static MStatus setFocusOnView(const MString& viewName);
    static bool isMVGView(const M3dView& view);
    static bool isActiveView(const M3dView& view);
    static bool getComplementaryView(const M3dView& view, M3dView& complementaryView);
    // context
    static MStatus createMVGContext();
    static MStatus deleteMVGContext();
    static MStatus activeContext();
    static MStatus activeMayaContext();
    static MStatus getCurrentContext(MString& context);
    static MStatus setCurrentContext(MString& context);
    static MStatus setCreationMode();
    static MStatus setTriangulationMode();
    static MStatus setPointCloudMode();
    static MStatus setAdjacentPlaneMode();
    static MStatus setLocatorMode();
    // cameras
    static MStatus setCameraInView(const MVGCamera& camera, const MString& viewName);
    static MStatus getCameraInView(MDagPath& path, const MString& viewName);
    static MStatus clearCameraInView(const MString& viewName);
    // maya selection
    static MStatus addToMayaSelection(const MString& objectName);
    static MStatus clearMayaSelection();
    static MStatus selectParticles(const MString &objectName, const std::set<int> &points);
    // attributes
    static MStatus getIntArrayAttribute(const MObject&, const MString&, MIntArray&, bool = false);
    static MStatus setIntArrayAttribute(const MObject&, const MString&, const MIntArray&,
                                        bool = false);
    static MStatus getIntAttribute(const MObject&, const MString&, int&, bool = false);
    static MStatus setIntAttribute(const MObject&, const MString&, const int&, bool = false);
    static MStatus getDoubleAttribute(const MObject&, const MString&, double&, bool = false);
    static MStatus setDoubleAttribute(const MObject&, const MString&, const double&, bool = false);
    static MStatus getDoubleArrayAttribute(const MObject&, const MString&, MDoubleArray&,
                                           bool = false);
    static MStatus setDoubleArrayAttribute(const MObject&, const MString&, const MDoubleArray&,
                                           bool = false);
    static MStatus getVectorArrayAttribute(const MObject&, const MString&, MVectorArray&,
                                           bool = false);
    static MStatus setVectorArrayAttribute(const MObject&, const MString&, const MVectorArray&,
                                           bool = false);
    static MStatus getPointArrayAttribute(const MObject&, const MString&, MPointArray&,
                                          bool = false);
    static MStatus setPointArrayAttribute(const MObject&, const MString&, const MPointArray&,
                                          bool = false);
    static MStatus getStringAttribute(const MObject&, const MString&, MString&, bool = false);
    static MStatus setStringAttribute(const MObject&, const MString&, const MString&, bool = false);
    static MStatus getColorAttribute(const MObject&, const MString&, MColor&, bool = false);
    static MStatus setColorAttribute(const MObject&, const MString&, const MColor&, bool=false);
    static MStatus findConnectedNodes(const MObject&, const MString&, std::vector<MObject>&);
    // nodes
    static MStatus getPlug(const MObject& node, const MString& plugName, bool networked, MPlug& plug);
    static MStatus getObjectByName(const MString&, MObject&);
    static MStatus getDagPathByName(const MString&, MDagPath&);
    // DAG
    /**
     * Create a locator of the given type under parent.
     * @param type the locator's type
     * @param name the locator's name
     * @param parent the parent node (MObject() to create the locator at root level)
     * @param locator the created locator
     * @param createTransform if True, create a transform between 'parent' and the locator
     * @return the status of this operation
     */
    static MStatus addLocator(const MString& type, const MString& name, 
                               const MObject& parent, MObject& locator, bool createTransform=true);
    // Conversion
    static MColor fromQColor(const QColor& color);
    static QColor fromMColor(const MColor& color);
    // environment
    static MString getEnv(const MString&);
    static MString getModulePath();
    // filedialog
    static MStatus openFileDialog(MString& directory);
    // undo/redo
    static MStatus getUndoName(MString& undoName);
    static MStatus getRedoName(MString& redoName);
};

} // namespace
