#pragma once

#include "mayaMVG/qt/MVGProjectWrapper.h"
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/M3dView.h>

namespace mayaMVG {

class MVGContext;
class MVGEditCmd;

#define POINT_RADIUS 10

class MVGManipulatorUtil {

	public: 
		enum IntersectionState {
			eIntersectionNone = 0
			, eIntersectionPoint
			, eIntersectionEdge
		};

		struct IntersectionData {
			std::string meshName;
			// Index of the intersected point
			int	pointIndex;
			// Indexes of the points of the intersected edge
			MIntArray edgePointIndexes;
			// Heights (2D and 3D) and ratio of the intersected edge
			MVector edgeHeight3D;
			MVector edgeHeight2D;
			double edgeRatio;
			// Indexes of the points of the first face connected to intersected point or edge
			MIntArray facePointIndexes;

		};

	public: 
		MVGManipulatorUtil();
	
	public:
		// Getters & Setters
		IntersectionState& intersectionState() { return _intersectionState; }		
		IntersectionData& intersectionData() { return _intersectionData; }
		MPointArray& previewFace3D() { return _previewFace3D; }
		
		const MVGContext* getContext() const { return _ctx; }
		void setContext(MVGContext* context) { _ctx = context; }
		
		// Intersections
		bool intersectPoint(M3dView& view, DisplayData* displayData, const short&x, const short& y);
		bool intersectEdge(M3dView& view, DisplayData* displayData, const short&x, const short& y);
		void updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey);
        bool computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord);
		
		// Draw
		void drawPreview3D();
		
		// Commands
		bool addCreateFaceCommand(MVGEditCmd* cmd, MDagPath& meshPath, const MPointArray& facePoints3D);
		bool addUpdateFaceCommand(MVGEditCmd* cmd, MDagPath& meshPath, const MPointArray& newFacePoints3D, const MIntArray& verticesIndexes);
		
	private:
		MVGContext* _ctx;
		IntersectionState _intersectionState;
		IntersectionData _intersectionData;		
		MPointArray _previewFace3D;
};

} // mayaMVG
