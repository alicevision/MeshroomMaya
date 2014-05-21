#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
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

bool MVGBuildFaceManipulator::_connectFace(true);
bool MVGBuildFaceManipulator::_computeLastPoint(true);
MDagPath MVGBuildFaceManipulator::_lastCameraPath = MDagPath();

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
	bool arePointsAligned2d(MPoint P, MPoint A, MPoint B) 
	{
		MVector AB = B - A;
		MVector AP = P - A;
		
		double cross = crossProduct2d(AB, AP);
		
		// Cross not null
		if(cross > EDGE_TOLERANCE
			|| cross < -EDGE_TOLERANCE) 
		{
			return false;
		}
				
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
					view.worldToView(_intersectingEdgePoints[0], x, y);
					glVertex2f(x, y);
					view.worldToView(_intersectingEdgePoints[1], x, y);
					glVertex2f(x, y);
				glEnd();
			}
			
			MDagPath cameraPath;
			view.getCamera(cameraPath);

			if(cameraPath == _lastCameraPath)
			{
				if(!getCameraPoints().empty())
				{
					short x;
					short y;
					glColor4f(1.f, 0.f, 0.f, 0.6f);

					if(getCameraPointsCount() > 2)
					{
						glBegin(GL_POLYGON);
						for(size_t i = 0; i < getCameraPointsCount(); ++i){
							view.worldToView(getCameraPointAtIndex(i), x, y);
							glVertex2f(x, y);
						}
						glEnd();
					}			
					else if(getCameraPointsCount() > 1)
					{
						glBegin(GL_LINES);
						view.worldToView(getCameraPointAtIndex(0), x, y);
						glVertex2f(x, y);
						view.worldToView(getCameraPointAtIndex(1), x, y);
						glVertex2f(x, y);
						glEnd();	
						
						glPointSize(4.f);
						glBegin(GL_POINTS);
						for(size_t i = 0; i < getCameraPointsCount(); ++i){
							view.worldToView(getCameraPointAtIndex(i), x, y);
							glVertex2f(x, y);
						}
						glEnd();
						
						// Preview of quad
						if(MVGBuildFaceManipulator::_computeLastPoint)
						{										
							MVector height = getCameraPointAtIndex(0) -getCameraPointAtIndex(1);
							_lastPoint = _mousePoint + height;

							glColor4f(1.f, 1.f, 1.f, 0.6f);
							glBegin(GL_POLYGON);
								view.worldToView(getCameraPointAtIndex(0), x, y);
								glVertex2f(x, y);
								view.worldToView(getCameraPointAtIndex(1), x, y);
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
		clearCameraPoints();
	_lastCameraPath = cameraPath;
	
	// Set _isShapeFinished when first point is clicked
	if(getCameraPoints().empty())
		setIsShapeFinished(false);
	
	// Add a new point
	MPoint wpos;
	MVector wdir;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	view.viewToWorld(mousex, mousey, wpos, wdir);
	addPointToCamera(wpos);
		

	// TODO
	// check if this point intersect an existing Point2D
	
	// Find fourth point
	if(MVGBuildFaceManipulator::_computeLastPoint 
		&& getCameraPointsCount() > 2)
	{		
		addPointToCamera(_lastPoint);
	}
	
	// Create face3D
	if(getCameraPointsCount() > 3)
	{
		createFace3d(view, getCameraPoints());
		
		// Keep the last two points to connect the next face
		if(MVGBuildFaceManipulator::_connectFace)
		{
			std::vector<MPoint> tmp(getCameraPoints());
			clearCameraPoints();
			addPointToCamera(tmp[3]);
			addPointToCamera(tmp[2]);
		}
		else
		{
			clearCameraPoints();
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

	// Check for existing points
	MVGMesh mesh(MVGProject::_MESH);
	if(!mesh.isValid()) {
		mesh = MVGMesh::create(MVGProject::_MESH);
		LOG_INFO("New OpenMVG Mesh.")
	}
	
	// Get mesh points (World Coords)
	MPointArray meshPoints;
	mesh.getPoints(meshPoints);
		
	if(meshPoints.length() > 0)
	{
		
		// Check intersection in 2D ?
		// Mouse intersects existing point
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
			while(!edgeIt.isDone())
			{
				view.worldToView(edgeIt.point(0), x0, y0);
				view.worldToView(edgeIt.point(1), x1, y1);
				
				MVector wdir;
				MPoint A;
				view.viewToWorld(x0, y0, A, wdir);
				MPoint B;
				view.viewToWorld(x1, y1, B, wdir);
				
				if(arePointsAligned2d(_mousePoint, A, B))
				{
					_doIntersectExistingEdge = true;
					_intersectingEdgePoints.clear();
					_intersectingEdgePoints.append(A);
					_intersectingEdgePoints.append(B);
					break;
				}
				edgeIt.next();
			}
		}
	}

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
	for(size_t i = 1; i < getCameraPointsCount(); ++i)
		drawManager.line2d(getCameraPointAtIndex(i-1), getCameraPointAtIndex(i));
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

void MVGBuildFaceManipulator::createFace3d(M3dView& view, std::vector<MPoint> facePoints)
{
	// Check points order	
	MVector AD = getCameraPointAtIndex(3) - getCameraPointAtIndex(0);
	MVector BC = getCameraPointAtIndex(2) - getCameraPointAtIndex(1);
			
	if(edgesIntersection(getCameraPointAtIndex(0), getCameraPointAtIndex(1), AD, BC))
	{
		std::vector<MPoint>	tmp(getCameraPoints());
		setCameraPointAtIndex(3, tmp[2]);
		setCameraPointAtIndex(2, tmp[3]);
	}
	
	MVGFace3D face3D;
	MVGFace2D face2D(getCameraPoints());
		
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

std::vector<MPoint> MVGBuildFaceManipulator::getCameraPoints() const
{
	return getMVGCamera().getPoints();
}

void MVGBuildFaceManipulator::setCameraPoints(std::vector<MPoint> points) const
{
	getMVGCamera().setPoints(points);
}

void MVGBuildFaceManipulator::addPointToCamera(MPoint& point) const
{
	getMVGCamera().addPoint(point);
}

void MVGBuildFaceManipulator::clearCameraPoints() const
{
	getMVGCamera().clearPoints();
}

MPoint MVGBuildFaceManipulator::getCameraPointAtIndex(int i) const
{
	return getMVGCamera().getPointAtIndex(i);
}

void MVGBuildFaceManipulator::setCameraPointAtIndex(int i, MPoint point) const
{
	getMVGCamera().setPointAtIndex(i, point);
}

int MVGBuildFaceManipulator::getCameraPointsCount() const
{
	return getMVGCamera().getPointsCount();
}

bool MVGBuildFaceManipulator::isShapeFinished() const
{
	return getMVGCamera().isShapeFinished();
}

void MVGBuildFaceManipulator::setIsShapeFinished(bool finished) const
{
	getMVGCamera().setIsShapeFinished(finished);
}

}

