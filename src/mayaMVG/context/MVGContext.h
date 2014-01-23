#pragma once

#include <maya/MPxContext.h>
#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <vector>

namespace mayaMVG {

// forward declaration
class MVGContextEventFilter;

class MVGContext: public MPxContext {
    public:
        struct MVGPoint {
            MPoint wpos;
            MPoint spos;
            MVector wdir;
        };
        typedef std::vector<MVGPoint> vpoint_t;
        typedef std::vector<MVGPoint>::const_iterator vpointIt_t;
    public:
        MVGContext();
        virtual ~MVGContext(){}
        virtual void toolOnSetup(MEvent &event);
        virtual void toolOffCleanup();
        virtual MStatus doPress(MEvent & e);
        virtual MStatus doRelease(MEvent & e);
        virtual MStatus doDrag(MEvent & e);
        virtual void getClassName(MString & name) const;
    public:
        static void updateManipulators(void* clientData);
    public:
        void setMousePos(size_t x, size_t y);
        size_t mousePosX() const;
        size_t mousePosY() const;
        vpoint_t points() const;
    private:
    	size_t m_mousePosX;
    	size_t m_mousePosY;
        vpoint_t m_points;
        MObject m_mesh;
        MVGContextEventFilter* m_eventFilter;
};

} // mayaMVG
