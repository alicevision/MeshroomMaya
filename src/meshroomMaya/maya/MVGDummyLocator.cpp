#include "MVGDummyLocator.h"

#include <maya/MFnDependencyNode.h>

MTypeId MVGDummyLocator::_id(0xaf26c); // FIXME

MVGDummyLocator::MVGDummyLocator()
{
}

MVGDummyLocator::~MVGDummyLocator()
{
}

MStatus MVGDummyLocator::initialize()
{
    return MS::kSuccess;
}

void* MVGDummyLocator::creator()
{
    return new MVGDummyLocator();
}

void MVGDummyLocator::postConstructor()
{
    MFnDependencyNode nodeFn(thisMObject());
    nodeFn.setName("MVGDummyLocatorShape#");
}

void MVGDummyLocator::draw(M3dView& view, const MDagPath& path, M3dView::DisplayStyle style,
                           M3dView::DisplayStatus displayStatus)
{

    MStatus status;

    view.beginGL();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // main gizmo
    glBegin(GL_LINES);
    glColor3f(1.f, 0.f, 0.f);
    glVertex3f(0.0, 0.0, 0.0); // X
    glVertex3f(1.0, 0.0, 0.0);
    glColor3f(0.f, 1.f, 0.f);
    glVertex3f(0.0, 0.0, 0.0); // Y
    glVertex3f(0.0, 1.0, 0.0);
    glColor3f(0.f, 0.f, 1.f);
    glVertex3f(0.0, 0.0, 0.0); // Z
    glVertex3f(0.0, 0.0, 1.0);
    glEnd();

    glPopAttrib();
    view.endGL();
}
