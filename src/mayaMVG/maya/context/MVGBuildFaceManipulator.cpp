#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGProject.h"
#include <maya/MFnCamera.h>

using namespace mayaMVG;

namespace {
	double crossProduct2d(MVector& A, MVector& B) {
		return A[0]*B[1] - A[1]*B[0];
	}

	bool segmentIntersection(MPoint A, MPoint B, MVector AD,  MVector BC)
	{		
		// r x s = 0
		double cross = crossProduct2d(AD, BC);
		double eps = 0.00001;
		if(cross < eps && cross > -eps)
			return false;

		MVector AB = B - A;

		double x =  crossProduct2d(AB, BC) / crossProduct2d(AD, BC);
		double y = crossProduct2d(AB, AD) / crossProduct2d(AD, BC);

		if( x >= 0 
			&& x <= 1 
			&& y >= 0 
			&& y <= 1)
		{
			return true;
		}

		return false;
	}
}

bool MVGBuildFaceManipulator::_connectFace(true);
bool MVGBuildFaceManipulator::_computeLastPoint(true);
MTypeId MVGBuildFaceManipulator::_id(0x99999); // FIXME /!\ 

MVGBuildFaceManipulator::MVGBuildFaceManipulator()
{
//	MVGBuildFaceManipulator::_connectFace = true;
//	MVGBuildFaceManipulator::_computeLastPoint = true;
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

			if(!_wpoints.empty()) 
			{
				MDagPath cameraPath;
				view.getCamera(cameraPath);
				if(cameraPath == _lastCameraPath)
				{
					short x;
					short y;
						
					if(_wpoints.size() > 2)
					{
						glBegin(GL_POLYGON);
						for(size_t i = 0; i < _wpoints.size(); ++i){
							view.worldToView(_wpoints[i], x, y);
							glVertex2f(x, y);
						}
						glEnd();
					}			
					else if(_wpoints.size() > 1)
					{
						glBegin(GL_LINES);
						view.worldToView(_wpoints[0], x, y);
						glVertex2f(x, y);
						view.worldToView(_wpoints[1], x, y);
						glVertex2f(x, y);
						glEnd();	
						
						glPointSize(4.f);
						glBegin(GL_POINTS);
						for(size_t i = 0; i < _wpoints.size(); ++i){
							view.worldToView(_wpoints[i], x, y);
							glVertex2f(x, y);
						}					
						glEnd();
						
						// Preview of quad
						if(MVGBuildFaceManipulator::_computeLastPoint)
						{										
							MVector height = _wpoints[0] - _wpoints[1];
							_lastPoint = _mousePoint + height;

							glColor4f(1.f, 1.f, 1.f, 0.6f);
							glBegin(GL_POLYGON);
								view.worldToView(_wpoints[0], x, y);
								glVertex2f(x, y);
								view.worldToView(_wpoints[1], x, y);
								glVertex2f(x, y);
								view.worldToView(_mousePoint, x, y);
								glVertex2f(x, y);
								view.worldToView(_lastPoint, x, y);
								glVertex2f(x, y);
							glEnd();
						}		
					}					
					
				}
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

	// View changed : clear all points & register the new camera path
	if((_lastCameraPath.length() > 0) && !(cameraPath == _lastCameraPath))
		_wpoints.clear();
	_lastCameraPath = cameraPath;
	
	// Add a new point
	MPoint wpos;
	MVector wdir;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	view.viewToWorld(mousex, mousey, wpos, wdir);
	_wpoints.push_back(wpos);
	
	// TODO
	// check if this point intersect an existing Point2D
	
	// Find fourth point
	if(MVGBuildFaceManipulator::_computeLastPoint && _wpoints.size() > 2)
	{		
		_wpoints.push_back(_lastPoint);
	}
	
	// Create face3D
	if(_wpoints.size() > 3)
	{
		createFace3d(view, _wpoints);
		
		// Keep the last two points to connect the next face
		if(MVGBuildFaceManipulator::_connectFace) {
			std::vector<MPoint> tmp(_wpoints);
			_wpoints.clear();
			_wpoints.push_back(tmp[2]);
			_wpoints.push_back(tmp[3]);
		}
		else
		{
			_wpoints.clear();
		}	
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
	
	// Update mousePoint
	MVector wdir;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	view.viewToWorld(mousex, mousey, _mousePoint, wdir);

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
	for(size_t i = 1; i < _wpoints.size(); ++i)
		drawManager.line2d(_wpoints[i-1], _wpoints[i]);
	drawManager.endDrawable();
}

MVGCamera MVGBuildFaceManipulator::getMVGCamera()
{
	return MVGCamera(_lastCameraPath.partialPathName().asChar());
}

MVGCamera MVGBuildFaceManipulator::getMVGCamera(M3dView& view)
{
	MDagPath cameraPath;
	view.getCamera(cameraPath);
	// cameraPath.pop();
	return MVGCamera(cameraPath.partialPathName().asChar());
}

void MVGBuildFaceManipulator::createFace3d(M3dView& view, std::vector<MPoint> facePoints)
{
	// Check points order
	MVector AD = _wpoints[3] - _wpoints[0];
	MVector BC = _wpoints[2] - _wpoints[1];
		
	if(segmentIntersection(_wpoints[0], _wpoints[1], AD, BC))
	{
		std::vector<MPoint>	tmp(_wpoints);
		_wpoints[3] = tmp[2];
		_wpoints[2] = tmp[3];
	}
	
	MVGFace3D face3D;
	MVGFace2D face2D(_wpoints);
		
	MVGMesh mesh(MVGProject::_MESH);
	if(!mesh.isValid()) {
		mesh = MVGMesh::create(MVGProject::_MESH);
		LOG_INFO("New OpenMVG Mesh.")
	}
	MVGPointCloud pointCloud(MVGProject::_CLOUD);
	if(!pointCloud.isValid()) {
		pointCloud = MVGPointCloud::create(MVGProject::_CLOUD);
		LOG_INFO("New OpenMVG Point Cloud.")
	}
	MVGCamera camera = getMVGCamera();
	
	if(MVGGeometryUtil::projectFace2D(face3D, pointCloud, view, camera, face2D))
	{
		mesh.addPolygon(face3D);
	}
}
