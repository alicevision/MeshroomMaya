#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"

using namespace mayaMVG;

MTypeId MVGBuildFaceManipulator::_id(0x99999);

MVGBuildFaceManipulator::MVGBuildFaceManipulator()
{
}

MVGBuildFaceManipulator::~MVGBuildFaceManipulator()
{
}

void * MVGBuildFaceManipulator::creator()
{
	return new MVGBuildFaceManipulator();
}

MStatus MVGBuildFaceManipulator::initialize()
{
	return MS::kSuccess;
}

void MVGBuildFaceManipulator::postConstructor()
{
	registerForMouseMove();
}

void MVGBuildFaceManipulator::draw(M3dView & view, const MDagPath & path,
                               M3dView::DisplayStyle style, M3dView::DisplayStatus dispStatus)
{
	if(!MVGMayaUtil::isMVGView(view) || !MVGMayaUtil::isActiveView(view))
		return;

	short mousex, mousey;
	mousePosition(mousex, mousey);
	GLdouble radius = 3.0;

	view.beginGL();
	
	// needed to enable doPress, doRelease
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);
	colorAndName(view, glPickableItem, true, mainColor());

	// drawing part
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	{
		// draw in screen space
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			glOrtho(0, view.portWidth(), 0, view.portHeight(), -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glColor3f(1.f, 0.f, 0.f);

			// draw GL cursor
			glBegin(GL_LINES);
			glVertex2f((GLfloat)(mousex + (cos(M_PI / 4.0f) * (radius + 10.0f))),
			           (GLfloat)(mousey + (sin(M_PI / 4.0f) * (radius + 10.0f))));
			glVertex2f((GLfloat)(mousex + (cos(-3.0f * M_PI / 4.0f) * (radius + 10.0f))),
			           (GLfloat)(mousey + (sin(-3.0f * M_PI / 4.0f) * (radius + 10.0f))));
			glVertex2f((GLfloat)(mousex + (cos(3.0f * M_PI / 4.0f) * (radius + 10.0f))),
			           (GLfloat)(mousey + (sin(3.0f * M_PI / 4.0f) * (radius + 10.0f))));
			glVertex2f((GLfloat)(mousex + (cos(-M_PI / 4.0f) * (radius + 10.0f))),
			           (GLfloat)(mousey + (sin(-M_PI / 4.0f) * (radius + 10.0f))));
			glEnd();

			// draw GL lines between clicked points
			glBegin(GL_LINES);
			for(size_t i = 1; i < _points.size(); ++i)
			{
				glVertex2f(_points[i-1].x, _points[i-1].y);
				glVertex2f(_points[i].x, _points[i].y);
			}
			glEnd();

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
		}
		glPopMatrix();
	}
	glPopAttrib();
	view.endGL();
}

MStatus MVGBuildFaceManipulator::doPress(M3dView& view)
{
	short mousex, mousey;
	mousePosition(mousex, mousey);
	_points.push_back(MPoint(mousex, mousey));
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGBuildFaceManipulator::doRelease(M3dView& view)
{
	short mousex, mousey;
	mousePosition(mousex, mousey);
	_points.push_back(MPoint(mousex, mousey));
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGBuildFaceManipulator::doMove(M3dView& view, bool& refresh)
{
	refresh = true;
	return MPxManipulatorNode::doMove(view, refresh);
}

void MVGBuildFaceManipulator::preDrawUI(const M3dView&)
{

}

void MVGBuildFaceManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	for(size_t i = 1; i < _points.size(); ++i)
		drawManager.line2d(_points[i-1], _points[i]);
	drawManager.endDrawable();
}
