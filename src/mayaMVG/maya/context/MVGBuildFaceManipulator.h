#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGGeometryUtil.h"
#include <maya/MPxManipulatorNode.h>
#include <maya/MDagPath.h>
#include <maya/MPointArray.h>
#include <vector>

class M3dView;

namespace mayaMVG {


class MVGBuildFaceManipulator: public MPxManipulatorNode
{
	public:
		enum EMode {
			eModeCreate,
			eModeMoveInPlane,
			eModeMoveRecompute
		};

		enum EEditAction
		{
			eEditActionNone,
			eEditActionMovePoint,
			eEditActionMoveEdge,
			eEditActionExtendEdge
		};
		
	public:
		MVGBuildFaceManipulator();
		virtual ~MVGBuildFaceManipulator();
		
	public:
		static void * creator();
		static MStatus initialize();

	public:
		virtual void postConstructor();
		virtual void draw(M3dView&, const MDagPath&, M3dView::DisplayStyle, M3dView::DisplayStatus);
		virtual MStatus doPress(M3dView &view);
		virtual MStatus doRelease(M3dView &view);
		virtual MStatus doMove(M3dView &view, bool& refresh);
		virtual MStatus doDrag(M3dView& view);
		
		// viewport 2.0 manipulator draw overrides
		virtual void preDrawUI(const M3dView&);
		virtual void drawUI(MHWRender::MUIDrawManager&,	const MHWRender::MFrameContext&) const;

		MVGCamera getMVGCamera() const;
		MVGCamera getMVGCamera(M3dView&);
	
	private:
		void updateCamera(M3dView& view);
		void updateMouse(M3dView& view);
		
		bool intersectPoint(M3dView& view, MPoint& point);
		bool intersectEdge(M3dView& view, MPoint& point);
		
		void update2DFacePreview(M3dView& view);
		bool update3DFacePreview(M3dView& view);
		bool computeFace3d(M3dView& view, std::vector<MPoint>& pointArray, MVGFace3D& face3D, bool computeLastPoint = false, MVector height = MVector(0, 0, 0));
		void addFace3d(MVGFace3D& face3d);
		

	public:
		static MTypeId _id;
		static MDagPath _lastCameraPath; // TODO: to remove, use _camera
		static MVGCamera _camera;  // TODO: remove static
		MDagPath	_cameraPathClickedPoints;
		bool _drawEnabled;
		
		///@brief Mouse information
		///@{
		MPoint _mousePoint; // TODO: to remove ?
		MPoint	_mousePointOnPressEdge;
		MPoint	_mousePointOnDragEdge;
		///@}
		
		///@brief Mode & action
		///@{
		static EMode	_mode;
		static EEditAction _editAction;
		///@}
		
		///@brief Display & Preview
		///@{
		/// 2D points for display (clicked points + others for display)
		/// Warning: converted in 3D world space to be independant from scale and offset.
		///          It's just a shortcut to rely on maya functions (viewToWorld)
		static std::vector<MPoint> _display2DPoints_world;
		MVGFace3D	_preview3DFace;  //< 3D points (4 points to describe the face)
		MVGFace3D	_preview2DFace;  //< 2D points (4 points to describe the face)
		std::vector<GLfloat> _drawColor;
		///@}
		
		///@brief Intersections
		///@{
		int	_pressedPointId;
		int	_intersectedEdgeId;
		MPointArray _intersectingEdgePoints3D;
		MPointArray _clickedEdgePoints3D;	
		MVector	_edgeHeight3D;
		MVector	_edgeHeight2D;
		float	_edgeRatio;
		///@}
		
		///@brief Status when moving point/
		///@{
		MIntArray _connectedFacesId;
		bool _edgeConnected;
		bool _faceConnected;
		///@}
		
		
		
		
};

}
