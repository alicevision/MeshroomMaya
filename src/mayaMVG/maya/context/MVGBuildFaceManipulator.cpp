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
	_drawEnabled = MVGMayaUtil::isMVGView(view);
	if(!_drawEnabled)
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
		// glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(0);

		// draw in screen space
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			glOrtho(0, view.portWidth(), 0, view.portHeight(), -1, 1);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glColor4f(1.f, 0.f, 0.f, 0.6f);

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
			MDagPath cameraPath;
			view.getCamera(cameraPath);
			if(cameraPath == _lastCameraPath)
			{
				if(_points.size() > 2)
				{
					glBegin(GL_POLYGON);
					for(size_t i = 0; i < _points.size(); ++i)
						glVertex2f(_points[i].x, _points[i].y);
					glEnd();
				}
				else if(_points.size() > 1) {
					glBegin(GL_LINES);
					glVertex2f(_points[0].x, _points[0].y);
					glVertex2f(_points[1].x, _points[1].y);
					glEnd();
				}
				glPointSize(4.f);
				glBegin(GL_POINTS);
				for(size_t i = 0; i < _points.size(); ++i)
					glVertex2f(_points[i].x, _points[i].y);
				glEnd();
			}

			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
		}
		glPopMatrix();
		
		glDisable(GL_BLEND);
		glDepthMask(1);
	}
	glPopAttrib();
	view.endGL();
}




MStatus MVGBuildFaceManipulator::doPress(M3dView& view)
{
	MDagPath cameraPath;
	view.getCamera(cameraPath);

	// view changed - clear all points
	if((_lastCameraPath.length() > 0) && !(cameraPath == _lastCameraPath))
	{
		_points.clear();
	}
	_lastCameraPath = cameraPath;

	// prepare for new 2D face
	if(_points.size() > 3)
	{
		MPoint first, second;
		first = _points[3];
		second = _points[2];
		_points.clear();
		_points.push_back(first);
		_points.push_back(second);
	}

	// add a new point
	short mousex, mousey;
	mousePosition(mousex, mousey);
	_points.push_back(MPoint(mousex, mousey));

	// build the face
	if(_points.size() > 2)
	{
		MPoint barycenter = (_points[_points.size()-3] + _points[_points.size()-1]) / 2.0;
		MPoint p = _points[_points.size()-2] + (2 * ( barycenter - _points[_points.size()-2]));
		_points.push_back(p);
	}
	
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGBuildFaceManipulator::doRelease(M3dView& view)
{
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGBuildFaceManipulator::doMove(M3dView& view, bool& refresh)
{
	refresh = true;
	return MPxManipulatorNode::doMove(view, refresh);
}

void MVGBuildFaceManipulator::preDrawUI(const M3dView& view)
{
	_drawEnabled = MVGMayaUtil::isMVGView(view);
}

void MVGBuildFaceManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	if(!_drawEnabled)
		return;
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	for(size_t i = 1; i < _points.size(); ++i)
		drawManager.line2d(_points[i-1], _points[i]);
	drawManager.endDrawable();
}
