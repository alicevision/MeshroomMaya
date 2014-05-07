#include "mayaMVG/qt/MVGCameraWrapper.h"
#include "mayaMVG/qt/MVGProjectWrapper.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/maya/MVGMayaUtil.h"

namespace mayaMVG {

MVGCameraWrapper::MVGCameraWrapper(const MVGCamera& camera)
	: _camera(camera)
	, _state("NORMAL")
	, _isLeftChecked(false)
	, _isRightChecked(false)
{
}

MVGCameraWrapper::~MVGCameraWrapper() {
}

const MVGCamera& MVGCameraWrapper::camera() const
{
	return _camera;
}

const QString MVGCameraWrapper::name() const 
{
	return QString(_camera.name().c_str());
}

const QString MVGCameraWrapper::imagePath() const
{
	return _camera.imagePlane().c_str();
}

const QString& MVGCameraWrapper::state() const
{
	return _state;
}

const bool MVGCameraWrapper::isLeftChecked() const
{
	return _isLeftChecked;
}

const bool MVGCameraWrapper::isRightChecked() const
{
	return _isRightChecked;
}

void MVGCameraWrapper::setState(const QString& state)
{
	_state = state;
	emit stateChanged();
}

void  MVGCameraWrapper::setLeftChecked(const bool state)
{
	_isLeftChecked = state;
	emit isLeftCheckedChanged();
}

void  MVGCameraWrapper::setRightChecked(const bool state)
{
	_isRightChecked = state;
	emit isRightCheckedChanged();
}

void MVGCameraWrapper::onLeftButtonClicked() 
{	
	// Reset left states
	for(int i = 0; i < MVGProjectWrapper::instance().cameraModel().size(); ++i)
	{
		dynamic_cast<MVGCameraWrapper*>(MVGProjectWrapper::instance().getCameraAtIndex(i))->setLeftChecked(false);
	}
	
	// Update new left button
	setLeftChecked(true);
	
	// Update left view
	camera().select();
	camera().loadImagePlane();
	MVGMayaUtil::setMVGLeftCamera(camera());
}

void MVGCameraWrapper::onRightButtonClicked()
{
	// Reset right states
	for(int i = 0; i < MVGProjectWrapper::instance().cameraModel().size(); ++i)
	{
		dynamic_cast<MVGCameraWrapper*>(MVGProjectWrapper::instance().getCameraAtIndex(i))->setRightChecked(false);
	}
	
	// Update new right button
	setRightChecked(true);
	
	// Update right view
	camera().select();
	camera().loadImagePlane();
	MVGMayaUtil::setMVGRightCamera(camera());
}

}
