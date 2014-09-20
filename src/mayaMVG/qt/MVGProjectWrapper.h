#pragma once

#include <QObject>
#include <QStringList>
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/qt/QObjectListModel.h"
#include "mayaMVG/qt/MVGPanelWrapper.h"

namespace mayaMVG
{

class MVGProjectWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ getProjectDirectory WRITE setProjectDirectory NOTIFY
                   projectDirectoryChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ getCameraModel NOTIFY cameraModelChanged);
    Q_PROPERTY(QString currentContext READ getCurrentContext WRITE setCurrentContext NOTIFY
                   currentContextChanged);
    Q_PROPERTY(QObjectListModel* panelList READ getPanelList NOTIFY panelListChanged)

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

signals:
    void projectDirectoryChanged();
    void cameraModelChanged();
    void panelModelChanged();
    void currentContextChanged();
    void panelListChanged();

public:
    void selectItems(const QList<QString>& cameraNames) const;
    Q_INVOKABLE void selectCameras(const QStringList& cameraNames) const;
    Q_INVOKABLE QString openFileDialog() const;
    Q_INVOKABLE void activeSelectionContext() const;
    Q_INVOKABLE void activeMVGContext();
    Q_INVOKABLE void loadExistingProject();
    Q_INVOKABLE void loadNewProject(const QString& projectDirectoryPath);
    Q_INVOKABLE void setCameraToView(QObject* camera, const QString& viewName,
                                     bool rebuildCache = true) const;
    void clear();
    void removeCameraFromUI(MDagPath& cameraPath);

private:
    void reloadMVGCamerasFromMaya();

private:
    QObjectListModel _cameraList;
    MVGProject _project;
    QObjectListModel _panelList;
    QString _currentContext;
};

} // namespace
