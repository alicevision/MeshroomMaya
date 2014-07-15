#pragma once

#include <QObject>
#include <QFileDialog>
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/patterns/Singleton.h"
#include "mayaMVG/qt/QObjectListModel.h"

#include <maya/M3dView.h>

namespace mayaMVG {
	
typedef std::pair<std::string, MPoint> PairStringToPoint;
typedef std::map<std::pair<std::string, MPoint>, std::pair<std::string, MPoint> > Map2Dto3D;
typedef std::map<std::pair<std::string, MPoint>, std::vector<std::pair<std::string, MPoint> > > Map3Dto2D;


struct	Point2D {
	/// Position in camera coords
	MPoint	pos;
	/// Position in camera coord of the projected associated point 3D
	MPoint	projectedPoint3D;
	/// Number of views in which point3D is placed
	int		nbViews;
	/// Is point placed in this view
	bool	isInThisView;

};

struct DisplayData {
	MVGCamera camera;
	/// temporary points in Camera before having 3D information
	MPointArray buildPoints2D;
	/// 2D points in Camera coord (centered normalized on width?)
	MPointArray cameraPoints2D;

	// TODO : use instead of cameraPoints2D
	std::vector<Point2D> allPoints2D;
};

class MVGProjectWrapper : public QObject, public Singleton<MVGProjectWrapper>
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ projectDirectory NOTIFY projectDirectoryChanged);
    Q_PROPERTY(QString cameraDirectory READ cameraDirectory NOTIFY cameraDirectoryChanged);
    Q_PROPERTY(QString imageDirectory READ imageDirectory NOTIFY imageDirectoryChanged);
    Q_PROPERTY(QString pointCloudFile READ pointCloudFile NOTIFY pointCloudFileChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ cameraModel NOTIFY cameraModelChanged);
	Q_PROPERTY(QStringList panelModel READ panelModel NOTIFY panelModelChanged);
    Q_PROPERTY(QString logText READ logText WRITE setLogText NOTIFY logTextChanged);
    MAKE_SINGLETON_WITHCONSTRUCTORS(MVGProjectWrapper)

public:
    Q_INVOKABLE const QString moduleDirectory() const;
    Q_INVOKABLE const QString projectDirectory() const;
    Q_INVOKABLE const QString cameraDirectory() const;
    Q_INVOKABLE const QString imageDirectory() const;
    Q_INVOKABLE const QString pointCloudFile() const;
    Q_INVOKABLE QObjectListModel* cameraModel() {return &_cameraList;}
    Q_INVOKABLE QStringList panelModel() {return _visiblePanelNames;}
    Q_INVOKABLE void setProjectDirectory(const QString& directory);
    Q_INVOKABLE const QString logText() const;
    Q_INVOKABLE void setLogText(const QString);
    void appendLogText(const QString);
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void onSelectContextButtonClicked();
    Q_INVOKABLE void onPlaceContextButtonClicked();
    Q_INVOKABLE void loadProject(const QString& projectDirectoryPath);
    void selectItems(const QList<QString>& cameraNames);
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName);
	
	// TODO
	Map3Dto2D& getMap3Dto2D() { return _map3Dto2D; }
	Map2Dto3D& getMap2Dto3D() { return _map2Dto3D; }
	
	DisplayData* getCachedDisplayData(M3dView& view);
	Q_INVOKABLE void reloadProjectFromMaya();
	Q_INVOKABLE void rebuildCacheFromMaya();

signals:
    void projectDirectoryChanged();
    void cameraDirectoryChanged();
    void imageDirectoryChanged();
    void pointCloudFileChanged();
    void cameraModelChanged();
	void panelModelChanged();
    void logTextChanged();

private:
    QObjectListModel _cameraList;
    MVGProject _project;
    QString _logText;
	
	// TODO
	Map2Dto3D _map2Dto3D;
	Map3Dto2D _map3Dto2D;
	std::map<std::string, DisplayData> _cache;
	QStringList _allPanelNames;
	QStringList _visiblePanelNames;
	std::map<std::string, std::string> _panelToCamera;
	
};

} // mayaMVG

namespace std {
	inline bool operator<(const mayaMVG::PairStringToPoint& pair_a, const mayaMVG::PairStringToPoint& pair_b) { 
		if(pair_a.first != pair_b.first)
			return (pair_a.first < pair_b.first);
		
		if(pair_a.second.x != pair_b.second.x)
			return pair_a.second.x < pair_b.second.x;
		
		return pair_a.second.y < pair_b.second.y;
	}
}	// std
