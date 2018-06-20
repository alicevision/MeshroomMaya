#pragma once

#include "meshroomMaya/core/MVGCamera.hpp"
#include <QObject>
#include <QSize>
#include <QStringList>

namespace meshroomMaya
{

class MVGCameraWrapper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString dagPath READ getDagPathAsString CONSTANT)
    Q_PROPERTY(QString imagePath READ getImagePath CONSTANT)
    Q_PROPERTY(bool isSelected READ isSelected WRITE setIsSelected NOTIFY isSelectedChanged)
    Q_PROPERTY(QStringList views READ getViews NOTIFY viewsChanged)
    Q_PROPERTY(QSize sourceSize READ getSourceSize CONSTANT)
    Q_PROPERTY(qint64 sourceWeight READ getSourceWeight CONSTANT)

public:
    MVGCameraWrapper(const MVGCamera& camera, QObject* parent=nullptr);
    MVGCameraWrapper(const MVGCameraWrapper& other);
    ~MVGCameraWrapper();

public Q_SLOTS:
    const QString getName() const { return QString::fromStdString(_camera.getName()); }
    const QString getDagPathAsString() const;
    const QString getImagePath() const
    {
        return QString::fromStdString(_camera.getThumbnailPath());
    }
    bool isSelected() const { return _isSelected; }
    void setIsSelected(const bool isSelected);
    const QStringList& getViews() const { return _views; }
    const QSize getSourceSize();
    const qint64 getSourceWeight() const;

Q_SIGNALS:
    void isSelectedChanged();
    void viewsChanged();

public:
    const MVGCamera& getCamera() const;
    Q_INVOKABLE bool isInView(const QString& viewName) const { return _views.contains(viewName); }
    Q_INVOKABLE void setInView(const QString& viewName, const bool value);
    Q_INVOKABLE void selectCameraNode() const;

private:
    const MVGCamera _camera;
    bool _imageLoaded;
    QSize _imageSize;
    bool _isSelected;
    QStringList _views; //< camera is displayed in thoses views
};

} // namespace
