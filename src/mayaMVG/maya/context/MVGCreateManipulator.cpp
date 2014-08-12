#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/qt/MVGUserLog.h"
#include "mayaMVG/core/MVGGeometryUtil.h"

namespace mayaMVG {

MTypeId MVGCreateManipulator::_id(0x99111); // FIXME /!\ 

MVGCreateManipulator::MVGCreateManipulator()
    : _createColor(0.f, 0.f, 1.f)
    , _extendColor(0.9f, 0.9f, 0.1f)
    , _faceColor(1.f, 1.f, 1.f)
    , _cursorColor(0.f, 0.f, 0.f)
{
	_manipUtils.intersectionData().pointIndex = -1;
}

MVGCreateManipulator::~MVGCreateManipulator()
{
}

void * MVGCreateManipulator::creator()
{
	return new MVGCreateManipulator();
}

MStatus MVGCreateManipulator::initialize()
{
	return MS::kSuccess;
}

void MVGCreateManipulator::postConstructor()
{
	registerForMouseMove();
}

void MVGCreateManipulator::draw(M3dView & view, const MDagPath & path,
                               M3dView::DisplayStyle style, M3dView::DisplayStatus dispStatus)
{
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return;
	
	short mousex, mousey;
	updateMouse(view, data, mousex, mousey);

	view.beginGL();

    // CLEAN the input maya OpenGL State
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_LINE_STIPPLE);
    
    {
        // enable gl picking
        // will call manipulator::doPress/doRelease 
        MGLuint glPickableItem;
        glFirstHandle(glPickableItem);
        colorAndName(view, glPickableItem, true, mainColor());

        // Preview 3D (while extending edge)
        _manipUtils.drawPreview3D();

        // Draw	
        MVGDrawUtil::begin2DDrawing(view);
            MPoint center(0, 0);
            MVGDrawUtil::drawCircle2D(center, _cursorColor, 1, 5); // needed - FIXME

            // Draw only in active view
            if(MVGMayaUtil::isActiveView(view))
            {
                drawCursor(mousex, mousey);
                drawIntersections(view, mousex, mousey);
                glColor3f(1.f, 0.f, 0.f);
                drawPreview2D(view, data);
            }

        MVGDrawUtil::end2DDrawing();
    }
	view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{	
	// Left Button
	Qt::MouseButtons mouseButtons = QApplication::mouseButtons();
	if(!(mouseButtons & Qt::LeftButton))
		return MS::kFailure;
	
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	// Undo/Redo
	MVGEditCmd* cmd = NULL;
	if(!_manipUtils.getContext()) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}
	
	short mousex, mousey;
	MPoint mousePoint;
	mousePoint = updateMouse(view, data, mousex, mousey);

    _manipUtils.updateIntersectionState(view, data, mousex, mousey);
	switch(_manipUtils.intersectionState()) {
		case MVGManipulatorUtil::eIntersectionNone: 
        case MVGManipulatorUtil::eIntersectionPoint:
        {		
            _createState = eCreateNone;			
			data->buildPoints2D.append(mousePoint);
						
			// Create face if enough points (4))
			if(data->buildPoints2D.length() < 4)
				break;
			
			// Compute 3D face
			MPointArray facePoints3D;	
			if(!MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->buildPoints2D))
            {
                data->buildPoints2D.remove(data->buildPoints2D.length() - 1);
                USER_ERROR("Can't find a 3D face with these points")
                break;
            }

			MDagPath emptyPath;
			if(!_manipUtils.addCreateFaceCommand(cmd, emptyPath, facePoints3D))
				return MS::kFailure;					
			break;
		}
			break;
		case MVGManipulatorUtil::eIntersectionEdge:
			_manipUtils.computeEdgeIntersectionData(view, data, mousePoint);            
            _createState = eCreateExtend;
			break;
	}

	return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{	
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	// Undo/Redo
	MVGEditCmd* cmd = NULL;
	if(!_manipUtils.getContext()) {
	   LOG_ERROR("invalid context object.")
	   return MS::kFailure;
	}
	
	switch(_createState) {
		case eCreateNone:
			break;
		case eCreateExtend: 
		{			
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(_manipUtils.intersectionData().meshName.c_str(), meshPath);
			if(!_manipUtils.addCreateFaceCommand(cmd, meshPath, _manipUtils.previewFace3D()))
				return MS::kFailure;
			_manipUtils.previewFace3D().clear();
			break;
		}
	}
    
    // FIX ME : error with view if called after addCreateFaceCommand()
    //_manipUtils.updateIntersectionState(view, data, mousex, mousey);
    _createState = eCreateNone;	
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGCreateManipulator::doMove(M3dView& view, bool& refresh)
{	
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	_manipUtils.updateIntersectionState(view, data, mousex, mousey);
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGCreateManipulator::doDrag(M3dView& view)
{		
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	MPoint mousePoint;
	mousePoint = updateMouse(view, data, mousex, mousey);
		
	switch(_createState) {
		case eCreateNone:
			break;
		case eCreateExtend: 
		{
			computeTmpFaceOnEdgeExtend(view, data, mousePoint);
			break;
		}
	}
	return MPxManipulatorNode::doDrag(view);
}

void MVGCreateManipulator::preDrawUI(const M3dView& view)
{
}

void MVGCreateManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	// TODO
	drawManager.endDrawable();
}

void MVGCreateManipulator::setContext(MVGContext* ctx)
{
	_manipUtils.setContext(ctx);
}

MPoint MVGCreateManipulator::updateMouse(M3dView& view, DisplayData* data, short& mousex, short& mousey)
{
	mousePosition(mousex, mousey);
	MPoint mousePointInCameraCoord;
	MVGGeometryUtil::viewToCamera(view, mousex, mousey, mousePointInCameraCoord);
	
	return mousePointInCameraCoord;
}

void MVGCreateManipulator::drawCursor(float mousex, float mousey)
{
	MVGDrawUtil::drawTargetCursor(mousex, mousey, _cursorColor);
	
	if(_manipUtils.intersectionState() == MVGManipulatorUtil::eIntersectionEdge)
		MVGDrawUtil::drawExtendItem(mousex + 10, mousey + 10, _extendColor);
}

void MVGCreateManipulator::drawIntersections(M3dView& view, float mousex, float mousey)
{
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return;
	
	if(data->allPoints2D.size() == 0)
		return;
	
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& meshPoints = data->allPoints2D[intersectionData.meshName];
	
	short x, y;
	switch(_manipUtils.intersectionState())
	{
		case MVGManipulatorUtil::eIntersectionPoint:
			break;
		case MVGManipulatorUtil::eIntersectionEdge:				
        {
            MPoint A = MVGGeometryUtil::worldToView(view,  meshPoints[intersectionData.edgePointIndexes[0]].point3D);
            MPoint B = MVGGeometryUtil::worldToView(view, meshPoints[intersectionData.edgePointIndexes[1]].point3D);
            MVGDrawUtil::drawLine2D(A, B, _extendColor);
			break;
        }
	}	
}

void MVGCreateManipulator::drawPreview2D(M3dView& view, DisplayData* data)
{
	short mousex, mousey;
	mousePosition(mousex, mousey);
    MPoint viewPointA;
    MPoint viewPointB;
	
	MPointArray points = data->buildPoints2D;
	if(points.length() > 0)
	{
		for(int i = 0; i < points.length() - 1; ++i) {
			MVGGeometryUtil::cameraToView(view, points[i], viewPointA);
            MVGGeometryUtil::cameraToView(view, points[i+1], viewPointB);
			MVGDrawUtil::drawCircle2D(viewPointA, _createColor, POINT_RADIUS, 30);
            MVGDrawUtil::drawLine2D(viewPointA, viewPointB, _createColor);
		}
		
		// Last point to mouse
		MVGGeometryUtil::cameraToView(view, points[points.length() - 1], viewPointA);
		MVGDrawUtil::drawCircle2D(viewPointA, _createColor, POINT_RADIUS, 30);
        MPoint mouseView(mousex, mousey);
        MVGDrawUtil::drawLine2D(viewPointA, mouseView, _createColor);
	}
	if(points.length() > 2)
	{
        MPointArray drawPoints;
        for(int i = 0; i < points.length(); ++i)
        {
            MVGGeometryUtil::cameraToView(view, points[i], viewPointA);
            drawPoints.append(viewPointA);
        }
        drawPoints.append(MPoint(mousex, mousey));
        MVGDrawUtil::drawLineLoop2D(drawPoints, _createColor);
        MVGDrawUtil::drawPolygon2D(drawPoints, _faceColor, 0.6f);
	}
}

void MVGCreateManipulator::computeTmpFaceOnEdgeExtend(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord)
{
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtils.intersectionData();
	std::vector<MVGPoint2D>& mvgPoints = data->allPoints2D[intersectionData.meshName];
	
	// Get edge 3D points 
	MPoint edgePoint3D_0 = mvgPoints[intersectionData.edgePointIndexes[0]].point3D;
	MPoint edgePoint3D_1 = mvgPoints[intersectionData.edgePointIndexes[1]].point3D;

	// Build 2D points preview to compute 3D face
	MPointArray previewPoints2D;
	previewPoints2D.append(mvgPoints[intersectionData.edgePointIndexes[0]].projectedPoint3D);
	previewPoints2D.append(mvgPoints[intersectionData.edgePointIndexes[1]].projectedPoint3D);
	MPoint P3 = mousePointInCameraCoord - (1 - intersectionData.edgeRatio) * intersectionData.edgeHeight2D;
	MPoint P4 = mousePointInCameraCoord + intersectionData.edgeRatio * intersectionData.edgeHeight2D;
	previewPoints2D.append(P3);
	previewPoints2D.append(P4);

	// Compute 3D face
	_manipUtils.previewFace3D().clear();
	if(!MVGGeometryUtil::projectFace2D(view, _manipUtils.previewFace3D(), data->camera, previewPoints2D, true, intersectionData.edgeHeight3D))
	{ 
        _manipUtils.previewFace3D().setLength(4);    
        MVGMesh mesh(intersectionData.meshName);
        if(!mesh.isValid())
            return;
        
        // Compute plane with old face points
        MPointArray faceVertices;
        for(int i = 0; i < intersectionData.facePointIndexes.length(); ++i)
        {
            faceVertices.append(mvgPoints[intersectionData.facePointIndexes[i]].point3D);
        }     
        MPoint movedPoint;
        PlaneKernel::Model model;
        MVGGeometryUtil::computePlane(faceVertices, model);

        // Project new points on plane		
        MVGGeometryUtil::projectPointOnPlane(P3, view, model, data->camera, movedPoint);
        _manipUtils.previewFace3D()[2] = movedPoint;

        // Keep 3D length
        MVGGeometryUtil::projectPointOnPlane(P4, view, model, data->camera, movedPoint);
        _manipUtils.previewFace3D()[3] = movedPoint;		
	}
         
    // Keep the old first 2 points to have a connected face
    _manipUtils.previewFace3D()[0] = edgePoint3D_1;
    _manipUtils.previewFace3D()[1] = edgePoint3D_0;
   
	// TODO : compute plane with straight line constraint
}
}	//mayaMVG
