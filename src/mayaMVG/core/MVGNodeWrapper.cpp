#include "mayaMVG/core/MVGNodeWrapper.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include <maya/MGlobal.h>
#include <maya/MFnDagNode.h>
#include <maya/MSelectionList.h>

namespace mayaMVG
{

MVGNodeWrapper::MVGNodeWrapper()
{
}

MVGNodeWrapper::MVGNodeWrapper(const std::string& name)
{
    if(name.empty())
        return;
    MVGMayaUtil::getDagPathByName(name.c_str(), _dagpath);
    if(_dagpath.apiType() == MFn::kTransform)
        _dagpath.extendToShape();
    _object = _dagpath.node();
}

MVGNodeWrapper::MVGNodeWrapper(const MString& name)
{
    if(name.length() == 0)
        return;
    MVGMayaUtil::getDagPathByName(name, _dagpath);
    if(_dagpath.apiType() == MFn::kTransform)
        _dagpath.extendToShape();
    _object = _dagpath.node();
}

MVGNodeWrapper::MVGNodeWrapper(const MDagPath& dagPath)
    : _dagpath(dagPath)
{
    if(_dagpath.apiType() == MFn::kTransform)
        _dagpath.extendToShape();
    _object = _dagpath.node();
}

MVGNodeWrapper::MVGNodeWrapper(const MObject& object)
    : _object(object)
{
    MDagPath::getAPathTo(object, _dagpath);
    if(_dagpath.apiType() == MFn::kTransform)
        _dagpath.extendToShape();
}

void MVGNodeWrapper::selectNode() const
{
    MSelectionList list;
    list.add(_dagpath);
    MGlobal::setActiveSelectionList(list);
}

const MDagPath& MVGNodeWrapper::getDagPath() const
{
    return _dagpath;
}

const MObject& MVGNodeWrapper::getObject() const
{
    return _object;
}

std::string MVGNodeWrapper::getName() const
{
    MObject obj = _object;
    if(_dagpath.isValid())
        obj = _dagpath.transform(); // use the transform node
    MFnDependencyNode fn(obj);
    return fn.name().asChar();
}

void MVGNodeWrapper::setName(const std::string& name) const
{
    MObject obj = _object;
    if(_dagpath.isValid())
        obj = _dagpath.transform(); // use the transform node
    MFnDependencyNode fn(obj);
    fn.setName(name.c_str());
}

} // namespace
