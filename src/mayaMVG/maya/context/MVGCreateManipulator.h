#pragma once

#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include <maya/MPxManipulatorNode.h>
#include "mayaMVG/core/MVGMesh.h"
#include "mayaMVG/core/MVGCamera.h"
#include <map>
#include <utility>

namespace mayaMVG {

class MVGContext;
class MVGEditCmd;

class MVGCreateManipulator: public MPxManipulatorNode
{
	public:
		MVGCreateManipulator();
		virtual ~MVGCreateManipulator();
		
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
		virtual void preDrawUI(const M3dView&);
		virtual void drawUI(MHWRender::MUIDrawManager&,	const MHWRender::MFrameContext&) const;

	public:
		void setContext(MVGContext* ctx);
	private:	
		void updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey);		
		void drawPreview2D(M3dView& view, DisplayData* data);
		void drawPreview3D();
		void computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord);
		void computeTmpFaceOnEdgeExtend(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord);
		
		bool addCreateFaceCommand(M3dView& view, DisplayData* data, MVGEditCmd* cmd, MDagPath& meshPath, const MPointArray& facePoints3D);

	public:
		static MTypeId _id;
//		std::map<std::string, DisplayData> _cache; //FIXME use caching on the wrapper side
		MVGManipulatorUtil::IntersectionState _intersectionState;
        MVGContext* _ctx;
		MVGManipulatorUtil::IntersectionData _intersectionData;		
		
		MPointArray _previewFace3D;
};



} // namespace
