#pragma once

#include "mayaMVG/qt/QObjectListModel.hpp"
#include "mayaMVG/qt/MVGPanelWrapper.hpp"
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "maya/MDistance.h"
#include <QObject>

namespace mayaMVG
{

#define IMAGE_CACHE_SIZE 3

class MVGProjectWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ getProjectDirectory WRITE setProjectDirectory NOTIFY
                   projectDirectoryChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
    Q_PROPERTY(QString currentContext READ getCurrentContext WRITE setCurrentContext NOTIFY
                   currentContextChanged);
    Q_PROPERTY(QObjectListModel* panelList READ getPanelList NOTIFY panelListChanged);
    Q_PROPERTY(QString currentUnit READ getCurrentUnit NOTIFY currentUnitChanged);

public:
    MVGProjectWrapper();
    ~MVGProjectWrapper();

public slots:
    const QString getProjectDirectory() const;
    void setProjectDirectory(const QString& directory);
    QObjectListModel* getCameraModel() { return &_cameraList; }
    QObjectListModel* getPanelList() { return &_panelList; }
    const QString getCurrentContext() const;
    void setCurrentContext(const QString&);
    const QString getCurrentUnit() const;

signals:
    void projectDirectoryChanged();
    void cameraModelChanged();
    void panelModelChanged();
    void currentContextChanged();
    void panelListChanged();
    void currentUnitChanged();
    void unitModelChanged();

public:
    void selectItems(const QList<QString>& cameraNames) const;
    Q_INVOKABLE void selectCameras(const QStringList& cameraNames) const;
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void activeSelectionContext() const;
    Q_INVOKABLE void activeMVGContext();
    Q_INVOKABLE void loadExistingProject();
    Q_INVOKABLE void loadNewProject(const QString& projectDirectoryPath);
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName,
                                     bool rebuildCache = true);
    Q_INVOKABLE void scaleScene(const double scaleSize) const;
    void pushImageInCache(const std::string& imageName);
    void clear();
    void removeCameraFromUI(MDagPath& cameraPath);
    void emitCurrentUnitChanged();

private:
    void reloadMVGCamerasFromMaya();

private:
    QObjectListModel _cameraList;
    MVGProject _project;
    QObjectListModel _panelList;
    QString _currentContext;

    std::map<std::string, MVGCameraWrapper*> _camerasByName;
    /// map view to active camera
    std::map<std::string, std::string> _activeCameraNameByView;
    /// FIFO queue indicating the list of images/cameras keept in memory
    std::list<std::string> _cachedImagePlanes;
    QMap<MDistance::Unit, QString> _unitMap;
};

} // namespace
