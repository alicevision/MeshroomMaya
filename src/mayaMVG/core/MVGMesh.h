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
		bool addPolygon(const MPointArray& pointArray, int& index) const;
		bool deletePolygon(const int index) const;
		void getPoints(MPointArray& pointArray) const;
		int getVerticesCount() const;
		bool intersect(MPoint& point, MVector& dir, MPointArray&points) const;
		int getNumConnectedFacesToVertex(int vertexId);
		int getNumConnectedFacesToEdge(int edgeId);
		MIntArray getConnectedFacesToVertex(int vertexId);
		int getConnectedFacesToEdge(MIntArray& facesId, int edgeId);
		MIntArray getFaceVertices(int faceId);
		MIntArray getEdgeVertices(int edgeId);
		void setPoint(int vertexId, MPoint& point);
};

} // mayaMVG
