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

		// viewport 2.0 manipulator draw overrides
		virtual void preDrawUI(const M3dView&);
		virtual void drawUI(MHWRender::MUIDrawManager&,	const MHWRender::MFrameContext&) const;

		MVGCamera getMVGCamera() const;
		MVGCamera getMVGCamera(M3dView&);
		
		void createFace3d(M3dView& view);
		void previewFace3d(M3dView& view, std::vector<MPoint>& pointArray, MVGFace3D& previewPoints3d);
				
	public:
		static MTypeId _id;
		MPoint _mousePoint;
		MPoint _lastPoint;
		/// 2D points for display (clicked points + others for display)
		/// Warning: converted in 3D world space to be independant from scale and offset.
		///          It's just a shortcut to rely on maya functions (viewToWorld)
		static std::vector<MPoint> _display2DPoints_world;
		MVGFace3D	_preview3DFace;  //< 3D points (4 points to describe the face)
		static MDagPath _lastCameraPath;
		static MVGCamera _camera;  // TODO: remove static
		bool _drawEnabled;
		static bool _connectFace;
		static bool _computeLastPoint;
		static bool _isNewShape;
		bool _doIntersectExistingPoint;
		bool _doIntersectExistingEdge;
		MPointArray _intersectingEdgePoints;
};

}
