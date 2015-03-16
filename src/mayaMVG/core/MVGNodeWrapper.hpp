#pragma once

#include <maya/MDagPath.h>
#include <string>

namespace mayaMVG
{

class MVGNodeWrapper
{

public:
    MVGNodeWrapper();
    MVGNodeWrapper(const std::string& name);
    MVGNodeWrapper(const MString& name);
    MVGNodeWrapper(const MDagPath& dagPath);
    MVGNodeWrapper(const MObject& object);
    virtual ~MVGNodeWrapper() {}

public:
    virtual bool isValid() const = 0;
    void selectNode() const;

public:
    const MDagPath& getDagPath() const;
    const MObject& getObject() const;
    std::string getName() const;
    void setName(const std::string&) const;

protected:
    MDagPath _dagpath;
    MObject _object;
};

} // namespace
