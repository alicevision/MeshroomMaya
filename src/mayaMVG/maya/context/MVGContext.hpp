#pragma once

#include "mayaMVG/maya/context/MVGManipulatorCache.hpp"
#include "mayaMVG/qt/MVGEventFilter.hpp"
#include <maya/MPxContext.h>
#include <maya/MDagPath.h>

class MPxManipulatorNode;
class M3dView;

struct EventData
{
    EventData()
        : onPressCameraHPan(0)
        , onPressCameraVPan(0)
        , isDragging(false)
    {
    }
    MDagPath cameraPath;
    QPoint onPressMousePos;
    double onPressCameraHPan;
    double onPressCameraVPan;
    bool isDragging;
};

namespace mayaMVG
{

class MVGEditCmd;

class MVGContext : public MPxContext
{
public:
    enum EEditMode
    {
        eEditModeCreate = 0,
        eEditModeMove = 1,
        eEditModeLocator = 2,
    };

public:
    MVGContext();
    virtual ~MVGContext() {}
    virtual void toolOnSetup(MEvent& event);
    virtual void toolOffCleanup();
    virtual void getClassName(MString& name) const;

public:
    void updateManipulators();
    bool eventFilter(QObject* obj, QEvent* e);
    MVGEditCmd* newCmd();

public:
    MVGManipulatorCache& getCache() { return _manipulatorCache; }
    const EEditMode& getEditMode() const { return _editMode; }
    void setEditMode(EEditMode mode) { _editMode = mode; }

private:
    bool setFocusOnView(QObject* obj);

public:
    static MString _lastMVGManipulator;
    static MString _lastMayaManipulator;

private:
    EventData _eventData;
    MVGEventFilter<MVGContext> _filter;
    MVGEventFilter<MVGContext> _filterLV;
    MVGEventFilter<MVGContext> _filterRV;
    EEditMode _editMode;
    MVGManipulatorCache _manipulatorCache;
};

} // namespace
