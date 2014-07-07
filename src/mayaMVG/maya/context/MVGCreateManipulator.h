#pragma once

#include <maya/MPxManipulatorNode.h>
#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
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
		void updateIntersectionState(M3dView& view, MVGManipulatorUtil::DisplayData* data, double mousex, double mousey);		
		void drawPreview2D(M3dView& view, MVGManipulatorUtil::DisplayData* data);
		
		bool addCreateFaceCommand(M3dView& view, MVGManipulatorUtil::DisplayData* data, MVGEditCmd* cmd, MPointArray& facePoints3D);

	public:
		static MTypeId _id;
		std::map<std::string, MVGManipulatorUtil::DisplayData> _cache; //FIXME use caching on the wrapper side
		std::map<std::pair<std::string, MPoint>, std::pair<std::string, MPoint> >_pointsMap;
		MVGManipulatorUtil::IntersectionState _intersectionState;
        MVGContext* _ctx;
		MVGManipulatorUtil::IntersectionData _intersectionData;		
		
		MPointArray _previewFace3D;
};



} // namespace

namespace std {
	typedef std::pair<std::string, MPoint> cameraPair;
	inline bool operator<(const cameraPair& pair_a, const cameraPair& pair_b) { 
		
		if(pair_a.first != pair_b.first)
			return (pair_a.first < pair_b.first);
		
		if(pair_a.second.x != pair_b.second.x)
			return pair_a.second.x < pair_b.second.x;
		
		return pair_a.second.y < pair_b.second.y;
	}
}