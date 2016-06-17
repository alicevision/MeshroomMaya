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

    Q_PROPERTY(QString projectDirectory READ getProjectDirectory NOTIFY projectDirectoryChanged);
    Q_PROPERTY(int editMode READ getEditMode NOTIFY editModeChanged);
    Q_PROPERTY(int moveMode READ getMoveMode NOTIFY moveModeChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
    Q_PROPERTY(QObjectListModel* meshModel READ getMeshModel NOTIFY meshModelChanged);
    Q_PROPERTY(QString currentContext READ getCurrentContext WRITE setCurrentContext NOTIFY
                   currentContextChanged);
    Q_PROPERTY(QObjectListModel* panelList READ getPanelList NOTIFY panelListChanged);
    Q_PROPERTY(QString currentUnit READ getCurrentUnit NOTIFY currentUnitChanged);
    Q_PROPERTY(QString pluginVersion READ getPluginVersion CONSTANT);
    Q_PROPERTY(bool isProjectLoading READ getIsProjectLoading NOTIFY isProjectLoadingChanged);
    Q_PROPERTY(bool activeSynchro READ getActiveSynchro WRITE setActiveSynchro NOTIFY
                   activeSynchroChanged);

public:
    MVGProjectWrapper();
    ~MVGProjectWrapper();

public Q_SLOTS:
    const QString getProjectDirectory() const;
    void setProjectDirectory(const QString& directory);
    const int getEditMode() const { return _editMode; }
    const int getMoveMode() const { return _moveMode; }
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
    bool getActiveSynchro() const { return _activeSynchro; }
    void setActiveSynchro(const bool value);

Q_SIGNALS:
    void projectDirectoryChanged();
    void editModeChanged();
    void moveModeChanged();
    void cameraModelChanged();
    void meshModelChanged();
    void currentContextChanged();
    void panelListChanged();
    void currentUnitChanged();
    void isProjectLoadingChanged();
    void activeSynchroChanged();
    void centerCameraListByIndex(const int cameraIndex);
    void centerMeshListByIndex(const int meshIndex);

public:
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void applySceneTransformation() const;
    // Context & Modes
    Q_INVOKABLE void activeSelectionContext() const;
    Q_INVOKABLE void setCreationMode();
    Q_INVOKABLE void setTriangulationMode();
    Q_INVOKABLE void setPointCloudMode();
    Q_INVOKABLE void setAdjacentPlaneMode();
    Q_INVOKABLE void setLocatorMode();
    // Project
    Q_INVOKABLE void loadExistingProject();
    Q_INVOKABLE void loadABC(const QString& abcFilePath);
    Q_INVOKABLE void remapPaths(const QString& abcFilePath);
    // Selection
    Q_INVOKABLE void addCamerasToIHMSelection(const QStringList& cameraNames, bool center = false);
    Q_INVOKABLE void addCamerasToMayaSelection(const QStringList& cameraNames) const;
    Q_INVOKABLE void addMeshesToIHMSelection(const QStringList& selectedMeshes,
                                             bool center = false);
    Q_INVOKABLE void addMeshesToMayaSelection(const QStringList& meshes) const;
    Q_INVOKABLE void selectClosestCam() const;
    // Cameras
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName);
    Q_INVOKABLE void setCamerasNear(const double near);
    Q_INVOKABLE void setCamerasFar(const double far);
    Q_INVOKABLE void setCamerasDepth(const double far);
    Q_INVOKABLE void setCameraLocatorScale(const double scale);
    // Meshes
    Q_INVOKABLE void clearAllBlindData();
    // Should be a private and non invokable function
    Q_INVOKABLE void reloadMVGMeshesFromMaya();

    // Clear
    void clear();
    void clearAndUnloadImageCache();
    void clearCameraSelection();
    void clearMeshSelection();
    // UI
    void removeCameraFromUI(MDagPath& cameraPath);
    void addMeshToUI(const MDagPath& meshPath);
    void removeMeshFromUI(const MDagPath& meshPath);
    // Signals
    void emitCurrentUnitChanged();
    // Setter not callable from QML
    void setEditMode(const int mode);
    void setMoveMode(const int mode);

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
    int _editMode;
    int _moveMode;
    bool _isProjectLoading;
    bool _activeSynchro;

    std::map<std::string, MVGCameraWrapper*> _camerasByName;
    std::map<std::string, MVGMeshWrapper*> _meshesByName;
    /// map view to active camera
    std::map<std::string, std::string> _activeCameraNameByView;
    QMap<MDistance::Unit, QString> _unitMap;
};

} // namespace
