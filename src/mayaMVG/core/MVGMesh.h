#pragma once

#include "mayaMVG/core/MVGNodeWrapper.h"
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>

namespace mayaMVG {

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
		bool intersect(MPoint& point, MVector& dir, MPointArray& points) const;
		int getNumConnectedFacesToVertex(int vertexId) const;
		int getNumConnectedFacesToEdge(int edgeId) const;
		const MIntArray getConnectedFacesToVertex(int vertexId) const;
		int getConnectedFacesToEdge(MIntArray& facesId, int edgeId) const;
		const MIntArray getFaceVertices(int faceId) const;
		const MIntArray getEdgeVertices(int edgeId) const;
		MStatus getPoint(int vertexId, MPoint& point) const;
		MStatus setPoint(int vertexId, MPoint& point) const;
};

} // namespace
