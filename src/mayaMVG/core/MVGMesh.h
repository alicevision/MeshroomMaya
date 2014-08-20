#pragma once

#include "mayaMVG/core/MVGNodeWrapper.h"
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <vector>

namespace mayaMVG {

class MVGMesh : public MVGNodeWrapper  {

	public:
		MVGMesh(const std::string& name);
		MVGMesh(const MString& name);
		MVGMesh(const MDagPath& dagPath);
		virtual ~MVGMesh();

	public:
		virtual bool isValid() const;
		
	public:
		static MVGMesh create(const std::string& name);
		static std::vector<MVGMesh> list();

	public:
		bool addPolygon(const MPointArray& pointArray, int& index) const;
		bool deletePolygon(const int index) const;
		MStatus getPoints(MPointArray& pointArray) const;
        int getPolygonsCount() const;
		const MIntArray getConnectedFacesToVertex(int vertexId) const;
		const MIntArray getFaceVertices(int faceId) const;
		MStatus getPoint(int vertexId, MPoint& point) const;
		MStatus setPoint(int vertexId, MPoint& point) const;
};

} // namespace
