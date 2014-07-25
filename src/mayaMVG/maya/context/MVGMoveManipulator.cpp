#include "mayaMVG/maya/context/MVGMoveManipulator.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/core/MVGLog.h"

namespace mayaMVG {

MTypeId MVGMoveManipulator::_id(0x99222); // FIXME /!\ 


MVGMoveManipulator::MVGMoveManipulator() :
	_moveState(eMoveNone)
{	
	_manipUtils.intersectionData().pointIndex = -1;
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
	
	view.beginGL();

	// enable gl picking
	// will call manipulator::doPress/doRelease 
	MGLuint glPickableItem;
	glFirstHandle(glPickableItem);
	colorAndName(view, glPickableItem, true, mainColor());
		
	// Preview 3D 
	_manipUtils.drawPreview3D();
	
	// draw	
	MVGDrawUtil::begin2DDrawing(view);
		MVGDrawUtil::drawCircle(0, 0, 1, 5); // needed - FIXME
				
		// Draw Camera points
//		glColor3f(1.f, 0.5f, 0.f);
//		MVGManipulatorUtil::drawCameraPoints(view, data);
		
		if(MVGMayaUtil::isActiveView(view))
		{		
			_manipUtils.drawIntersections(view, data);

			switch(_moveState)
			{
				case eMoveNone:
					break;
				case eMovePoint:
//					glColor3f(0.9f, 0.5f, 0.4f);
//					MVGDrawUtil::drawFullCross(mousex, mousey);
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
	

	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	short mousex, mousey;
	updateMouse(view, data, mousex, mousey);
	
	_manipUtils.updateIntersectionState(view, data, mousex, mousey);
	
	switch(_manipUtils.intersectionState())
	{
		case MVGManipulatorUtil::eIntersectionNone:
			break;
		case MVGManipulatorUtil::eIntersectionPoint:
		{
			// Update face informations
			MVGMesh mesh(_manipUtils.intersectionData().meshName);
			_manipUtils.intersectionData().connectedFace = false;
			_manipUtils.intersectionData().facePointIndexes.clear();
			MIntArray connectedFaceIndex = mesh.getConnectedFacesToVertex(_manipUtils.intersectionData().pointIndex);
			if(connectedFaceIndex.length() > 0)
			{
				_manipUtils.intersectionData().facePointIndexes = mesh.getFaceVertices(connectedFaceIndex[0]);				
				for(int i = 0; i < _manipUtils.intersectionData().facePointIndexes.length(); ++i)
				{
					if(mesh.getNumConnectedFacesToVertex(_manipUtils.intersectionData().facePointIndexes[i]) > 1)
					{
						_manipUtils.intersectionData().connectedFace = true;
						break;
					}
				}
			}
			
			_moveState = eMovePoint;
			break;
		}
		case MVGManipulatorUtil::eIntersectionEdge:
			_moveState = eMoveEdge;
			break;
	}
	
	return MPxManipulatorNode::doPress(view);
}

MStatus MVGMoveManipulator::doRelease(M3dView& view)
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
	
	short mousex, mousey;
	MPoint mousePoint = updateMouse(view, data, mousex, mousey);
	
	switch(_manipUtils.intersectionState()) {
		case MVGManipulatorUtil::eIntersectionNone:
		case MVGManipulatorUtil::eIntersectionPoint:
		{
			MDagPath meshPath;
			MVGMayaUtil::getDagPathByName(_manipUtils.intersectionData().meshName.c_str(), meshPath);
			
			_manipUtils.addUpdateFaceCommand(cmd, meshPath, _manipUtils.previewFace3D(), _manipUtils.intersectionData().facePointIndexes);
			_manipUtils.previewFace3D().clear();
			_manipUtils.clearIntersectionData();
			break;
		}
		case MVGManipulatorUtil::eIntersectionEdge: 
		{
			//LOG_INFO("MOVE TMP EDGE")
			break;
		}
	}


//	switch(_moveState)
//	{
//		case eMoveNone:
//			break;
//		case eMovePoint:
//		{		
//			try {	
//				PairStringToPoint cameraPair = std::make_pair(data->camera.name(), data->cameraPoints2D[_manipUtils.intersectionData().pointIndex]);
//				const PairStringToPoint meshPair = MVGProjectWrapper::instance().getMap2Dto3D().at(cameraPair);
//				
//				MVGProjectWrapper::instance().getMap2Dto3D().erase(cameraPair);		
//				std::vector<PairStringToPoint >& vec = MVGProjectWrapper::instance().getMap3Dto2D().at(meshPair);			
//				vec.erase(std::remove(vec.begin(), vec.end(), cameraPair));
//				
//				cameraPair = std::make_pair(data->camera.name(), mousePoint);
//				MVGProjectWrapper::instance().getMap2Dto3D().insert(std::make_pair(cameraPair, meshPair));
//				vec.push_back(cameraPair);		
//				
//			}
//			catch(const std::exception& e){
//				break;
//			}
//			
//			data->cameraPoints2D[_manipUtils.intersectionData().pointIndex] = mousePoint;
//			data->camera.setClickedtPointAtIndex(_manipUtils.intersectionData().pointIndex, mousePoint);			
//			break;
//		}
//		case eMoveEdge:
//			break;
//	}
//	
//	_moveState = eMoveNone;
	return MPxManipulatorNode::doRelease(view);
}

MStatus MVGMoveManipulator::doMove(M3dView& view, bool& refresh)
{	
	refresh = true;
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;

	short mousex, mousey;
	mousePosition(mousex, mousey);
	// TODO: intersect 2D point (from camera object)
	//       or intersect 2D edge (from camera object)
	//       or intersect 3D point (fetched point from mesh object)
	if(MVGProjectWrapper::instance().getCacheMeshToPointArray().size() > 0)
		_manipUtils.updateIntersectionState(view, data, mousex, mousey);
	return MPxManipulatorNode::doMove(view, refresh);
}

MStatus MVGMoveManipulator::doDrag(M3dView& view)
{		
	DisplayData* data = MVGProjectWrapper::instance().getCachedDisplayData(view);
	if(!data)
		return MS::kFailure;
	
	short mousex, mousey;
	MPoint mousePoint = updateMouse(view, data, mousex, mousey);
	
	switch(_manipUtils.intersectionState()) {
		case MVGManipulatorUtil::eIntersectionNone:
		case MVGManipulatorUtil::eIntersectionPoint:
			if(!_manipUtils.intersectionData().connectedFace)
				computeTmpFaceOnMovePoint(view, data, mousePoint);
			break;
		case MVGManipulatorUtil::eIntersectionEdge: 
		{
			//LOG_INFO("MOVE TMP EDGE")
			break;
		}
	}

	return MPxManipulatorNode::doDrag(view);
}

void MVGMoveManipulator::preDrawUI(const M3dView& view)
{
}

void MVGMoveManipulator::setContext(MVGContext* ctx)
{
	_manipUtils.setContext(ctx);
}

MPoint MVGMoveManipulator::updateMouse(M3dView& view, DisplayData* data, short& mousex, short& mousey)
{
	mousePosition(mousex, mousey);
	MPoint mousePointInCameraCoord;
	MVGGeometryUtil::viewToCamera(view, data->camera, mousex, mousey, mousePointInCameraCoord);
	
	return mousePointInCameraCoord;
}

void MVGMoveManipulator::computeTmpFaceOnMovePoint(M3dView& view, DisplayData* data, MPoint& mousePoint)
{
	MPointArray& meshPoints = MVGProjectWrapper::instance().getMeshPoints(_manipUtils.intersectionData().meshName);

	MIntArray verticesId = _manipUtils.intersectionData().facePointIndexes;

	// Move in plane
//	{
//		// Fill previewFace3D
//		MPointArray facePoints3D;
//		for(int i = 0; i < verticesId.length(); ++i)
//		{
//			facePoints3D.append(meshPoints[verticesId[i]]);
//		}
//		MPoint movedPoint;
//		PlaneKernel::Model model;
//
//		MVGGeometryUtil::computePlane(facePoints3D, model);
//		MVGGeometryUtil::projectPointOnPlane(mousePoint, view, model, data->camera, movedPoint);
//
//		_manipUtils.previewFace3D().setLength(4);
//		for(int i = 0; i < verticesId.length(); ++i)
//		{
//			if(_manipUtils.intersectionData().pointIndex == verticesId[i])
//				_manipUtils.previewFace3D()[i] = movedPoint;
//			else
//				_manipUtils.previewFace3D()[i] = facePoints3D[i];
//		}		
//	}

	// Recompute plane
	{
		MPointArray previewPoints2D;
		MPoint wpos;
		for(int i = 0; i < verticesId.length(); ++i)
		{
			if(_manipUtils.intersectionData().pointIndex == verticesId[i])
			{
				previewPoints2D.append(mousePoint);
			}

			else
			{
				MVGGeometryUtil::worldToCamera(view, data->camera, meshPoints[verticesId[i]], wpos);
				previewPoints2D.append(wpos);
			}
		}
		
		// Compute face		
		_manipUtils.previewFace3D().clear();
		MVGGeometryUtil::projectFace2D(view, _manipUtils.previewFace3D(), data->camera, previewPoints2D);
	}
}

void MVGMoveManipulator::drawUI(MHWRender::MUIDrawManager& drawManager, const MHWRender::MFrameContext&) const
{
	drawManager.beginDrawable();
	drawManager.setColor(MColor(1.0, 0.0, 0.0, 0.6));
	// TODO
	drawManager.endDrawable();
}
}	//mayaMVG