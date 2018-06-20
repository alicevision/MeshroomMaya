#include "MVGCameraSetWrapper.hpp"
#include "MVGCameraWrapper.hpp"
#include "meshroomMaya/core/MVGProject.hpp"

#include <maya/MItDependencyNodes.h>

namespace meshroomMaya
{

const MColor MVGCameraSetWrapper::LOCATOR_HIGHLIGHT_COLOR = MColor(0.37f, 0.91f, 0.65f, 1.0f);
    
MVGCameraSetWrapper::MVGCameraSetWrapper(const QString& displayName, QObject* parent):
QObject(parent),
_displayName(displayName),
_cameraWrappers(this)
{
}

MVGCameraSetWrapper::MVGCameraSetWrapper(const MObject& set, QObject* parent):
QObject(parent),
_cameraWrappers(this)
{
    _fnSet.setObject(set);
    std::string shortName = _fnSet.name().asChar();
    shortName = shortName.substr(MVGProject::_CAMERASET_PREFIX.size(), shortName.size());
    _displayName = QString::fromStdString(shortName);
}

MVGCameraSetWrapper::MVGCameraSetWrapper(const MVGCameraSetWrapper& other):
QObject(other.parent()),
_displayName(other._displayName),
_cameraWrappers(this)
{
    _fnSet.setObject(other.fnSet().object());
    _cameraWrappers.setObjectList(other._cameraWrappers.objectList());
}

MVGCameraSetWrapper::~MVGCameraSetWrapper()
{
}

void MVGCameraSetWrapper::highlightLocators(bool highlight)
{
    for(auto* cam : _cameraWrappers.asQList<MVGCameraWrapper>())
    {
        if(!cam->getViews().empty())
            continue;  // Already defines a custom locator color matching the panel's color
        cam->getCamera().setLocatorCustomColor(highlight, highlight ? LOCATOR_HIGHLIGHT_COLOR : MColor());
    }
}

}
