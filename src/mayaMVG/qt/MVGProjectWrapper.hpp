#pragma once

#include "mayaMVG/qt/QObjectListModel.hpp"
#include "mayaMVG/qt/MVGPanelWrapper.hpp"
#include "mayaMVG/qt/MVGCameraWrapper.hpp"
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
    Q_PROPERTY(int editMode READ getEditMode NOTIFY editModeChanged);
    Q_PROPERTY(int moveMode READ getMoveMode NOTIFY moveModeChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
    Q_PROPERTY(QString currentContext READ getCurrentContext WRITE setCurrentContext NOTIFY
                   currentContextChanged);
    Q_PROPERTY(QObjectListModel* panelList READ getPanelList NOTIFY panelListChanged);
    Q_PROPERTY(QString currentUnit READ getCurrentUnit NOTIFY currentUnitChanged);
    Q_PROPERTY(QString pluginVersion READ getPluginVersion CONSTANT);
    Q_PROPERTY(bool isProjectLoading READ getIsProjectLoading NOTIFY isProjectLoadingChanged);

public:
    MVGProjectWrapper();
    ~MVGProjectWrapper();

public slots:
    const QString getProjectDirectory() const;
    void setProjectDirectory(const QString& directory);
    const int getEditMode() const { return _editMode; }
    const int getMoveMode() const { return _moveMode; }
    QObjectListModel* getCameraModel() { return &_cameraList; }
    QStringList& getSelectedCameras() { return _selectedCameras; }
    QObjectListModel* getPanelList() { return &_panelList; }
    const QString getCurrentContext() const;
    void setCurrentContext(const QString&);
    const QString getCurrentUnit() const;
    const QString getPluginVersion() const;
    const bool getIsProjectLoading() const { return _isProjectLoading; }
    void setIsProjectLoading(const bool value);

signals:
    void projectDirectoryChanged();
    void editModeChanged();
    void moveModeChanged();
    void cameraModelChanged();
    void currentContextChanged();
    void panelListChanged();
    void currentUnitChanged();
    void isProjectLoadingChanged();
    void centerCameraListByIndex(const int cameraIndex);

public:
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void scaleScene(const double scaleSize) const;
    // Context & Modes
    Q_INVOKABLE void activeSelectionContext() const;
    Q_INVOKABLE void setCreationMode();
    Q_INVOKABLE void setTriangulationMode();
    Q_INVOKABLE void setPointCloudMode();
    Q_INVOKABLE void setAdjacentPlaneMode();
    // Project
    Q_INVOKABLE void loadExistingProject();
    Q_INVOKABLE void loadNewProject(const QString& projectDirectoryPath);
    // Selection
    Q_INVOKABLE void addCamerasToIHMSelection(const QStringList& cameraNames, bool center = false);
    Q_INVOKABLE void addCamerasToMayaSelection(const QStringList& cameraNames) const;
    // Cameras
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName,
                                     bool rebuildCache = true);
    Q_INVOKABLE void setCamerasNear(const double near);
    Q_INVOKABLE void setCamerasFar(const double far);
    Q_INVOKABLE void setCameraLocatorScale(const double scale);

    void clear();
    void removeCameraFromUI(MDagPath& cameraPath);
    void emitCurrentUnitChanged();
    void emitModeChanged();

    // Setter not callable from QML
    void setEditMode(const int mode);
    void setMoveMode(const int mode);

private:
    void reloadMVGCamerasFromMaya();

private:
    QObjectListModel _cameraList;
    /// Names of the selected cameras
    /// Selection already stored in camera but this list is needed for a faster access
    QStringList _selectedCameras;
    MVGProject _project;
    QObjectListModel _panelList;
    QString _currentContext;
    int _editMode;
    int _moveMode;
    bool _isProjectLoading;

    std::map<std::string, MVGCameraWrapper*> _camerasByName;
    /// map view to active camera
    std::map<std::string, std::string> _activeCameraNameByView;
    QMap<MDistance::Unit, QString> _unitMap;
};

} // namespace
