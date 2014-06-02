#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGProject.h"
#include <maya/MFnCamera.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshEdge.h>
#include <maya/MSelectionList.h>
#include <vector>

namespace mayaMVG {
	
#define EDGE_TOLERANCE 1.0e-6

MTypeId MVGBuildFaceManipulator::_id(0x99999); // FIXME /!\ 

MDagPath MVGBuildFaceManipulator::_lastCameraPath = MDagPath();
MVGCamera MVGBuildFaceManipulator::_camera = MVGCamera(_lastCameraPath);
std::vector<MPoint> MVGBuildFaceManipulator::_display2DPoints_world = std::vector<MPoint>();

namespace {
	double crossProduct2d(MVector& A, MVector& B) {
		return A.x*B.y - A.y*B.x;
	}
	
	double dotProduct2d(MVector& A, MVector& B) {
		return A.x*B.x - A.y*B.y;
	}

	bool edgesIntersection(MPoint A, MPoint B, MVector AD,  MVector BC)
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
	
	// Points must be in 2D
	bool arePointsAligned2d(MPoint& P, MPoint& A, MPoint& B) 
	{
		MVector AB = B - A;
		MVector AP = P - A;
		
		double cross = crossProduct2d(AB, AP);
		
		if(cross > EDGE_TOLERANCE
			|| cross < -EDGE_TOLERANCE) 
		{
			return false;
		}
		
		return true;
	}
	
	bool isPointOnEdge(MPoint& P, MPoint& A, MPoint& B)
	{	
		MVector PA = A - P;
		MVector PB = B - P;
		// Points aligned
		if(!arePointsAligned2d(P, A, B))
			return false;
		
//		MVector AP = P - A;
//		MVector AB = B - A;
//		double r = dotProduct2d(AB, AP) / pow(AB.length(), 2);
//		LOG_INFO("r = " << r);
//		if( r < - || r > 1)
//			return false;
		
		double scalar = dotProduct2d(PA, PB);
		if(scalar > kMFnMeshTolerance)
			return false;
		
		return true;
	}
	
	void drawDisk(int x, int y, int r, int segments)
	{
		glBegin(GL_TRIANGLE_FAN );
			glVertex2f(x, y);
			for( int n = 0; n <= segments; ++n ) {
				float const t = 2*M_PI*(float)n/(float)segments;
				glVertex2f(x + sin(t)*r, y + cos(t)*r);
			}
		glEnd();
	}
}

MVGBuildFaceManipulator::MVGBuildFaceManipulator()
{
	_doIntersectExistingPoint = false;
	_doIntersectExistingEdge = false;
	_onEdgeExtension = false;
	_cameraPathClickedPoints = MDagPath();
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
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
			
			// Intersection with point
			if(_doIntersectExistingPoint)
			{
				short x;
				short y;								
				view.worldToView(_mousePoint, x, y);
				
				glColor4f(0.f, 1.f, 0.f, 0.6f);
				drawDisk(x, y, 10, 10);
			}
			
			// Intersection with edge
			if(_doIntersectExistingEdge)
			{
				short x, y;
				glColor4f(0.f, 1.f, 0.f, 0.6f);
				glBegin(GL_LINES);
					view.worldToView(_intersectingEdgePoints3D[0], x, y);
					glVertex2f(x, y);
					view.worldToView(_intersectingEdgePoints3D[1], x, y);
					glVertex2f(x, y);
				glEnd();
			}
			
			MDagPath cameraPath;
			view.getCamera(cameraPath);

			if(cameraPath == _lastCameraPath)
			{												
				// Draw lines and poly
				if(!_display2DPoints_world.empty())
				{
					short x;
					short y;
					glColor4f(1.f, 0.f, 0.f, 0.6f);

					if(!_onEdgeExtension 
						&& _display2DPoints_world.size() < 3 
						&& (_cameraPathClickedPoints == _lastCameraPath))
					{
						// Lines
						if(_display2DPoints_world.size() > 1)
						{
							glBegin(GL_LINES);
							view.worldToView(_display2DPoints_world[0], x, y);
							glVertex2f(x, y);
							view.worldToView(_display2DPoints_world[1], x, y);
							glVertex2f(x, y);
							glEnd();	

							glPointSize(4.f);
							glBegin(GL_POINTS);
							for(size_t i = 0; i < _display2DPoints_world.size(); ++i){
								view.worldToView(_display2DPoints_world[i], x, y);
								glVertex2f(x, y);
							}
							glEnd();					
						}	

						if(_display2DPoints_world.size() > 0)
						{
							glBegin(GL_LINES);
							view.worldToView(_display2DPoints_world[_display2DPoints_world.size() - 1], x, y);
							glVertex2f(x, y);
							view.worldToView(_mousePoint, x, y);
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
	}
	glPopAttrib();
	view.endGL();
}

MStatus MVGBuildFaceManipulator::doPress(M3dView& view)
{	
	MPoint wpos;
	MVector wdir;	
	short mousex, mousey;
	mousePosition(mousex, mousey);
		
	// Construction by edge : store pressedPoint
	if(_doIntersectExistingEdge)
	{
		view.viewToWorld(mousex, mousey, _mousePointOnPressEdge, wdir);	
		_clickedEdgePoints3D.clear();
		_clickedEdgePoints3D.append(_intersectingEdgePoints3D[0]);
		_clickedEdgePoints3D.append(_intersectingEdgePoints3D[1]);
		_onEdgeExtension = true;
	}
	
	if(!_onEdgeExtension)
	{
		if(!(_cameraPathClickedPoints == _lastCameraPath))
		{
			_display2DPoints_world.clear();
			_cameraPathClickedPoints = _lastCameraPath;
		}

		// Add a new point		
		view.viewToWorld(mousex, mousey, wpos, wdir);	
		//_camera.addClickedPoint(wpos);
		_display2DPoints_world.push_back(wpos);

		// Create face3D
		if(_display2DPoints_world.size() > 3)
		{
			addFace3d(_preview3DFace);
			MVGMayaUtil::deletePreviewShape();
			_display2DPoints_world.clear();
			//_camera.clearClickedPoints();
		}

	}

	return MPxManipulatorNode::doPress(view);
}

MStatus MVGBuildFaceManipulator::doRelease(M3dView& view)
{
	if(_onEdgeExtension)
	{
		addFace3d(_preview3DFace);
		MVGMayaUtil::deletePreviewShape();
		_onEdgeExtension = false;
	}
	
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGBuildFaceManipulator::doMove(M3dView& view, bool& refresh)
{
	MDagPath cameraPath;
	view.getCamera(cameraPath);

	if(!(cameraPath == _lastCameraPath))
	{
		_lastCameraPath = cameraPath;
		_camera = getMVGCamera();
		
	}
	
	if(!_onEdgeExtension && !_display2DPoints_world.empty() && !(cameraPath == _cameraPathClickedPoints))
	{
		LOG_WARNING("Change of view while creating a face : if you click a point, it will erase previous points. ");
		if(_display2DPoints_world.size() > 2)
		{
			MVGMesh mesh(MVGProject::_PREVIEW_MESH);
			if(mesh.isValid())
				MVGMayaUtil::deletePreviewShape();
		}
			
	}
	
	refresh = true;
	
	// Update mousePoint
	MVector wdir;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	view.viewToWorld(mousex, mousey, _mousePoint, wdir);

	// Check for existing points
	MVGMesh mesh(MVGProject::_MESH);
	if(!mesh.isValid()) {
		mesh = MVGMesh::create(MVGProject::_MESH);
		LOG_INFO("New OpenMVG Mesh.")
	}
		
	// Preview
	if(_display2DPoints_world.size() > 2
		&& (_lastCameraPath == _cameraPathClickedPoints))
	{		
		std::vector<MPoint> previewPoints2d;
			
		previewPoints2d.clear();
		previewPoints2d.push_back(_display2DPoints_world[0]);
		previewPoints2d.push_back(_display2DPoints_world[1]);
		previewPoints2d.push_back(_display2DPoints_world[2]);
		previewPoints2d.push_back(_mousePoint);
		
		if(computeFace3d(view, previewPoints2d, _preview3DFace))
			previewFace3d(_preview3DFace);
	}
	
	// Intersections
	MPointArray meshPoints;
	mesh.getPoints(meshPoints);
		
	if(meshPoints.length() > 0)
	{
		// Point intersection
		_doIntersectExistingPoint = false;
		for(int i = 0; i < meshPoints.length(); ++i)
		{
			short x, y;
			view.worldToView(meshPoints[i], x, y);

			if(mousex <= x + kMFnMeshPointTolerance
				&& mousex >= x - kMFnMeshPointTolerance
				&& mousey <= y + kMFnMeshPointTolerance
				&& mousey >= y - kMFnMeshPointTolerance)
			{
				_doIntersectExistingPoint = true;
				break;
			}
		}
		
		// Edge intersection
		MPointArray points;
		short x0, y0, x1, y1;
		_doIntersectExistingEdge = false;
		if(mesh.intersect(_mousePoint, wdir, points))
		{
			
			MItMeshEdge edgeIt(mesh.dagPath());
			double minLenght = -1;
			double lenght;
			MPointArray edgePoints;
			while(!edgeIt.isDone())
			{
				view.worldToView(edgeIt.point(0), x0, y0);
				view.worldToView(edgeIt.point(1), x1, y1);
				
				MVector wdir;
				MPoint A;
				view.viewToWorld(x0, y0, A, wdir);
				MPoint B;
				view.viewToWorld(x1, y1, B, wdir);
								
				if(isPointOnEdge(_mousePoint, A, B))
				{
					_doIntersectExistingEdge = true;

					lenght = A.distanceTo(B);
					if(minLenght < 0)
					{
						minLenght = lenght;
						edgePoints.clear();
						edgePoints.append(edgeIt.point(0));
						edgePoints.append(edgeIt.point(1));	
					}
				}
				edgeIt.next();
			}
			if(_doIntersectExistingEdge)
			{
				_intersectingEdgePoints3D = edgePoints;
			}
		}
	}
	
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGBuildFaceManipulator::doDrag(M3dView& view)
{
	// Update mousePoint
	MVector wdir;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	view.viewToWorld(mousex, mousey, _mousePoint, wdir);
	
	if(_onEdgeExtension)
	{
		view.viewToWorld(mousex, mousey, _mousePointOnDragEdge, wdir);	
		
		std::vector<MPoint> previewPoints2d;
			
		short x0, y0;
		short x1, y1;
		MPoint edge0, edge1;
		MVector wdir;
		view.worldToView(_clickedEdgePoints3D[1], x0, y0);
		view.viewToWorld(x0, y0, edge0, wdir);
		previewPoints2d.push_back(edge0);
		view.worldToView(_clickedEdgePoints3D[0], x1, y1);
		view.viewToWorld(x1, y1, edge1, wdir);
		previewPoints2d.push_back(edge1);
		MPoint P3 = _mousePointOnDragEdge + (edge0 - _mousePointOnPressEdge);
		MPoint P4 = _mousePointOnDragEdge + (edge1 - _mousePointOnPressEdge);
		previewPoints2d.push_back(P3);
		previewPoints2d.push_back(P4);

		if(computeFace3d(view, previewPoints2d, _preview3DFace, true))
			previewFace3d(_preview3DFace);
	}
	
	return MPxManipulatorNode::doDrag(view);
}

void MVGBuildFaceManipulator::MVGBuildFaceManipulator::preDrawUI(const M3dView& view)
{
	_drawEnabled = MVGMayaUtil::isMVGView(view);
}

void MVGBuildFaceManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	if(!_drawEnabled)
		return;
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	for(size_t i = 1; i < _camera.getClickedPointsCount(); ++i)
		drawManager.line2d(_camera.geClickedtPointAtIndex(i-1), _camera.geClickedtPointAtIndex(i));
	drawManager.endDrawable();
}

MVGCamera MVGBuildFaceManipulator::getMVGCamera() const
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

bool MVGBuildFaceManipulator::computeFace3d(M3dView& view, std::vector<MPoint>& pointArray, MVGFace3D& face3D, bool computeLastPoint)
{
	// Check points order	
	MVector AD = pointArray[3] - pointArray[0];
	MVector BC = pointArray[2] - pointArray[1];
			
	if(edgesIntersection(pointArray[0],pointArray[1], AD, BC))
	{
		std::vector<MPoint>	tmp(pointArray);
		pointArray[3] = tmp[2];
		pointArray[2] = tmp[3];
	}
	
	MVGFace2D face2D(pointArray);
	
	return MVGGeometryUtil::projectFace2D(face3D, view, _camera, face2D, computeLastPoint);
	
}
void MVGBuildFaceManipulator::previewFace3d(MVGFace3D& face3d)
{
	MVGMesh previewMesh(MVGProject::_PREVIEW_MESH);
	if(!previewMesh.isValid()) {
		previewMesh = MVGMesh::create(MVGProject::_PREVIEW_MESH);
		LOG_INFO("New Preview Mesh.");
	}
	
	if(_onEdgeExtension )
		{				
			MVGMesh mesh(MVGProject::_MESH);
			if(!mesh.isValid()) {
				mesh = MVGMesh::create(MVGProject::_MESH);
				LOG_INFO("New OpenMVG Mesh.")
			}
			MPointArray meshPoints;		
			mesh.getPoints(meshPoints);
						
			face3d._p[0] = _clickedEdgePoints3D[1];
			face3d._p[1] = _clickedEdgePoints3D[0];	
					
			previewMesh.addPolygon(face3d);
			
		}
		else {
			previewMesh.addPolygon(face3d);
		}
		
		previewMesh.deleteFace(0);
}
void MVGBuildFaceManipulator::addFace3d(MVGFace3D& face3d)
{
	MVGMesh mesh(MVGProject::_MESH);
	if(!mesh.isValid()) {
		mesh = MVGMesh::create(MVGProject::_MESH);
		LOG_INFO("New OpenMVG Mesh.")
	}
	
	mesh.addPolygon(face3d);

}
}	// namespace mayaMVG 

