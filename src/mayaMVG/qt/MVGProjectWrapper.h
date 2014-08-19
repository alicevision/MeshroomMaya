#pragma once

#include <QObject>
#include <QFileDialog>
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/qt/QObjectListModel.h"

namespace mayaMVG {
	
class MVGProjectWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ projectDirectory WRITE setProjectDirectory NOTIFY projectDirectoryChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
	Q_PROPERTY(QStringList visiblePanelNames READ getVisiblePanelNames NOTIFY panelModelChanged);
    Q_PROPERTY(QString logText READ logText WRITE setLogText NOTIFY logTextChanged);
    // Q_PROPERTY(QString currentContext READ currentContext WRITE setCurrentContext NOTIFY currentContextChanged);

public:
	MVGProjectWrapper();
	~MVGProjectWrapper();

public slots:
	const QString projectDirectory() const;
	void setProjectDirectory(const QString& directory);
	QObjectListModel* getCameraModel() { return &_cameraList; }
    const QStringList& getVisiblePanelNames() const { return _visiblePanelNames; }
	const QString logText() const;
    void setLogText(const QString&);
    // const QString currentContext() const;
    // void setCurrentContext(const QString&);
	
signals:
    void projectDirectoryChanged();
    void cameraModelChanged();
	void panelModelChanged();
    void logTextChanged();
    void currentContextChanged();

public:
	void appendLogText(const QString&);
	void selectItems(const QList<QString>& cameraNames) const;
	Q_INVOKABLE void selectCameras(const QStringList& cameraNames) const;
    Q_INVOKABLE const QString moduleDirectory() const;
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void activeSelectionContext() const;
    Q_INVOKABLE void activeMVGContext();
    Q_INVOKABLE void loadProject(const QString& projectDirectoryPath);
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName, bool rebuildCache = true) const;
    void reloadMVGCamerasFromMaya();

private:
    QObjectListModel _cameraList;
    MVGProject _project;
    QString _logText;
	QStringList _allPanelNames;
	QStringList _visiblePanelNames;
};

} // mayaMVG
