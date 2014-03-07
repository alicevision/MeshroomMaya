#include "mayaMVG/core/MVGCamera.h"
#include <maya/MPoint.h>
#include <maya/MString.h>

using namespace mayaMVG;

MVGCamera::MVGCamera(const MDagPath& path)
	: _dagpath(path)
	, _step(STEP_NEW)
{
}

MVGCamera::~MVGCamera()
{
}

void MVGCamera::add2DPoint(const MPoint&)
{
}

void MVGCamera::move2DPoint(const MPoint&)
{
}

void MVGCamera::setImagePlane(const MString& name)
{
}

void MVGCamera::setZoom(float z)
{
}

void MVGCamera::setPan(float x, float y)
{
}
