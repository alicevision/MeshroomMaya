#pragma once

#include <maya/MPxContext.h>
#include <maya/MDagPath.h>
#include "mayaMVG/qt/MVGEventFilter.h"

class MPxManipulatorNode;
class M3dView;

struct EventData {
	EventData()
        : onPressCameraHPan(0)
        , onPressCameraVPan(0)
        , isDragging(false) {}
    MDagPath cameraPath;
    QPoint onPressMousePos;
    double onPressCameraHPan;
    double onPressCameraVPan;
    bool isDragging;
};

namespace mayaMVG {

class MVGEditCmd;

class MVGContext: public MPxContext {
    public:
    enum EditMode {
        eModeCreate = 0
        , eModeMove
        // , eModeDelete
    };
	
	enum EKeyPressed {
		eKeyNone = 0
		, eKeyCtrl
		, eKeyShift
	};
	
    public:
        MVGContext();
        virtual ~MVGContext(){}
        virtual void toolOnSetup(MEvent &event);
        virtual void toolOffCleanup();
        virtual void getClassName(MString & name) const;
    public:
        void updateManipulators();
        bool eventFilter(QObject *obj, QEvent *e);
        MVGEditCmd* newCmd();
    private:
        EventData _eventData;
        MVGEventFilter<MVGContext> _filter;
        MVGEventFilter<MVGContext> _filterLV;
        MVGEventFilter<MVGContext> _filterRV;
        EditMode _editMode;
};

} // namespace
