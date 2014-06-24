#pragma once

#include <QObject>
#include <QSize>
#include <QtDeclarative>
#include <QDeclarativeListProperty>
#include "mayaMVG/core/MVGCamera.h"

namespace mayaMVG {

class MVGCameraWrapper : public QObject 
{
        Q_OBJECT

		Q_PROPERTY(QString name READ name CONSTANT)
		Q_PROPERTY(QString imagePath READ imagePath CONSTANT)
        Q_PROPERTY(bool isSelected READ isSelected NOTIFY isSelectedChanged)
        Q_PROPERTY(QStringList views READ views NOTIFY viewsChanged)
		Q_PROPERTY(QSize sourceSize READ sourceSize CONSTANT)
		Q_PROPERTY(qint64 sourceWeight READ sourceWeight CONSTANT)

    public:
        MVGCameraWrapper(const MVGCamera& camera);
        MVGCameraWrapper(const MVGCameraWrapper& other)
            : _camera(other._camera)
            , _imageLoaded(other._imageLoaded)
            , _imageSize(other._imageSize)
            , _isSelected(other._isSelected)
            , _views(other._views) {}
		~MVGCameraWrapper();

	public:
		const MVGCamera& camera() const;
        Q_INVOKABLE QString name() const { return QString::fromStdString(_camera.name()); }
        Q_INVOKABLE QString imagePath() const { return QString::fromStdString(_camera.imagePlane()); }
        Q_INVOKABLE bool isSelected() const { return _isSelected; }
        Q_INVOKABLE bool isInView(const QString& viewName) const {return _views.contains(viewName);}
        Q_INVOKABLE void setIsSelected(const bool isSelected) { _isSelected = isSelected; Q_EMIT isSelectedChanged(); }
        Q_INVOKABLE void setInView(const QString& viewName, const bool value);
        Q_INVOKABLE QStringList views() { return _views; }
        Q_INVOKABLE const QSize sourceSize();
		Q_INVOKABLE const qint64 sourceWeight() const;
		Q_INVOKABLE void select() const;

	signals:
        void isSelectedChanged();
        void viewsChanged();

	private:
		const MVGCamera _camera;
        bool _imageLoaded;
        QSize _imageSize;
        bool _isSelected;
        QStringList _views; //< camera is displayed in thoses views
};

}
