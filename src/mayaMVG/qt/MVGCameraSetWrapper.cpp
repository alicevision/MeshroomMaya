#include "MVGCameraSetWrapper.hpp"
#include <mayaMVG/core/MVGProject.hpp>
#include <maya/MItDependencyNodes.h>

namespace mayaMVG
{
    
MVGCameraSetWrapper::MVGCameraSetWrapper(const QString& displayName, QObject* parent):
QObject(parent),
_displayName(displayName)
{
}

MVGCameraSetWrapper::MVGCameraSetWrapper(const MObject& set, QObject* parent):
QObject(parent)
{
    _fnSet.setObject(set);
    std::string shortName = _fnSet.name().asChar();
    shortName = shortName.substr(MVGProject::_CAMERASET_PREFIX.size(), shortName.size());
    _displayName = QString::fromStdString(shortName);
}

MVGCameraSetWrapper::MVGCameraSetWrapper(const MVGCameraSetWrapper& other):
QObject(other.parent()),
_displayName(other._displayName)
{
    _fnSet.setObject(other.fnSet().object());
    _cameraWrappers.setObjectList(other._cameraWrappers.objectList());
}

MVGCameraSetWrapper::~MVGCameraSetWrapper()
{
}

}
