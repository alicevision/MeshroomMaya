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
        struct SelectedPoint {
            double x;
            double y;
            size_t index;
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
        void createMesh();
        void addPointToAttribute( const MString& attributeName,
                                  const size_t vertexIndex,
                                  const MPoint& point );
        void createCameraAttribute( const MString& nameAttribute );
        size_t m_mousePosX;
        size_t m_mousePosY;
        vpoint_t m_points;
        MVGContextEventFilter* m_eventFilter;
        SelectedPoint selectedPoint;
    public:
        MDagPath m_meshPath;
        
};

} // mayaMVG
