#pragma once

#include <maya/MPxContext.h>
#include <maya/MDagPath.h>
#include <maya/MPoint.h>

namespace mayaMVG {

// forward declaration
class MVGContextEventFilter;

class MVGContext: public MPxContext {
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
    private:
    	size_t m_mousePosX;
    	size_t m_mousePosY;
        MVGContextEventFilter* m_eventFilter;
};

} // mayaMVG
