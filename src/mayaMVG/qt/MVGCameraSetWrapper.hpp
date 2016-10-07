#pragma once

#include "MVGQt.hpp"
#include "QObjectListModel.hpp"
#include <maya/MFnSet.h>
#include <QtDeclarative>

/**
 * MVGCameraSetWrapper is an object exposing a list of cameras as a Qt model.
 * It can be based on a Maya set or
 */
namespace mayaMVG 
{
    
class MVGCameraSetWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getDisplayName NOTIFY nameChanged)
    Q_PROPERTY(QObjectListModel* cameras READ getCameras CONSTANT)
    Q_PROPERTY(bool editable READ isEditable CONSTANT)

public:
    MVGCameraSetWrapper(const QString& displayName="-", QObject* parent=nullptr);
    MVGCameraSetWrapper(const MObject& set, QObject* parent=nullptr);
    MVGCameraSetWrapper(const MVGCameraSetWrapper& other);
    virtual ~MVGCameraSetWrapper();

    const MFnSet& fnSet() const { return _fnSet; }
    
    QString getDisplayName() { return _displayName; }
    QObjectListModel* getCameras() { return &_cameraWrappers; }
    
    /// Sets the camera wrappers corresponding to this camera set
    void setCameraWrappers(const QObjectList& camWrappers)
    {
        _cameraWrappers.setObjectList(camWrappers);
    }

    bool isEditable() {
        // Only sets corresponding to a real maya setObjet are editable/deletable
        return !_fnSet.object().isNull();
    }

signals:
    void nameChanged();

private:
    MFnSet _fnSet;
    QString _displayName;
    QObjectListModel _cameraWrappers;
};

}
