#pragma once

#include "mayaMVG/core/MVGCamera.h"
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/M3dView.h>
#include <map>

namespace mayaMVG {

#define POINT_RADIUS 10

class MVGContext; // forward declaration

class MVGManipulatorUtil {

	public:

		// on press, move, release, drag
		enum IntersectionState {
			eIntersectionNone = 0
			, eIntersectionPoint
			, eIntersectionEdge
		};

		// on press
		struct IntersectionData {
			std::string meshName;
			int	pointIndex; // Index of the intersected point
			MIntArray edgePointIndexes; // Indexes of the points of the intersected edge
			MVector edgeHeight3D; // Height (2D) of the intersected edge
			MVector edgeHeight2D; // Height (3D) of the intersected edge
			double edgeRatio; // Ratio of the intersected edge
			MIntArray facePointIndexes; // Indexes of the points of the first face connected to intersected point or edge
		};

		enum EPointState {
			eUnMovable = 0
			, eMovableInSamePlane = 1
			, eMovableRecompute = 2
		};

		struct MVGPoint2D {
			MPoint projectedPoint3D; // Position in camera coord of the projected associated point 3D
			MPoint point3D; // Position 3D
			EPointState movableState; // How the point is movable
		};

		// per camera
		struct DisplayData {
			MVGCamera camera; // needed?
			MPointArray buildPoints2D; // Temporary points in Camera before having 3D information
			std::map<std::string, std::vector<MVGPoint2D> > allPoints2D; // Map mesh to MVGPoints2D
		};

	public:
		MVGManipulatorUtil(MVGContext*);

	public:
		// getters & setters
		IntersectionState& intersectionState() { return _intersectionState; }
		IntersectionData& intersectionData() { return _intersectionData; }
		MPointArray& previewFace3D() { return _previewFace3D; }

		// intersections
		bool intersectPoint(M3dView& view, DisplayData* displayData, const short&x, const short& y);
		bool intersectEdge(M3dView& view, DisplayData* displayData, const short&x, const short& y);
		void updateIntersectionState(M3dView& view, DisplayData* data, double mousex, double mousey);
        bool computeEdgeIntersectionData(M3dView& view, DisplayData* data, const MPoint& mousePointInCameraCoord);

		// draw
		void drawPreview3D();

		// commands
		bool addCreateFaceCommand(const MDagPath& meshPath, const MPointArray& facePoints3D);
		bool addUpdateFaceCommand(const MDagPath& meshPath, const MPointArray& newFacePoints3D, const MIntArray& verticesIndexes);

		// cache
		MStatus rebuildAllMeshesCacheFromMaya(); // Temporary
		MStatus rebuildMeshCacheFromMaya(const MDagPath& meshPath); // Temporary
		void rebuild();
		DisplayData* getDisplayData(M3dView& view);
		DisplayData* getComplementaryDisplayData(M3dView& view);
		int getCacheCount() const { return _cacheCameraToDisplayData.size(); }

		// Reset
		void resetCache();
		void resetMeshCache();
		void resetTemporaryData();
		void resetIntersections();

	private:
		MVGContext* _context;
		IntersectionState _intersectionState;
		IntersectionData _intersectionData;
		MPointArray _previewFace3D;
		std::map<std::string, DisplayData> _cacheCameraToDisplayData;
		std::map<std::string, MPointArray> _cacheMeshToPointArray; // Map from meshName to mesh points (Temporary)
		std::map<std::string, std::vector<EPointState> > _cacheMeshToMovablePoint; // Map from meshName to numConnectedFace by point (Temporary)
		std::map<std::string, std::vector<MIntArray> > _cacheMeshToEdgeArray; // Map from meshName to edge points ID (Temporary)
};

} // mayaMVG
