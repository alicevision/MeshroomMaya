#pragma once

#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include <maya/MPxManipulatorNode.h>

namespace mayaMVG {

class MVGEditCmd;
class MVGContext;

class MVGCreateManipulator: public MPxManipulatorNode
{
    enum ECreateState {
        eCreateNone = 0
        , eCreateExtend
    };
    
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
		MPoint updateMouse(M3dView& view, DisplayData* data, short& mousex, short& mousey);

		// Draw
		void drawCursor(const float mousex, const float mousey);
		void drawIntersections(M3dView& view, const float mousex, const float mousey);
		void drawPreview2D(M3dView& view, const DisplayData* data);
        void drawOtherPreview2D(M3dView& view, const DisplayData* data);

		// Compute
		void computeTmpFaceOnEdgeExtend(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord);

        // Command
        bool addCreateFaceCommand(MVGEditCmd* cmd, MDagPath& meshPath, const MPointArray& facePoints3D) const;

	public:
		static MTypeId _id;
        MVGContext* _ctx;
		MVGManipulatorUtil _manipUtils;
        ECreateState _createState;

        MVector _createColor;
        MVector _neutralCreateColor;
        MVector _extendColor;
        MVector _faceColor;
        MVector _cursorColor;
};
} // namespace
