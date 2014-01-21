#pragma once

#include <maya/MPxContext.h>
#include <maya/MDagPath.h>

namespace mayaMVG {

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
};

} // mayaMVG
