#pragma once

#include <maya/MPoint.h>
#include <maya/MDagPath.h>
#include <vector>

namespace mayaMVG {

class MVGFace3D;

class MVGMesh {

	public:
		MVGMesh(const std::string& name);
		MVGMesh(const MDagPath& dagPath);
		virtual ~MVGMesh();

	public:
		static MVGMesh create(const std::string& name);

	public:
		void setName(const std::string&);
		const std::string name() const;
		void add3DPoint(const MPoint&);
		void move3DPoint(const MPoint&);
		void addPolygon(const MVGFace3D& face3d);

	private:
		MDagPath _dagpath;
		std::vector<MPoint> _points;
};

} // mayaMVG
