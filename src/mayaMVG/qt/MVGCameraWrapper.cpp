#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/MVGMayaUtil.h"

namespace mayaMVG {

MVGCameraWrapper::MVGCameraWrapper(const MVGCamera& camera)
	: _camera(camera)
    , _isSelected(false)
    , _imageLoaded(false)
{
}

MVGCameraWrapper::~MVGCameraWrapper() {
}

const MVGCamera& MVGCameraWrapper::camera() const
{
	return _camera;
}

void MVGCameraWrapper::setInView(const QString& viewName, const bool value)
{
    if(value) {
        if(!_views.contains(viewName)){
            _views.push_back(viewName);
            Q_EMIT viewsChanged();
        }
        return;
    }
    if(_views.removeOne(viewName))
        Q_EMIT viewsChanged();
}

const QSize MVGCameraWrapper::sourceSize()
{
    if(!_imageLoaded) {
        QImage image(imagePath());
        _imageLoaded = true;
        _imageSize = image.size();
    }
    return _imageSize;
}

const qint64 MVGCameraWrapper::sourceWeight() const
{
	QFileInfo info(imagePath());
	return info.size();
}

void MVGCameraWrapper::select() const{
	_camera.select();
}

}
