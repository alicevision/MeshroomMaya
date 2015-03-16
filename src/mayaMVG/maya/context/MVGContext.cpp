#include "mayaMVG/maya/context/MVGContext.hpp"
#include "mayaMVG/maya/context/MVGCreateManipulator.hpp"
#include "mayaMVG/maya/context/MVGMoveManipulator.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/qt/MVGQt.hpp"
#include <maya/MQtUtil.h>
#include <maya/MArgList.h>

namespace mayaMVG
{

MVGContext::MVGContext()
    : _filter((QObject*)MVGMayaUtil::getMVGWindow(), this)
    , _filterLV((QObject*)MVGMayaUtil::getMVGViewportLayout("mvgLPanel"), this)
    , _filterRV((QObject*)MVGMayaUtil::getMVGViewportLayout("mvgRPanel"), this)
    , _editMode(eModeMove)
{
    setTitleString("MVG tool");
}

void MVGContext::toolOnSetup(MEvent& event)
{
    updateManipulators();
    _manipulatorCache.rebuildMeshesCache();
}

void MVGContext::toolOffCleanup()
{
    deleteManipulators();
    MPxContext::toolOffCleanup();
}

void MVGContext::getClassName(MString& name) const
{
    name.set("mayaMVGTool");
}

void MVGContext::updateManipulators()
{
    MString currentContext;
    MVGMayaUtil::getCurrentContext(currentContext);
    if(currentContext != "mayaMVGTool1")
        return;
    // delete all manipulators
    deleteManipulators();
    // then add a new one, depending on edit mode
    MStatus status;
    MObject manipObject;
    switch(_editMode)
    {
        case eModeCreate:
        {
            MVGCreateManipulator* manip = static_cast<MVGCreateManipulator*>(
                MPxManipulatorNode::newManipulator("MVGCreateManipulator", manipObject, &status));
            if(!status || !manip)
                return;
            manip->setContext(this);
            manip->setCache(&_manipulatorCache);
            break;
        }
        case eModeMove:
        {
            MVGMoveManipulator* manip = static_cast<MVGMoveManipulator*>(
                MPxManipulatorNode::newManipulator("MVGMoveManipulator", manipObject, &status));
            if(!status || !manip)
                return;
            manip->setContext(this);
            manip->setCache(&_manipulatorCache);
            break;
        }
        default:
            return;
    }
    if(status)
        addManipulator(manipObject);
}

bool MVGContext::eventFilter(QObject* obj, QEvent* e)
{
    QWidget* widget = qobject_cast<QWidget*>(obj);

    if(!obj)
        return false;

    // key pressed
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyevent = static_cast<QKeyEvent*>(e);
        switch(keyevent->key())
        {
            case Qt::Key_C:
                if(_editMode != eModeCreate)
                {
                    _editMode = eModeCreate;
                    updateManipulators();
                }
                return true;
            case Qt::Key_V:
                if(!MVGCreateManipulator::_doSnap)
                    MVGCreateManipulator::_doSnap = true;
                break; // Spread event to Maya
        }
        // Check for autorepeat
        if(!keyevent->isAutoRepeat())
        {
            switch(keyevent->key())
            {
                case Qt::Key_F:
                {
                    M3dView view = M3dView::active3dView();
                    MDagPath cameraDPath;
                    view.getCamera(cameraDPath);
                    MVGCamera camera(cameraDPath);
                    camera.resetZoomAndPan();
                    return true;
                }
                case Qt::Key_Control:
                    if(_editMode == eModeCreate)
                        break;
                    MVGMoveManipulator::_mode = static_cast<MVGMoveManipulator::MoveMode>(
                        (MVGMoveManipulator::_mode + 1) % 3);
                    _manipulatorCache.clearSelectedComponent();
                    break; // Spread event to Maya
                case Qt::Key_Escape:
                    updateManipulators();
                    _manipulatorCache.clearSelectedComponent();
                    break;
                case Qt::Key_Return:
                case Qt::Key_Enter:
                {
                    const MVGManipulatorCache::MVGComponent& selectedComponent =
                        _manipulatorCache.getSelectedComponent();
                    if(selectedComponent.type != MFn::kMeshVertComponent &&
                       selectedComponent.type != MFn::kBlindData)
                        break;
                    // Clear blind data
                    MVGEditCmd* cmd = newCmd();
                    if(cmd)
                    {
                        MIntArray componentId;
                        componentId.append(selectedComponent.vertex->index);
                        MDagPath meshPath = selectedComponent.meshPath;

                        cmd->clearBD(meshPath, componentId);
                        MArgList args;
                        if(cmd->doIt(args))
                        {
                            cmd->finalize();
                            _manipulatorCache.rebuildMeshesCache();
                            _manipulatorCache.clearSelectedComponent();
                        }
                        break;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    // key released
    else if(e->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyevent = static_cast<QKeyEvent*>(e);
        if(!keyevent->isAutoRepeat())
        {
            switch(keyevent->key())
            {
                case Qt::Key_C:
                    _editMode = eModeMove;
                    updateManipulators();
                    return true;
                case Qt::Key_V:
                    MVGCreateManipulator::_doSnap = false;
                    break; // Spread event to Maya
                case Qt::Key_Escape:
                    updateManipulators();
                    return true;
                default:
                    break;
            }
        }
    }
    // mouse button pressed
    else if(e->type() == QEvent::MouseButtonPress)
    {
        setFocusOnView(obj); // TODO: check how to do it only if needed.

        QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
        // middle button: initialize camera pan
        if((mouseevent->button() & Qt::MidButton))
        {
            if(_eventData.cameraPath.isValid())
            {
                MVGCamera camera(_eventData.cameraPath);
                _eventData.onPressMousePos = mouseevent->pos();
                _eventData.onPressCameraHPan = camera.getHorizontalPan();
                _eventData.onPressCameraVPan = camera.getVerticalPan();
                _eventData.isDragging = true;
                return true;
            }
        }
    }
    // mouse button moved
    else if(e->type() == QEvent::MouseMove)
    {
        if(widget && _eventData.isDragging && _eventData.cameraPath.isValid())
        {
            MVGCamera camera(_eventData.cameraPath);
            // compute pan offset
            QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
            if(!mouseevent)
                return false;
            QPointF offset_screen = _eventData.onPressMousePos - mouseevent->posF();
            const double viewport_width = widget->width();
            QPointF offset = (offset_screen / viewport_width) * camera.getHorizontalFilmAperture() *
                             camera.getZoom();
            camera.setPan(_eventData.onPressCameraHPan + offset.x(),
                          _eventData.onPressCameraVPan - offset.y());
            return true;
        }
    }
    // mouse button released
    else if(e->type() == QEvent::MouseButtonRelease)
    {
        _eventData.isDragging = false;
    }
    // mouse wheel rolled
    else if(e->type() == QEvent::Wheel)
    {
        if(widget && _eventData.cameraPath.isValid())
        {
            // compute & set zoom value
            QMouseEvent* mouseevent = static_cast<QMouseEvent*>(e);
            QWheelEvent* wheelevent = static_cast<QWheelEvent*>(e);
            MVGCamera camera(_eventData.cameraPath);
            const double viewportWidth = widget->width();
            const double viewportHeight = widget->height();
            static const double wheelStep = 1.15;
            const double previousZoom = camera.getZoom();
            double newZoom =
                wheelevent->delta() > 0 ? previousZoom / wheelStep : previousZoom * wheelStep;
            camera.setZoom(std::max(newZoom, 0.0001));
            const double scaleRatio = newZoom / previousZoom;
            // compute & set pan offset
            QPointF center_ratio(0.5, 0.5 * viewportHeight / viewportWidth);
            QPointF mouse_ratio_center = (center_ratio - (mouseevent->posF() / viewportWidth));
            QPointF mouse_maya_center =
                mouse_ratio_center * camera.getHorizontalFilmAperture() * previousZoom;
            QPointF mouseAfterZoo_maya_center = mouse_maya_center * scaleRatio;
            QPointF offset = mouse_maya_center - mouseAfterZoo_maya_center;
            camera.setPan(camera.getHorizontalPan() - offset.x(),
                          camera.getVerticalPan() + offset.y());
            return true;
        }
    }
    else if(e->type() == QEvent::Leave)
    {
        _eventData.cameraPath = MDagPath();
    }
    // mouse enters widget's boundaries
    else if(e->type() == QEvent::Enter)
    {
        if(widget && !widget->isActiveWindow())
            return false;

        return setFocusOnView(obj);
    }
    return false;
}

bool MVGContext::setFocusOnView(QObject* obj)
{
    // check if we are entering an MVG panel
    QVariant panelName = obj->property("mvg_panel");
    if(!panelName.isValid())
        return false;

    // find & register the associated camera path
    MVGMayaUtil::getCameraInView(_eventData.cameraPath, MQtUtil::toMString(panelName.toString()));
    if(!_eventData.cameraPath.isValid())
        return false;

    // automatically set focus on this MVG panel
    MVGMayaUtil::setFocusOnView(MQtUtil::toMString(panelName.toString()));
    _manipulatorCache.setActiveView(M3dView::active3dView());
    return true;
}

MVGEditCmd* MVGContext::newCmd()
{
    return (MVGEditCmd*)newToolCommand();
}

} // namespace
