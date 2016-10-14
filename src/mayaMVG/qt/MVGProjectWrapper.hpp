#pragma once

#include "mayaMVG/qt/QObjectListModel.hpp"
#include "mayaMVG/qt/MVGPanelWrapper.hpp"
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
#include "mayaMVG/qt/MVGCameraSetWrapper.hpp"
#include "mayaMVG/qt/MVGMeshWrapper.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "maya/MDistance.h"
#include <QObject>
#include <set>

namespace mayaMVG
{

class MVGProjectWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ getProjectDirectory NOTIFY projectDirectoryChanged)
    Q_PROPERTY(int editMode READ getEditMode NOTIFY editModeChanged)
    Q_PROPERTY(int moveMode READ getMoveMode NOTIFY moveModeChanged)
    Q_PROPERTY(QObjectListModel* cameraSets READ getCameraSets CONSTANT)
    Q_PROPERTY(QObjectListModel* meshModel READ getMeshModel NOTIFY meshModelChanged)

    Q_PROPERTY(mayaMVG::MVGCameraSetWrapper* currentCameraSet READ getCurrentCameraSet
               NOTIFY currentCameraSetChanged)
    Q_PROPERTY(int currentCameraSetIndex READ getCurrentCameraSetIndex WRITE setCurrentCameraSetIndex
               NOTIFY currentCameraSetIndexChanged)

    Q_PROPERTY(bool useParticleSelection READ useParticleSelection WRITE setUseParticleSelection
               NOTIFY useParticleSelectionChanged)
    Q_PROPERTY(int particleSelectionTolerance READ getParticleSelectionTolerance
               WRITE setParticleSelectionTolerance NOTIFY particleSelectionToleranceChanged)
    Q_PROPERTY(QString currentContext READ getCurrentContext WRITE setCurrentContext NOTIFY
                   currentContextChanged)
    Q_PROPERTY(QObjectListModel* panelList READ getPanelList NOTIFY panelListChanged)
    Q_PROPERTY(QString currentUnit READ getCurrentUnit NOTIFY currentUnitChanged)
    Q_PROPERTY(QString pluginVersion READ getPluginVersion CONSTANT)
    Q_PROPERTY(bool isProjectLoading READ getIsProjectLoading NOTIFY isProjectLoadingChanged)
    Q_PROPERTY(bool activeSynchro READ getActiveSynchro WRITE setActiveSynchro NOTIFY
                   activeSynchroChanged)
    

    Q_PROPERTY(int cameraSelectionCount READ getCameraSelectionCount NOTIFY cameraSelectionCountChanged)

    Q_PROPERTY(int cameraPointsDisplayMode READ getCameraPointsDisplayMode
               WRITE setCameraPointsDisplayMode NOTIFY cameraPointsDisplayModeChanged)

public:
    MVGProjectWrapper(QObject* parent=nullptr);
    ~MVGProjectWrapper();

public Q_SLOTS:
    const QString getProjectDirectory() const;
    void setProjectDirectory(const QString& directory);
    int getEditMode() const { return _editMode; }
    int getMoveMode() const { return _moveMode; }
    int getCameraPointsDisplayMode() const;
    void setCameraPointsDisplayMode(int mode);

    int getCurrentCameraSetIndex() const;
    void setCurrentCameraSetIndex(int value);

    MVGCameraSetWrapper* getCurrentCameraSet() { return _currentCameraSet; }

    QObjectListModel* getCameraSets() { return &_cameraSets; }
    QObjectListModel* getMeshModel() { return &_meshesList; }
    QStringList& getSelectedCameras() { return _selectedCameras; }
    QStringList& getSelectedMeshes() { return _selectedMeshes; }
    QObjectListModel* getPanelList() { return &_panelList; }
    const QString getCurrentContext() const;
    void setCurrentContext(const QString&);
    const QString getCurrentUnit() const;
    const QString getPluginVersion() const;
    bool getIsProjectLoading() const { return _isProjectLoading; }
    void setIsProjectLoading(const bool value);
    bool getActiveSynchro() const { return _activeSynchro; }
    void setActiveSynchro(const bool value);

    int getCameraSelectionCount() { return _selectedCameras.count(); }

    bool useParticleSelection() const;
    void setUseParticleSelection(bool value);
    void updateParticleSelection(const std::set<int>& selection);

    int getParticleSelectionTolerance() const { return _selectionTolerance; }
    void setParticleSelectionTolerance(int value) {
        if(_selectionTolerance == value)
            return;
        _selectionTolerance = value;
        updateCamerasFromParticleSelection();
        Q_EMIT particleSelectionToleranceChanged();
    }

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
    void cameraSelectionCountChanged();
    void centerCameraListByIndex(const int cameraIndex);
    void centerMeshListByIndex(const int meshIndex);
    void currentCameraSetChanged();
    void currentCameraSetIndexChanged();
    void cameraPointsDisplayModeChanged();
    void useParticleSelectionChanged();
    void particleSelectionToleranceChanged();

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
    Q_INVOKABLE void addCamerasToMayaSelection(const QStringList& cameraNames);
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
    // Camera Sets
    Q_INVOKABLE void createCameraSetFromSelection(const QString& name, bool makeCurrent);
    Q_INVOKABLE void duplicateCameraSet(const QString& copyName, mayaMVG::MVGCameraSetWrapper* sourceSet, bool makeCurrent);
    void createCameraSetFromDagPaths(const QString& name, const QStringList& paths, bool makeCurrent);
    /// Delete the Maya set corresponding to the given set wrapper
    Q_INVOKABLE void deleteCameraSet(mayaMVG::MVGCameraSetWrapper* setWrapper);
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
    void addCameraSetToUI(MObject& set, bool makeCurrent=false);
    void removeCameraSetFromUI(MObject& set);
    // Signals
    void emitCurrentUnitChanged();
    void emitCameraPointsDisplayModeChanged();
    // Setter not callable from QML
    void setEditMode(const int mode);
    void setMoveMode(const int mode);
    void updatePanelColor(const QString& viewName);
    
private:
    void initCameraPointsLocator();
    void updatePointsVisibility();
    void reloadMVGCamerasFromMaya();
    /// Update members of the camera set based on particle selection
    void updateCamerasFromParticleSelection(bool force=false);
    /// Update set's MVGCameraSetWrapper members (MVGCameraWrappers)
    void updateCameraSetWrapperMembers(const MObject &set);
    /// Use 'wrapper' as current camera set
    void setCurrentCameraSet(MVGCameraSetWrapper *wrapper);
    MVGCameraWrapper* cameraFromViewName(const QString& viewName);
    MVGPanelWrapper* panelFromViewName(const QString& viewName);

private:
    QObjectListModel _cameraSets;
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

    int _currentCameraSetId;
    std::set<int> _particleSelection;
    std::map<MVGCameraWrapper*, int> _selectionScorePerCamera;
    std::map<int, std::vector<MVGCameraWrapper*>> _camerasPerPoint;
    int _selectionTolerance;
    MVGCameraSetWrapper* _defaultCameraSet;
    MVGCameraSetWrapper* _currentCameraSet;
    MVGCameraSetWrapper* _particleSelectionCameraSet;

    std::map<std::string, MVGCameraWrapper*> _camerasByName;
    std::map<std::string, MVGMeshWrapper*> _meshesByName;
    std::map<std::string, MVGCameraSetWrapper*> _cameraSetsByName;
    /// map view to active camera
    std::map<std::string, std::string> _activeCameraNameByView;
    QMap<MDistance::Unit, QString> _unitMap;

    MCallbackId _cameraPointsLocatorCB;
    std::map<std::string, MCallbackIdArray> _nodeCallbacks;
};

} // namespace
