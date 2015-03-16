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
    enum EditMode
    {
        eModeCreate = 0,
        eModeMove
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

private:
    bool setFocusOnView(QObject* obj);

    EventData _eventData;
    MVGEventFilter<MVGContext> _filter;
    MVGEventFilter<MVGContext> _filterLV;
    MVGEventFilter<MVGContext> _filterRV;
    EditMode _editMode;
    MVGManipulatorCache _manipulatorCache;
};

} // namespace
