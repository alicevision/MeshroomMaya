#include "mayaMVG/maya/context/MVGBuildFaceManipulator.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGProject.h"
#include <maya/MFnCamera.h>
#include <maya/MFnMesh.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MItMeshVertex.h>
#include <maya/MSelectionList.h>
#include <vector>

namespace mayaMVG {
	
#define EDGE_TOLERANCE 1.0e-5

MTypeId MVGBuildFaceManipulator::_id(0x99999); // FIXME /!\ 

MDagPath MVGBuildFaceManipulator::_lastCameraPath = MDagPath();
MVGCamera MVGBuildFaceManipulator::_camera = MVGCamera(_lastCameraPath);
std::vector<MPoint> MVGBuildFaceManipulator::_display2DPoints_world = std::vector<MPoint>();
MVGBuildFaceManipulator::EMode	MVGBuildFaceManipulator::_mode = eModeCreate;

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
	
	bool isPointOnEdge(MPoint& P, MPoint& A, MPoint& B, double tolerance)
	{		
		MVector AB = B - A;
		MVector PA = A - P;
		MVector AP = P - A;
		MVector BP = P - B;
		MVector BA = A - B;
		
		// R
//		double r = dotProduct2d(PA, AB) / (AB.length() * AB.length());
//		LOG_INFO("=============");
//		LOG_INFO("P = " << P);
//		LOG_INFO("A = " << A);
//		LOG_INFO("B = " << B);
//		LOG_INFO("tolerance = " << tolerance);
//		LOG_INFO("AB.length = " << AB.length());
//		LOG_INFO("r = " << r);
//		LOG_INFO("=============");
		
//		if(r < -tolerance 
//			|| r > 1 + tolerance)
//			return false;
		
		// Dot signs				
		int sign1, sign2;
		
		(dotProduct2d(AP, AB) > 0) ? sign1 = 1 : sign1 = -1;
		(dotProduct2d(BP, BA) > 0) ? sign2 = 1 : sign2 = -1;
		
		if(sign1 != sign2)
			return false;	
	
		// Lenght of orthogonal projection on edge
		double s = crossProduct2d(AB, PA) /  (AB.length()*AB.length());
		if(s < 0)
			s*= -1;
		double PH = s * AB.length();

		if(PH < - tolerance 
			|| PH > tolerance)
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
	_cameraPathClickedPoints = MDagPath();
	_pressedPointId = -1;
	const GLfloat color[] = {1.f, 0.f, 0.f, 0.6f};
	_drawColor = std::vector<GLfloat>(color, color + sizeof(color)/ sizeof(GLfloat));	
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
	updateMouse(view);
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
			
			MDagPath cameraPath;
			view.getCamera(cameraPath);
			
			switch(_editAction)
			{
				case eEditActionNone:
					// Intersection with point
					if(intersectPoint(view, _mousePoint))
					{
						if(_connectedFacesId.length() == 1)
						{	
							// Green
							if(_mode == eModeMoveInPlane)
								glColor4f(0.f, 1.f, 0.f, 0.6f);
								
							// Purple
							else if(_mode == eModeMoveRecompute && !_faceConnected)
								glColor4f(0.5f, 0.3f, 0.9f, 0.6f);
	
							short x, y;				
							glPointSize(4.f);
							glBegin(GL_POINTS);
							MVGGeometryUtil::cameraToView(view, _camera, _mousePoint, x, y);
								glVertex2f(x, y);
							glEnd();	
						}						
					}

					// Intersection with edge
					else if(intersectEdge(view, _mousePoint))
					{
						// Yellow
						if(_mode == eModeCreate)
							glColor4f(0.9f, 0.9f, 0.1f, 0.6f);

						else if(_connectedFacesId.length() == 1
							&& !_edgeConnected)
						{
							// Green
							if(_mode == eModeMoveInPlane)
								glColor4f(0.f, 1.f, 0.f, 0.6f);
							// Purple
							else if(eModeMoveRecompute)
								glColor4f(0.5f, 0.3f, 0.9f, 0.6f);

						}

						short x, y;
						glBegin(GL_LINES);
							view.worldToView(_intersectingEdgePoints3D[0], x, y);
							glVertex2f(x, y);
							view.worldToView(_intersectingEdgePoints3D[1], x, y);
							glVertex2f(x, y);
						glEnd();
					}
					
					// Draw lines and poly : if face creation
					if(!_display2DPoints_world.empty())
					{
						short x;
						short y;
						glColor4f(1.f, 0.f, 0.f, 0.6f);

						if(_display2DPoints_world.size() < 3 
							&& (_cameraPathClickedPoints == _lastCameraPath))
						{
							// Lines
							if(_display2DPoints_world.size() > 1)
							{
								glBegin(GL_LINES);
								MVGGeometryUtil::cameraToView(view, _camera, _display2DPoints_world[0], x, y);
								glVertex2f(x, y);
								MVGGeometryUtil::cameraToView(view, _camera, _display2DPoints_world[1], x, y);
								glVertex2f(x, y);
								glEnd();	

								glPointSize(4.f);
								glBegin(GL_POINTS);
								for(size_t i = 0; i < _display2DPoints_world.size(); ++i){
									MVGGeometryUtil::cameraToView(view, _camera, _display2DPoints_world[i], x, y);
									glVertex2f(x, y);
								}
								glEnd();					
							}	

							if(_display2DPoints_world.size() > 0)
							{
								glBegin(GL_LINES);
								MVGGeometryUtil::cameraToView(view, _camera, _display2DPoints_world[_display2DPoints_world.size() - 1], x, y);
								glVertex2f(x, y);
								MVGGeometryUtil::cameraToView(view, _camera, _mousePoint, x, y);
								glVertex2f(x, y);
								glEnd();	

							}		
						}
					}
					
					// Preview 2D
					if((_display2DPoints_world.size() > 2
						&& _lastCameraPath == _cameraPathClickedPoints))
					{
						short x, y;
						glColor4f(0.f, 0.f, 1.f, 0.6f);
						glLineWidth(1.5f);
						glBegin(GL_LINE_LOOP);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[0], x, y);
							glVertex2f(x, y);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[1], x, y);
							glVertex2f(x, y);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[2], x, y);
							glVertex2f(x, y);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[3], x, y);
							glVertex2f(x, y);
						glEnd();

						glColor4f(1.f, 1.f, 1.f, 0.6f);
						glBegin(GL_POLYGON);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[0], x, y);
							glVertex2f(x, y);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[1], x, y);
							glVertex2f(x, y);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[2], x, y);
							glVertex2f(x, y);
							MVGGeometryUtil::cameraToView(view, _camera, _preview2DFace._p[3], x, y);
							glVertex2f(x, y);
						glEnd();
					}
					break;
				case eEditActionExtendEdge:
					// Preview
					short x, y;
					glColor4f(0.f, 0.f, 1.f, 0.6f);
					glLineWidth(1.5f);
					glBegin(GL_LINE_LOOP);
						view.worldToView(_preview3DFace._p[0], x, y);
						glVertex2f(x, y);
						view.worldToView(_preview3DFace._p[1], x, y);
						glVertex2f(x, y);
						view.worldToView(_preview3DFace._p[2], x, y);
						glVertex2f(x, y);
						view.worldToView(_preview3DFace._p[3], x, y);
						glVertex2f(x, y);
					glEnd();

					glColor4f(1.f, 1.f, 1.f, 0.6f);
					glBegin(GL_POLYGON);
						view.worldToView(_preview3DFace._p[0], x, y);
						glVertex2f(x, y);
						view.worldToView(_preview3DFace._p[1], x, y);
						glVertex2f(x, y);
						view.worldToView(_preview3DFace._p[2], x, y);
						glVertex2f(x, y);
						view.worldToView(_preview3DFace._p[3], x, y);
						glVertex2f(x, y);
					glEnd();
					break;
				case eEditActionMoveEdge:
				case eEditActionMovePoint:
					break;		
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
	updateMouse(view);
	updateCamera(view);
			
//	
//	LOG_INFO("### PRESS ###");
//	short x, y;
//	MPoint mouse;
//	mousePosition(x, y);
//	
//	LOG_INFO("viewMouse = [" << x << ", " << y << "]");
//	MVGGeometryUtil::viewToCamera(view, _camera, x, y, mouse);
//	LOG_INFO("cameraMouse = " << mouse);
//	MVGGeometryUtil::cameraToView(viex, _camera, mouse, x, y);
//	LOG_INFO("viewMouse = [" << x << ", " << y << "]");
//	MVGGeometryUtil::viewToCamera(view, _camera, x, y, mouse);
//	LOG_INFO("cameraMouse = " << mouse);
	// Define action
	if(intersectPoint(view, _mousePoint))
	{
		switch(_mode)
		{
			case eModeCreate:
				break;
			case eModeMoveInPlane:
				if(_connectedFacesId.length() == 1)
					_editAction = eEditActionMovePoint;
				break;
			case eModeMoveRecompute:
				if(_connectedFacesId.length() == 1
					&& !_faceConnected)
					_editAction = eEditActionMovePoint;
				break;				
		}
	}
	else if(intersectEdge(view, _mousePoint))
	{
		switch(_mode)
		{
			case eModeCreate:
				_editAction = eEditActionExtendEdge;
				break;
			case eModeMoveInPlane:
				// Check edge status
				if(_connectedFacesId.length() == 1
					&& !_edgeConnected)
				{
					_editAction = eEditActionMoveEdge;
				}
				break;
			case eModeMoveRecompute:
				// Check edge status
				if(_connectedFacesId.length() == 1
					&& !_edgeConnected)
				{
					_editAction = eEditActionMoveEdge;
				}
				break;
		}
	}
	else
	{
		_editAction = eEditActionNone;
	}
	
	switch(_editAction)
	{
		case eEditActionExtendEdge:
		case eEditActionMoveEdge:
			{
				// Compute height and edge ratio
				MPoint intersectingEdgePoints2D_0, intersectingEdgePoints2D_1;
				MVGGeometryUtil::worldToCamera(view, _camera, _intersectingEdgePoints3D[0], intersectingEdgePoints2D_0);
				MVGGeometryUtil::worldToCamera(view, _camera, _intersectingEdgePoints3D[1], intersectingEdgePoints2D_1);
								
				MVector ratioVector2D = intersectingEdgePoints2D_1 - _mousePoint;
				_edgeHeight2D = intersectingEdgePoints2D_1 - intersectingEdgePoints2D_0;
				_edgeRatio = ratioVector2D.length() / _edgeHeight2D.length();
				
				_edgeHeight3D = _intersectingEdgePoints3D[1] - _intersectingEdgePoints3D[0];

				if( _editAction == eEditActionExtendEdge )
				{
					_mousePointOnPressEdge = _mousePoint;
					_clickedEdgePoints3D.clear();
					_clickedEdgePoints3D.append(_intersectingEdgePoints3D[0]);
					_clickedEdgePoints3D.append(_intersectingEdgePoints3D[1]);
				}
			}
			break;
		case eEditActionMovePoint:
			break;
		case eEditActionNone:
		{
			if(!(_cameraPathClickedPoints == _lastCameraPath))
			{
				_display2DPoints_world.clear();
				_cameraPathClickedPoints = _lastCameraPath;
			}

			// Add a new point		
			_display2DPoints_world.push_back(_mousePoint);

			// Create face3D		
			if(_display2DPoints_world.size() > 3)
			{
				MVGFace3D face3D;
				std::vector<MPoint> pointArray;
				for(int i = 0; i < 4; ++i)
				{
					pointArray.push_back(_preview2DFace._p[i]);
				}
				if(computeFace3d(view, pointArray, face3D))
				{
					addFace3d(face3D);
					_display2DPoints_world.clear();
				}	
			}
			
			break;
		}
	}
	
	update2DFacePreview(view);

	return MPxManipulatorNode::doPress(view);
}

MStatus MVGBuildFaceManipulator::doRelease(M3dView& view)
{
	
	if(_editAction == eEditActionExtendEdge)
	{
		addFace3d(_preview3DFace);
	}
		
	_editAction = eEditActionNone;
	updateCamera(view);
	
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGBuildFaceManipulator::doMove(M3dView& view, bool& refresh)
{	
	updateMouse(view);
	updateCamera(view);
		
	// Warning if change of view during shape creation
	if( !_display2DPoints_world.empty() 
		&& !(_camera.dagPath() == _cameraPathClickedPoints))
	{
		LOG_WARNING("Change of view while creating a face : if you click a point, it will erase previous points. ");			
	}
	
	refresh = true;
	
	update2DFacePreview(view);
	
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGBuildFaceManipulator::doDrag(M3dView& view)
{	
//	LOG_INFO("======================================================= ");
//	LOG_INFO("======================= DO DRAG ======================= ");
//	LOG_INFO("======================================================= ");
	updateMouse(view);
	updateCamera(view);
	
	_mousePointOnDragEdge = _mousePoint;
	update3DFacePreview(view, _preview3DFace);
	
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

void MVGBuildFaceManipulator::updateCamera(M3dView& view)
{
	// Update camera & cameraPath
	MDagPath cameraPath;
	view.getCamera(cameraPath);

	if(!(cameraPath == _lastCameraPath))
	{
		_lastCameraPath = cameraPath;
		_camera = getMVGCamera();
	}
}

void MVGBuildFaceManipulator::updateMouse(M3dView& view)
{
	//MPoint mousePoint;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	MVGGeometryUtil::viewToCamera(view, _camera, mousex, mousey, _mousePoint);
	//return mousePoint;
}

void MVGBuildFaceManipulator::update2DFacePreview(M3dView& view)
{
	// Preview first face
	if(_display2DPoints_world.size() > 2
		&& (_lastCameraPath == _cameraPathClickedPoints))
	{		
		_preview2DFace._p[0] = _display2DPoints_world[0];
		_preview2DFace._p[1] = _display2DPoints_world[1];
		_preview2DFace._p[2] = _display2DPoints_world[2];
		_preview2DFace._p[3] = _mousePoint;
	}
}

bool MVGBuildFaceManipulator::update3DFacePreview(M3dView& view, MVGFace3D& face3D)
{
	bool check = false;
	std::vector<MPoint> previewPoints2d;
	
	switch(_editAction)
	{
		case eEditActionExtendEdge:
			{	
				MPoint point;

				MVGGeometryUtil::worldToCamera(view, _camera, _clickedEdgePoints3D[1], point);
				previewPoints2d.push_back(point);
				MVGGeometryUtil::worldToCamera(view, _camera, _clickedEdgePoints3D[0], point);
				previewPoints2d.push_back(point);			
				MPoint P3 = _mousePointOnDragEdge - (1 -_edgeRatio) * _edgeHeight2D;
				MPoint P4 = _mousePointOnDragEdge + _edgeRatio*_edgeHeight2D;
				previewPoints2d.push_back(P3);
				previewPoints2d.push_back(P4);

				// Preview keeping 3D length
				check = computeFace3d(view, previewPoints2d, _preview3DFace, true, _edgeHeight3D);

				// Keep the old first 2 points to have a connected face
				_preview3DFace._p[0] = _clickedEdgePoints3D[1];
				_preview3DFace._p[1] = _clickedEdgePoints3D[0];

				return check;
			}
			break;
			
		case eEditActionMoveEdge:
			{
				MVGMesh mesh(MVGProject::_MESH);
				if(!mesh.isValid()) {
					mesh = MVGMesh::create(MVGProject::_MESH);
					LOG_INFO("New OpenMVG Mesh.")
				}

				MIntArray vertices = mesh.getFaceVertices(_connectedFacesId[0]);	
				MPointArray meshPoints;
				mesh.getPoints(meshPoints);
				
				MIntArray edgeVertices = mesh.getEdgeVertices(_intersectedEdgeId);
				MIntArray fixedVertices;
				for(int i = 0; i < vertices.length(); ++i)
				{
					int found = false;
					for(int j = 0; j < edgeVertices.length(); ++j)
					{
						if(vertices[i] == edgeVertices[j])
							found = true;
					}

					if(!found)
						fixedVertices.append(vertices[i]);
				}
				switch(_mode)
				{
					case eModeMoveInPlane:
						{
							// Get face points in 3D
							MVGFace3D meshFace;
							for(int i = 0; i < 4; ++i)
							{
								meshFace._p[i] = meshPoints[vertices[i]];
							}

							// Compute plane with old face points
							short x, y;
							MPoint wpos;
							MVector wdir;
							MPoint movedPoint;
							PlaneKernel::Model model;
							MVGGeometryUtil::computePlane(meshFace, model);

							// Project new points on plane					
							MPoint P3 = _mousePointOnDragEdge + _edgeRatio * _edgeHeight2D;
							MVGGeometryUtil::projectPointOnPlane(P3, view, model, _camera, movedPoint);
							mesh.setPoint(edgeVertices[1], movedPoint);
							
							// Keep 3D length
							MPoint lastPoint3D = movedPoint - _edgeHeight3D; // --> Point 3D !
							MPoint lastPoint2D;
							MVGGeometryUtil::worldToCamera(view, _camera, lastPoint3D, lastPoint2D);
							MVGGeometryUtil::projectPointOnPlane(lastPoint2D, view, model, _camera, movedPoint);
							mesh.setPoint(edgeVertices[0], movedPoint);
						}
						break;
					case eModeMoveRecompute:
						{
							std::vector<MPoint> previewPoints2d;
							short x, y;
							MPoint point;
							MVector wdir;
							
							// First : fixed points
							MVGGeometryUtil::worldToCamera(view, _camera, meshPoints[fixedVertices[0]], point);
							previewPoints2d.push_back(point);
							MVGGeometryUtil::worldToCamera(view, _camera, meshPoints[fixedVertices[1]], point);
							previewPoints2d.push_back(point);
		
							// Then : mousePoints computed with egdeHeight and ratio							
							MPoint P3 = _mousePointOnDragEdge + _edgeRatio * _edgeHeight2D;
							previewPoints2d.push_back(P3);
							MPoint P4 = _mousePointOnDragEdge  - (1-_edgeRatio) *_edgeHeight3D;
							previewPoints2d.push_back(P4);
		
							// Only set the new points to keep a connected face
							if(computeFace3d(view, previewPoints2d, _preview3DFace, true, -_edgeHeight3D))
							{
								mesh.setPoint(edgeVertices[1], _preview3DFace._p[2]);
								mesh.setPoint(edgeVertices[0], _preview3DFace._p[3]);
							}
						}
						break;
				}
			}
			break;
		case eEditActionMovePoint:
			{
				MVGMesh mesh(MVGProject::_MESH);
				if(!mesh.isValid()) {
					mesh = MVGMesh::create(MVGProject::_MESH);
					LOG_INFO("New OpenMVG Mesh.")
				}
				MPointArray meshPoints;
				mesh.getPoints(meshPoints);

				MIntArray verticesId = mesh.getFaceVertices(_connectedFacesId[0]);	
				switch(_mode)
				{
					case eModeMoveInPlane:
						{
							MVGFace3D meshFace;
							for(int i = 0; i < 4; ++i)
							{
								meshFace._p[i] = meshPoints[verticesId[i]];
							}

							MPoint movedPoint;
							PlaneKernel::Model model;
							MVGGeometryUtil::computePlane(meshFace, model);
							MVGGeometryUtil::projectPointOnPlane(_mousePoint, view, model, _camera, movedPoint);

							mesh.setPoint(_pressedPointId, movedPoint);
						}
						break;
					case eModeMoveRecompute:
						{
							// Fill previewPoints2d with face points and dragMousePoint (in w2D)
							std::vector<MPoint> previewPoints2d;
							short x, y;
							MPoint wpos;
							MVector wdir;
							for(int i = 0; i < verticesId.length(); ++i)
							{
								if(_pressedPointId == verticesId[i])
								{
									previewPoints2d.push_back(_mousePoint);
								}
									
								else
								{
									MVGGeometryUtil::worldToCamera(view, _camera, meshPoints[verticesId[i]], wpos);
									previewPoints2d.push_back(wpos);
								}

							}

							// Compute face
							MVGFace3D face3D;							
							if(computeFace3d(view, previewPoints2d, face3D, false))
							{	
//								LOG_INFO("### Mesh Points 3D ###");
//								for(int i = 0; i < verticesId.length(); ++i)
//								{
//									LOG_INFO(meshPoints[verticesId[i]]);
//								}
								LOG_INFO("### Computed Points 3D ###");
								for(int i = 0; i < 4; ++i)
								{
									LOG_INFO(face3D._p[i]);
								}
																
//								LOG_INFO("### Mesh Points 2D ###");
//								MPoint camPoint;
//								for(int i = 0; i < verticesId.length(); ++i)
//								{
//									MVGGeometryUtil::worldToCamera(view, _camera, meshPoints[verticesId[i]], camPoint);
//									LOG_INFO(camPoint);
//								}
//								LOG_INFO("### Computed Points 2D ###");
//								for(int i = 0; i < 4; ++i)
//								{
//									MVGGeometryUtil::worldToCamera(view, _camera, face3D._p[i], camPoint);
//									LOG_INFO(camPoint);
//								}
								mesh.addPolygon(face3D);
			
//								for(int i = 0; i < verticesId.length(); ++i)
//								{
//									mesh.setPoint(verticesId[i], face3D._p[i]);
//								}
								
							}
						}
						break;
				}
			}	
			break;
		case eEditActionNone:
			break;
	}
		
	return false;
}

/**
 * 
 * @param[in] view
 * @param[in] pointArray
 * @param[out] outFace3D
 * @param[in] computeLastPoint
 * @param[in] height
 * 
 * @return if face computed
 */
bool MVGBuildFaceManipulator::computeFace3d(
	M3dView& view, std::vector<MPoint>& pointArray,
	MVGFace3D& outFace3D, bool computeLastPoint, MVector height)
{
	MVGFace2D face2D(pointArray);
	
	return MVGGeometryUtil::projectFace2D(outFace3D, view, _camera, face2D, computeLastPoint, height);
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

bool MVGBuildFaceManipulator::intersectPoint(M3dView& view, MPoint& point)
{		
	_faceConnected = false;
	MVGMesh mesh(MVGProject::_MESH);
	if(!mesh.isValid()) {
		mesh = MVGMesh::create(MVGProject::_MESH);
		LOG_INFO("New OpenMVG Mesh.")
	}
	MPointArray meshPoints;
	mesh.getPoints(meshPoints);

	if(meshPoints.length() < 1)
		return false;
	
	short pointX, pointY;
	MVGGeometryUtil::cameraToView(view, _camera, point, pointX, pointY);
	
	MFnCamera fnCamera(_camera.dagPath().node());
	for(int i = 0; i < meshPoints.length(); ++i)
	{
		short x, y;
		view.worldToView(meshPoints[i], x, y);

		// Tolerance en pixels ... 
		if(pointX <= x + 1 + fnCamera.zoom() * 10
			&& pointX >= x - 1 - fnCamera.zoom() * 10
			&& pointY <= y + 1 + fnCamera.zoom() * 10
			&& pointY >= y - 1 - fnCamera.zoom() * 10)
		{
			_pressedPointId = i;
			_connectedFacesId = mesh.getConnectedFacesToVertex(_pressedPointId);

			// Face not connected to other face
			MIntArray vertices = mesh.getFaceVertices(_connectedFacesId[0]);	
			for(int i = 0; i < vertices.length(); ++i)
			{
				if(mesh.getNumConnectedFacesToVertex(vertices[i]) > 1)
				{
					_faceConnected = true;

					return true;
				}
			}
			return true;
		}
	}
	
	return false;
}

bool MVGBuildFaceManipulator::intersectEdge(M3dView& view, MPoint& point)
{
	_edgeConnected = false;
	MVGMesh mesh(MVGProject::_MESH);
	if(!mesh.isValid()) {
		mesh = MVGMesh::create(MVGProject::_MESH);
		LOG_INFO("New OpenMVG Mesh.")
	}
	MPointArray meshPoints;
	mesh.getPoints(meshPoints);

	if(meshPoints.length() < 1)
		return false;
	
//	MPointArray points;
	short x0, y0, x1, y1;
	bool check = false;
//	if(mesh.intersect(_mousePoint, wdir, points)) // TODO: detect intersection with mesh
//	{
		MItMeshEdge edgeIt(mesh.dagPath());
		double minLenght = -1;
		double lenght;
		MPointArray edgePoints;

		MFnCamera fnCamera(_camera.dagPath().node());
		while(!edgeIt.isDone())
		{
			view.worldToView(edgeIt.point(0), x0, y0);
			view.worldToView(edgeIt.point(1), x1, y1);

			MPoint A, B;
			MVGGeometryUtil::viewToCamera(view, _camera, x0, y0, A);
			MVGGeometryUtil::viewToCamera(view, _camera, x1, y1, B);

			if(isPointOnEdge(_mousePoint, A, B, kMFnMeshTolerance * fnCamera.zoom() * 10))
			{
				check = true;
				lenght = A.distanceTo(B);
				if(minLenght < 0)
				{
					minLenght = lenght;
					edgePoints.clear();
					edgePoints.append(edgeIt.point(0));
					edgePoints.append(edgeIt.point(1));	
					_intersectedEdgeId = edgeIt.index();
				}
			}
			edgeIt.next();
		}
		if(check)
		{
			_intersectingEdgePoints3D = edgePoints;
			
			mesh.getConnectedFacesToEdge(_connectedFacesId, _intersectedEdgeId);
			if(_connectedFacesId.length() == 1)
			{
				// Check if edge vertices are connected to another face
				MIntArray edgeVertices = mesh.getEdgeVertices(_intersectedEdgeId);
				for(int i = 0; i < edgeVertices.length(); ++i)
				{
					if(mesh.getNumConnectedFacesToVertex(edgeVertices[i]) > 1)
					{
						_edgeConnected = true;
					}
				}
			}	
			return true;
		}
//	}			
	return false;
}
}	// namespace mayaMVG 

