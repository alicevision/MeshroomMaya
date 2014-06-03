#pragma once

#include "mayaMVG/core/MVGNodeWrapper.h"

#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

namespace mayaMVG {

class MVGFace3D;

class MVGMesh : public MVGNodeWrapper  {

	public:
		MVGMesh(const std::string& name);
		MVGMesh(const MDagPath& dagPath);
		virtual ~MVGMesh();

	public:
		virtual bool isValid() const;
		
	public:
		static MVGMesh create(const std::string& name);

	public:
		void addPolygon(const MVGFace3D& face3d) const;
		void deleteFace(const int index) const;
		void getPoints(MPointArray& pointArray) const;
		int getVerticesCount() const;
		bool intersect(MPoint& point, MVector& dir, MPointArray&points) const;
		int getNumConnectedFacesToVertex(int vertexId);
		MIntArray getConnectedFacesToVertex(int vertexId);
		MIntArray getFaceVertices(int faceId);
		void setPoint(int vertexId, MPoint& point);
};

} // mayaMVG
