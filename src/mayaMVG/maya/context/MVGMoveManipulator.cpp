#include "mayaMVG/maya/context/MVGMoveManipulator.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"
#include "mayaMVG/core/MVGProject.h"


using namespace mayaMVG;

MTypeId MVGMoveManipulator::_id(0x99222); // FIXME /!\ 


MVGMoveManipulator::MVGMoveManipulator() :
	_intersectionState(MVGManipulatorUtil::eIntersectionNone)
	, _moveState(eMoveNone)
	, _ctx(NULL)
{	
	_intersectionData.pointIndex = -1;
}

MVGMoveManipulator::~MVGMoveManipulator()
{
}

void * MVGMoveManipulator::creator()
{
	return new MVGMoveManipulator();
}

MStatus MVGMoveManipulator::initialize()
{
	return MS::kSuccess;
}

void MVGMoveManipulator::postConstructor()
{
	registerForMouseMove();
}

void MVGMoveManipulator::draw(M3dView & view, const MDagPath & path,
                               M3dView::DisplayStyle style, M3dView::DisplayStatus dispStatus)
{
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);

	view.beginGL();

	// enable gl picking
	// will call manipulator::doPress/doRelease 
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);
	colorAndName(view, glPickableItem, true, mainColor());
		
	// draw	
	MVGDrawUtil::begin2DDrawing(view);
		MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME
				
		// Draw Camera points
		glColor3f(1.f, 0.5f, 0.f);
		MVGManipulatorUtil::drawCameraPoints(view, data);
		
		if(MVGMayaUtil::isActiveView(view))
		{		
			// Draw intersections
			MVGManipulatorUtil::drawIntersections(view, data, _intersectionData, _intersectionState);

			switch(_moveState)
			{
				case eMoveNone:
					break;
				case eMovePoint:
					glColor3f(0.9f, 0.5f, 0.4f);
					MVGDrawUtil::drawFullCross(mousex, mousey);
					break;
				case eMoveEdge:
					break;
			}
		}
		
	MVGDrawUtil::end2DDrawing();
	view.endGL();
}

MStatus MVGMoveManipulator::doPress(M3dView& view)
{
	// Left Button
	Qt::MouseButtons mouseButtons = QApplication::mouseButtons();
	if(!(mouseButtons & Qt::LeftButton))
		return MS::kFailure;
	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	
	updateIntersectionState(view, data, mousex, mousey);
	
	switch(_intersectionState)
	{
		case MVGManipulatorUtil::eIntersectionNone:
			break;
		case MVGManipulatorUtil::eIntersectionPoint:
			_moveState = eMovePoint;
			break;
		case MVGManipulatorUtil::eIntersectionEdge:
			_moveState = eMoveEdge;
			break;
	}
	
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
{	
	short mousex, mousey;
	mousePosition(mousex, mousey);
	
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePoint);
	switch(_moveState)
	{
		case eMoveNone:
			break;
		case eMovePoint:
		{		
			try {	
				PairStringToPoint cameraPair = std::make_pair(data->camera.name(), data->cameraPoints2D[_intersectionData.pointIndex]);
				const PairStringToPoint meshPair = MVGProjectWrapper::instance().getMap2Dto3D().at(cameraPair);
				
				MVGProjectWrapper::instance().getMap2Dto3D().erase(cameraPair);		
				std::vector<PairStringToPoint >& vec = MVGProjectWrapper::instance().getMap3Dto2D().at(meshPair);			
				vec.erase(std::remove(vec.begin(), vec.end(), cameraPair));
				
				cameraPair = std::make_pair(data->camera.name(), mousePoint);
				MVGProjectWrapper::instance().getMap2Dto3D().insert(std::make_pair(cameraPair, meshPair));
				vec.push_back(cameraPair);		
				
			}
			catch(const std::exception& e){
				break;
			}
			
			data->cameraPoints2D[_intersectionData.pointIndex] = mousePoint;
			data->camera.setClickedtPointAtIndex(_intersectionData.pointIndex, mousePoint);			
			break;
		}
		case eMoveEdge:
			break;
	}
	
	_moveState = eMoveNone;
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{	
	refresh = true;
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	if(data->buildPoints2D.length() == 0 && data->cameraPoints2D.length() == 0)
		return MS::kSuccess;
	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	updateIntersectionState(view, data, mousex, mousey);
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{			
	return MPxManipulatorNode::doDrag(view);
}

void MVGMoveManipulator::preDrawUI(const M3dView& view)
{
}

void MVGMoveManipulator::setContext(MVGContext* ctx)
{
	_ctx = ctx;
}

void MVGMoveManipulator::updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey)
{
	_intersectionData.pointIndex = -1;
	_intersectionData.edgePointIndexes.clear();
	if(MVGManipulatorUtil::intersectPoint(view, data, _intersectionData, mousex, mousey)) {
		_intersectionState = MVGManipulatorUtil::eIntersectionPoint;
	} else if(MVGManipulatorUtil::intersectEdge(view, data, _intersectionData, mousex, mousey)) {
	 	_intersectionState = MVGManipulatorUtil::eIntersectionEdge;
	} else {
		_intersectionState = MVGManipulatorUtil::eIntersectionNone;
	}
}

void MVGMoveManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	// TODO
	drawManager.endDrawable();
}
