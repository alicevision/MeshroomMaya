#pragma once

#include <QObject>
#include <QFileDialog>
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/patterns/Singleton.h"
#include "mayaMVG/qt/QObjectListModel.h"

#include <maya/M3dView.h>

namespace mayaMVG {
	
enum EPointState {
	eUnMovable = 0
	, eMovableInSamePlane = 1
	, eMovableRecompute = 2
};

struct	MVGPoint2D {
	/// Position in camera coords
//	MPoint	pos;
	/// Position in camera coord of the projected associated point 3D
	MPoint	projectedPoint3D;
	/// Position 3D
	MPoint	point3D;
	/// Number of views in which point3D is placed
//	int		nbViews;
	/// Is point placed in this view
//	bool	isInThisView;
	/// How the point is movable 
	EPointState movableState;

};

struct DisplayData {
	MVGCamera camera;
	/// Temporary points in Camera before having 3D information
	MPointArray buildPoints2D;
	/// Map mesh to MVGPoints2D
	std::map<std::string, std::vector<MVGPoint2D> > allPoints2D;
};

class MVGProjectWrapper : public QObject, public Singleton<MVGProjectWrapper>
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ projectDirectory WRITE setProjectDirectory NOTIFY projectDirectoryChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
	Q_PROPERTY(QStringList visiblePanelNames READ getVisiblePanelNames NOTIFY panelModelChanged);
    Q_PROPERTY(QString logText READ logText WRITE setLogText NOTIFY logTextChanged);
    Q_PROPERTY(QString currentContext READ currentContext WRITE setCurrentContext NOTIFY currentContextChanged);
    MAKE_SINGLETON_WITHCONSTRUCTORS(MVGProjectWrapper)

public slots:
	const QString projectDirectory() const;
	void setProjectDirectory(const QString& directory);
	QObjectListModel* getCameraModel() { return &_cameraList; }
    const QStringList& getVisiblePanelNames() const { return _visiblePanelNames; }
	const QString logText() const;
    void setLogText(const QString&);
    const QString currentContext() const;
    void setCurrentContext(const QString&);
	
signals:
    void projectDirectoryChanged();
    void cameraModelChanged();
	void panelModelChanged();
    void logTextChanged();
    void currentContextChanged();
	
public:
	void appendLogText(const QString&);
	void selectItems(const QList<QString>& cameraNames);
	
    Q_INVOKABLE const QString moduleDirectory() const;   
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void activeSelectionContext();
    Q_INVOKABLE void activeMVGContext();
    Q_INVOKABLE void loadProject(const QString& projectDirectoryPath);
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName);
		
	DisplayData* getCachedDisplayData(M3dView& view);
	std::map<std::string, DisplayData>& getDisplayDataCache() { return _cacheCameraToDisplayData;}
	inline std::map<std::string, std::vector<MIntArray> >& getCacheMeshToEdgeArray() { return _cacheMeshToEdgeArray; }
	inline std::vector<MIntArray>& getMeshEdges(std::string meshName) { return _cacheMeshToEdgeArray[meshName]; }
	
	Q_INVOKABLE void reloadMVGCamerasFromMaya();
	Q_INVOKABLE void rebuildCacheFromMaya();
    void clear();
	MStatus rebuildAllMeshesCacheFromMaya();	// Temporary
	MStatus rebuildMeshCacheFromMaya(MDagPath& meshPath);	// Temporary

private:
    QObjectListModel _cameraList;
    MVGProject _project;
    QString _logText;
    QString _currentContext;
	
    /// Map from camera name to DisplayData
	std::map<std::string, DisplayData> _cacheCameraToDisplayData;
	/// Map from meshName to mesh points
	std::map<std::string, MPointArray> _cacheMeshToPointArray;	// Temporary
	/// Map from meshName to numConnectedFace by point
	std::map<std::string, std::vector<EPointState> > _cacheMeshToMovablePoint;	// Temporary
	/// Map from meshName to edge points ID
	std::map<std::string, std::vector<MIntArray> > _cacheMeshToEdgeArray;	// Temporary

	QStringList _allPanelNames;
	QStringList _visiblePanelNames;
	
};

} // mayaMVG
