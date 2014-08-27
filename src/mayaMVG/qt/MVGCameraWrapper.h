#pragma once

#include <QObject>
#include <QSize>
#include <QStringList>
#include "mayaMVG/core/MVGCamera.h"

namespace mayaMVG {

class MVGCameraWrapper : public QObject 
{
        Q_OBJECT

		Q_PROPERTY(QString name READ getName CONSTANT)
		Q_PROPERTY(QString imagePath READ getImagePath CONSTANT)
        Q_PROPERTY(bool isSelected READ isSelected WRITE setIsSelected NOTIFY isSelectedChanged)
        Q_PROPERTY(QStringList views READ getViews NOTIFY viewsChanged)
		Q_PROPERTY(QSize sourceSize READ getSourceSize CONSTANT)
		Q_PROPERTY(qint64 sourceWeight READ getSourceWeight CONSTANT)

    public:
        MVGCameraWrapper(const MVGCamera& camera);
        MVGCameraWrapper(const MVGCameraWrapper& other)
            : _camera(other._camera)
            , _imageLoaded(other._imageLoaded)
            , _imageSize(other._imageSize)
            , _isSelected(other._isSelected)
            , _views(other._views) {}
		~MVGCameraWrapper();

	public slots:
		const QString getName() const { return QString::fromStdString(_camera.getName()); }
		const QString getImagePath() const { return QString::fromStdString(_camera.getImagePlane()); }
        bool isSelected() const { return _isSelected; }
		void setIsSelected(const bool isSelected) { _isSelected = isSelected; Q_EMIT isSelectedChanged(); }
		const QStringList getViews() { return _views; }
        const QSize getSourceSize();
		const qint64 getSourceWeight() const;

	signals:
        void isSelectedChanged();
        void viewsChanged();

	public:
        const MVGCamera& getCamera() const;
        Q_INVOKABLE bool isInView(const QString& viewName) const {return _views.contains(viewName);}
        Q_INVOKABLE void setInView(const QString& viewName, const bool value);
		Q_INVOKABLE void select() const;

	private:
		const MVGCamera _camera;
        bool _imageLoaded;
        QSize _imageSize;
        bool _isSelected;
        QStringList _views; //< camera is displayed in thoses views
};

}
