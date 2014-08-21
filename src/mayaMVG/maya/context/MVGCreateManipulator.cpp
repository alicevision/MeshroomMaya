#include "mayaMVG/maya/context/MVGCreateManipulator.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/cmd/MVGEditCmd.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/qt/MVGUserLog.h"
#include "mayaMVG/qt/MVGQt.h"

using namespace mayaMVG;

class MVGContext;

MTypeId MVGCreateManipulator::_id(0x99111); // FIXME /!\

MVGCreateManipulator::MVGCreateManipulator()
	: _manipUtil(NULL)
    , _createState(eCreateNone)
    , _createColor(0.f, 1.f, 0.f)           // Green
    , _neutralCreateColor(0.7f, 0.7f, 0.7f) // Grey
    , _extendColor(0.9f, 0.9f, 0.1f)        // Yellow
    , _faceColor(1.f, 1.f, 1.f)             // White
    , _cursorColor(0.f, 0.f, 0.f)           // Black
{
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
	view.beginGL();

    // CLEAN the input maya OpenGL State
    glDisable(GL_POLYGON_STIPPLE);
    glDisable(GL_LINE_STIPPLE);

    // Enable Alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // update mouse coordinates & get it in camera space
    short mousex, mousey;
    MPoint mousePoint = updateMouse(view, mousex, mousey);

    {
        // enable gl picking
        // will call manipulator::doPress/doRelease
        MGLuint glPickableItem;
        glFirstHandle(glPickableItem);
        colorAndName(view, glPickableItem, true, mainColor());

        // Preview 3D (while extending edge)
        _manipUtil->drawPreview3D();

        // Draw	
        MVGDrawUtil::begin2DDrawing(view);
            MPoint center(0, 0);
            MVGDrawUtil::drawCircle2D(center, _cursorColor, 1, 5); // needed - FIXME

            // retrieve display data
            MVGManipulatorUtil::DisplayData* data = NULL;
            if(_manipUtil)
                data = _manipUtil->getDisplayData(view);
            // stop drawing in case of no data or not the active view
            if(!data || !MVGMayaUtil::isMVGView(view)) {
                MVGDrawUtil::end2DDrawing();
                view.endGL();
                return;
            }
            // Draw only in active view
            if(MVGMayaUtil::isActiveView(view))
            {
                drawCursor(mousex, mousey);
                drawIntersections(view, mousex, mousey);
                drawPreview2D(view, data);
            } else
                drawOtherPreview2D(view, data);

        MVGDrawUtil::end2DDrawing();
    }

    glDisable(GL_BLEND);

	view.endGL();
}

MStatus MVGCreateManipulator::doPress(M3dView& view)
{
	Qt::MouseButtons mouseButtons = QApplication::mouseButtons();
	if(!(mouseButtons & Qt::LeftButton))
		return MS::kFailure;

	MVGManipulatorUtil::DisplayData* data = NULL;
	if(_manipUtil)
		data = _manipUtil->getDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	MPoint mousePoint;
	mousePoint = updateMouse(view, mousex, mousey);

    _manipUtil->updateIntersectionState(view, data, mousex, mousey);
    // Clear buildPoint of the complementary view
    if(data->buildPoints2D.length() == 0)
    {
        MVGManipulatorUtil::DisplayData* complementaryCache = _manipUtil->getComplementaryDisplayData(view);
        if(complementaryCache != data)
            complementaryCache->buildPoints2D.clear();
    }
    switch(_createState)
    {
        case eCreateNone:
            if(_manipUtil->intersectionState() != MVGManipulatorUtil::eIntersectionEdge)
            {
                _createState = eCreateFace;
                data->buildPoints2D.append(mousePoint);
                if(data->buildPoints2D.length() < 4)
                    break;
                _createState = eCreateNone;              
                if(!createFace(view, data))
                    return MS::kFailure;
            }
            else
            {
                _manipUtil->computeEdgeIntersectionData(view, data, mousePoint);            
                _createState = eCreateExtend;
            }
            break;
        case eCreateFace:
            data->buildPoints2D.append(mousePoint);
            if(data->buildPoints2D.length() < 4)
                break;
            _createState = eCreateNone;
            if(!createFace(view, data))
                return MS::kFailure;
            break;
        case eCreateExtend:
            _manipUtil->computeEdgeIntersectionData(view, data, mousePoint);            
            _createState = eCreateExtend;
            break;
    }
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGCreateManipulator::doRelease(M3dView& view)
{	
	MVGManipulatorUtil::DisplayData* data = NULL;
	if(_manipUtil)
		data = _manipUtil->getDisplayData(view);
	if(!data)
		return MS::kFailure;

	switch(_createState) {
		case eCreateNone:
        case eCreateFace:
			break;
		case eCreateExtend: 
		{
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(_manipUtil->intersectionData().meshName.c_str(), meshPath);
			if(!_manipUtil->addCreateFaceCommand(meshPath, _manipUtil->previewFace3D()))
				return MS::kFailure;
			_manipUtil->previewFace3D().clear();
			_createState = eCreateNone;
			break;
		}
	}

	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGCreateManipulator::doMove(M3dView& view, bool& refresh)
{	
	MVGManipulatorUtil::DisplayData* data = NULL;
	if(_manipUtil)
		data = _manipUtil->getDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	_manipUtil->updateIntersectionState(view, data, mousex, mousey);

	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGCreateManipulator::doDrag(M3dView& view)
{
	MVGManipulatorUtil::DisplayData* data = NULL;
	if(_manipUtil)
		data = _manipUtil->getDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	MPoint mousePoint;
	mousePoint = updateMouse(view, mousex, mousey);

	switch(_createState) {
		case eCreateNone:
        case eCreateFace:
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


MPoint MVGCreateManipulator::updateMouse(M3dView& view, short& mousex, short& mousey)
{
	mousePosition(mousex, mousey);
	MPoint mousePointInCameraCoord;
	MVGGeometryUtil::viewToCamera(view, mousex, mousey, mousePointInCameraCoord);
	return mousePointInCameraCoord;
}

void MVGCreateManipulator::drawCursor(const float mousex, const float mousey)
{
	MVGDrawUtil::drawTargetCursor(mousex, mousey, _cursorColor);

    if(_createState == eCreateFace)
        return;
    
	if(_manipUtil->intersectionState() == MVGManipulatorUtil::eIntersectionEdge)
		MVGDrawUtil::drawExtendItem(mousex + 10, mousey + 10, _extendColor);
}

void MVGCreateManipulator::drawIntersections(M3dView& view, const float mousex, const float mousey)
{
	MVGManipulatorUtil::DisplayData* data = NULL;
	if(_manipUtil)
		data = _manipUtil->getDisplayData(view);
	if(!data || data->allPoints2D.empty())
		return;
	
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtil->intersectionData();
	std::vector<MVGManipulatorUtil::MVGPoint2D>& meshPoints = data->allPoints2D[intersectionData.meshName];
	
	switch(_manipUtil->intersectionState())
	{
		case MVGManipulatorUtil::eIntersectionPoint:
			break;
		case MVGManipulatorUtil::eIntersectionEdge:
        {
            if(_createState == eCreateFace)
                break;
            if(intersectionData.edgePointIndexes.length() == 0 || meshPoints.size() < intersectionData.edgePointIndexes[0] || meshPoints.size() < intersectionData.edgePointIndexes[1])
                return;
            MPoint A = MVGGeometryUtil::worldToView(view, meshPoints[intersectionData.edgePointIndexes[0]].point3D);
            MPoint B = MVGGeometryUtil::worldToView(view, meshPoints[intersectionData.edgePointIndexes[1]].point3D);
            MVGDrawUtil::drawLine2D(A, B, _extendColor);
			break;
        }
	}
}


void MVGCreateManipulator::drawPreview2D(M3dView& view, const MVGManipulatorUtil::DisplayData* data)
{
    if(!data)
        return;
    
	short mousex, mousey;
	mousePosition(mousex, mousey);

    MPoint viewPoint;

	const MPointArray& points = data->buildPoints2D;
	if(points.length() > 0)
	{
        MPointArray drawPoints;
        for(int i = 0; i < points.length(); ++i)
        {
			MVGGeometryUtil::cameraToView(view, points[i], viewPoint);
            drawPoints.append(viewPoint);
		}
        
        // Draw points
        for(int i = 0; i < drawPoints.length(); ++i)
            MVGDrawUtil::drawCircle2D(drawPoints[i], _createColor, POINT_RADIUS, 30);
        
        // Draw Poly
        if(drawPoints.length() > 2)
        {
            drawPoints.append(MPoint(mousex, mousey));
            MVGDrawUtil::drawLineLoop2D(drawPoints, _createColor);
            MVGDrawUtil::drawPolygon2D(drawPoints, _faceColor, 0.6f);
        }
        else
        {
            // Draw lines
            for(int i = 0; i < drawPoints.length() - 1; ++i) {
                MVGDrawUtil::drawCircle2D(drawPoints[i], _createColor, POINT_RADIUS, 30);
                MVGDrawUtil::drawLine2D(drawPoints[i], drawPoints[i+1], _createColor);
            }
            // Last point to mouse
            MPoint mouseView(mousex, mousey);
            MVGDrawUtil::drawLine2D(drawPoints[drawPoints.length() - 1], mouseView, _createColor);
        }
	}
}

void MVGCreateManipulator::drawOtherPreview2D(M3dView& view, const MVGManipulatorUtil::DisplayData* data)
{
    if(!data)
        return;
    
    short mousex, mousey;
	mousePosition(mousex, mousey);
    MPoint viewPoint;

	const MPointArray& points = data->buildPoints2D;
	if(points.length() > 0)
	{
        MPointArray drawPoints;
        for(int i = 0; i < points.length(); ++i)
        {
			MVGGeometryUtil::cameraToView(view, points[i], viewPoint);
            drawPoints.append(viewPoint);
		}
        
        // Draw points
        for(int i = 0; i < drawPoints.length(); ++i)
            MVGDrawUtil::drawCircle2D(drawPoints[i], _neutralCreateColor, POINT_RADIUS, 30);
        
        // Draw Poly
        if(drawPoints.length() > 2)
        {
            MVGDrawUtil::drawLineLoop2D(drawPoints, _neutralCreateColor);
            MVGDrawUtil::drawPolygon2D(drawPoints, _faceColor, 0.6f);
        }
        else
        {
            // Draw lines
            for(int i = 0; i < drawPoints.length() - 1; ++i) {
                MVGDrawUtil::drawCircle2D(drawPoints[i], _neutralCreateColor, POINT_RADIUS, 30);
                MVGDrawUtil::drawLine2D(drawPoints[i], drawPoints[i+1], _neutralCreateColor);
            }
        }
	}
}

void MVGCreateManipulator::computeTmpFaceOnEdgeExtend(M3dView& view, MVGManipulatorUtil::DisplayData* data, const MPoint& mousePointInCameraCoord)
{
	MVGManipulatorUtil::IntersectionData& intersectionData = _manipUtil->intersectionData();
	std::vector<MVGManipulatorUtil::MVGPoint2D>& mvgPoints = data->allPoints2D[intersectionData.meshName];
	
	// Get edge 3D points 
    if(intersectionData.edgePointIndexes.length() == 0 || mvgPoints.size() < intersectionData.edgePointIndexes[0] || mvgPoints.size() < intersectionData.edgePointIndexes[1])
        return;
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
	_manipUtil->previewFace3D().clear();
	if(!MVGGeometryUtil::projectFace2D(view, _manipUtil->previewFace3D(), data->camera, previewPoints2D, intersectionData.edgeHeight3D))
	{
        _manipUtil->previewFace3D().setLength(4);
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
        _manipUtil->previewFace3D()[2] = movedPoint;

        // Keep 3D length
        MVGGeometryUtil::projectPointOnPlane(P4, view, model, data->camera, movedPoint);
        _manipUtil->previewFace3D()[3] = movedPoint;		
	}
         
    // Keep the old first 2 points to have a connected face
    _manipUtil->previewFace3D()[0] = edgePoint3D_1;
    _manipUtil->previewFace3D()[1] = edgePoint3D_0;
   
	// TODO : compute plane with straight line constraint
}

bool MVGCreateManipulator::createFace(M3dView& view, MVGManipulatorUtil::DisplayData* data)
{
    if(!data)
        return false;
    
    // Compute 3D face
    MPointArray facePoints3D;	
    if(!MVGGeometryUtil::projectFace2D(view, facePoints3D, data->camera, data->buildPoints2D))
    {
        data->buildPoints2D.remove(data->buildPoints2D.length() - 1);
        USER_ERROR("Can't find a 3D face with these points")
        return false;
    }

    data->buildPoints2D.clear();
    MDagPath emptyPath;
    if(!_manipUtil->addCreateFaceCommand(emptyPath, facePoints3D))
        return false;
    
    return true;
}
