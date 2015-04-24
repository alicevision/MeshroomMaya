#pragma once

#include "mayaMVG/qt/QObjectListModel.hpp"
#include "mayaMVG/qt/MVGPanelWrapper.hpp"
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include "mayaMVG/qt/MVGMeshWrapper.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "maya/MDistance.h"
#include <QObject>

namespace mayaMVG
{

class MVGProjectWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ getProjectDirectory WRITE setProjectDirectory NOTIFY
                   projectDirectoryChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
    Q_PROPERTY(QObjectListModel* meshModel READ getMeshModel NOTIFY meshModelChanged);
    Q_PROPERTY(QString currentContext READ getCurrentContext WRITE setCurrentContext NOTIFY
                   currentContextChanged);
    Q_PROPERTY(QObjectListModel* panelList READ getPanelList NOTIFY panelListChanged);
    Q_PROPERTY(QString currentUnit READ getCurrentUnit NOTIFY currentUnitChanged);
    Q_PROPERTY(QString pluginVersion READ getPluginVersion CONSTANT);
    Q_PROPERTY(bool isProjectLoading READ getIsProjectLoading NOTIFY isProjectLoadingChanged);

public:
    MVGProjectWrapper();
    ~MVGProjectWrapper();

public Q_SLOTS:
    const QString getProjectDirectory() const;
    void setProjectDirectory(const QString& directory);
    QObjectListModel* getCameraModel() { return &_cameraList; }
    QObjectListModel* getMeshModel() { return &_meshesList; }
    QStringList& getSelectedCameras() { return _selectedCameras; }
    QStringList& getSelectedMeshes() { return _selectedMeshes; }
    QObjectListModel* getPanelList() { return &_panelList; }
    const QString getCurrentContext() const;
    void setCurrentContext(const QString&);
    const QString getCurrentUnit() const;
    const QString getPluginVersion() const;
    const bool getIsProjectLoading() const { return _isProjectLoading; }
    void setIsProjectLoading(const bool value);

Q_SIGNALS:
    void projectDirectoryChanged();
    void cameraModelChanged();
    void meshModelChanged();
    void currentContextChanged();
    void panelListChanged();
    void currentUnitChanged();
    void isProjectLoadingChanged();
    void centerCameraListByIndex(const int cameraIndex);
    void centerMeshListByIndex(const int meshIndex);

public:
    Q_INVOKABLE void addCamerasToIHMSelection(const QStringList& cameraNames, bool center = false);
    Q_INVOKABLE void addCamerasToMayaSelection(const QStringList& cameraNames) const;
    Q_INVOKABLE void addMeshesToIHMSelection(const QStringList& selectedMeshes,
                                             bool center = false);
    Q_INVOKABLE void addMeshesToMayaSelection(const QStringList& meshes) const;
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void activeSelectionContext() const;
    Q_INVOKABLE void activeMVGContext();
    Q_INVOKABLE void loadExistingProject();
    Q_INVOKABLE void loadNewProject(const QString& projectDirectoryPath);
    Q_INVOKABLE void scaleScene(const double scaleSize) const;
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName);
    Q_INVOKABLE void setCamerasNear(const double near);
    Q_INVOKABLE void setCamerasFar(const double far);
    Q_INVOKABLE void setCameraLocatorScale(const double scale);
    // Should be a private and non invokable function
    Q_INVOKABLE void reloadMVGMeshesFromMaya();

    void clear();
    void removeCameraFromUI(MDagPath& cameraPath);
    void addMeshToUI(const MDagPath& meshPath);
    void removeMeshFromUI(const MDagPath& meshPath);
    void emitCurrentUnitChanged();
    void clearCameraSelection();
    void clearMeshSelection();

private:
    void reloadMVGCamerasFromMaya();

private:
    QObjectListModel _cameraList;
    QObjectListModel _meshesList;
    // DagPaths of the selected cameras as stringNames of the selected cameras
    /// Selection already stored in camera but this list is needed for a faster access
    QStringList _selectedCameras;
    /// DagPaths of the selected meshes as string
    /// Selection already stored in mesh but this list is needed for a faster access
    QStringList _selectedMeshes;
    MVGProject _project;
    QObjectListModel _panelList;
    QString _currentContext;
    bool _isProjectLoading;

    std::map<std::string, MVGCameraWrapper*> _camerasByName;
    std::map<std::string, MVGMeshWrapper*> _meshesByName;
    /// map view to active camera
    std::map<std::string, std::string> _activeCameraNameByView;
    QMap<MDistance::Unit, QString> _unitMap;
};

} // namespace
