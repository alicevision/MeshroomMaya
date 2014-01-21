#include "MVGManipContainer.h"
#include "MVGContext.h"
#include "util/MVGLog.h"
#include <maya/MFnDagNode.h>

using namespace mayaMVG;

MTypeId MVGManipContainer::id(0x99999);
MString MVGManipContainer::drawDbClassification("drawdb/geometry/MVGManip");
MString MVGManipContainer::drawRegistrantId("MVGManipNode");

MVGManipContainer::MVGManipContainer() :
	m_ctx(NULL)
{
}

MVGManipContainer::~MVGManipContainer()
{
}

void * MVGManipContainer::creator()
{
	return new MVGManipContainer();
}

MStatus MVGManipContainer::initialize()
{
	return MPxManipContainer::initialize();
}

MStatus MVGManipContainer::createChildren()
{
	return MStatus::kSuccess;
}

void MVGManipContainer::draw(M3dView & view, const MDagPath & path,
                               M3dView::DisplayStyle style, M3dView::DisplayStatus dispStatus)
{
	// draw children
	MPxManipContainer::draw(view, path, style, dispStatus);

	// get infos from context
	if(!m_ctx)
		return;
	
	view.beginGL();
	glPushAttrib(GL_ALL_ATTRIB_BITS);
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

void MVGManipContainer::setContext(MVGContext * ctx) {
	m_ctx = ctx;
}
