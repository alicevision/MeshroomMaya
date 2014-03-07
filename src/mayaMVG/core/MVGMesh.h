#pragma once

#include <maya/MPoint.h>
#include <maya/MDagPath.h>
#include <vector>

namespace mayaMVG {

class MVGMesh {

	public:
		MVGMesh();
		virtual ~MVGMesh();

	public:
		void add3DPoint(const MPoint&);
		void move3DPoint(const MPoint&);

	private:
		MDagPath _dagpath;
		std::vector<MPoint> _points;

};

} // mayaMVG
