#include "mayaMVG/qt/MVGCameraWrapper.h"
#include <QImage>
#include <QFileInfo>

namespace mayaMVG
{

MVGCameraWrapper::MVGCameraWrapper(const MVGCamera& camera)
    : _camera(camera)
    , _isSelected(false)
    , _imageLoaded(false)
{
}

MVGCameraWrapper::MVGCameraWrapper(const MVGCameraWrapper& other)
    : _camera(other._camera)
    , _imageLoaded(other._imageLoaded)
    , _imageSize(other._imageSize)
    , _isSelected(other._isSelected)
    , _views(other._views)
{
}

MVGCameraWrapper::~MVGCameraWrapper()
{
}

const MVGCamera& MVGCameraWrapper::getCamera() const
{
    return _camera;
}

void MVGCameraWrapper::setInView(const QString& viewName, const bool value)
{
    if(value)
    {
        if(!_views.contains(viewName))
        {
            _views.push_back(viewName);
            Q_EMIT viewsChanged();
        }
        _camera.setInView(viewName.toStdString());
        return;
    }
    if(_views.removeOne(viewName))
        Q_EMIT viewsChanged();
}

const QSize MVGCameraWrapper::getSourceSize()
{
    if(!_imageLoaded)
    {
        QImage image(getImagePath());
        _imageLoaded = true;
        _imageSize = image.size();
    }
    return _imageSize;
}

const qint64 MVGCameraWrapper::getSourceWeight() const
{
    QFileInfo info(getImagePath());
    return info.size();
}

void MVGCameraWrapper::select() const
{
    _camera.select();
}

} // namespace
