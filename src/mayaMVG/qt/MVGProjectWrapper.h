#pragma once

#include <QObject>
#include <QFileDialog>
#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/patterns/Singleton.h"
#include "mayaMVG/qt/QObjectListModel.h"

namespace mayaMVG {

class MVGProjectWrapper : public QObject, public Singleton<MVGProjectWrapper>
{
    Q_OBJECT

    Q_PROPERTY(QString projectDirectory READ projectDirectory NOTIFY projectDirectoryChanged);
    Q_PROPERTY(QString cameraDirectory READ cameraDirectory NOTIFY cameraDirectoryChanged);
    Q_PROPERTY(QString imageDirectory READ imageDirectory NOTIFY imageDirectoryChanged);
    Q_PROPERTY(QString pointCloudFile READ pointCloudFile NOTIFY pointCloudFileChanged);
    Q_PROPERTY(QObjectListModel* cameraModel READ cameraModel NOTIFY cameraModelChanged);
    Q_PROPERTY(QString logText READ logText WRITE setLogText NOTIFY logTextChanged);
    MAKE_SINGLETON_WITHCONSTRUCTORS(MVGProjectWrapper)

public:
    Q_INVOKABLE const QString moduleDirectory() const;
    Q_INVOKABLE const QString projectDirectory() const;
    Q_INVOKABLE const QString cameraDirectory() const;
    Q_INVOKABLE const QString imageDirectory() const;
    Q_INVOKABLE const QString pointCloudFile() const;
    Q_INVOKABLE QObjectListModel* cameraModel() {return &_cameraList;}
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

private:
    void addCamera(const MVGCamera& camera);

signals:
    void projectDirectoryChanged();
    void cameraDirectoryChanged();
    void imageDirectoryChanged();
    void pointCloudFileChanged();
    void cameraModelChanged();
    void logTextChanged();

private:
    QObjectListModel _cameraList;
    MVGProject _project;
    QString _logText;

};

} // mayaMVG
