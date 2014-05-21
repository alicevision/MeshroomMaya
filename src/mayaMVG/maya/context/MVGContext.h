#pragma once

#include <vector>
#include <maya/MPxSelectionContext.h>

namespace mayaMVG {

// forward declaration
class MVGContextEventFilter;

class MVGContext: public MPxSelectionContext {

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

    private:
        MVGContextEventFilter* m_eventFilter;
        
};

} // mayaMVG
