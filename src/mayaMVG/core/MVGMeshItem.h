#pragma once

#include <maya/MPoint.h>
#include <maya/MColor.h>

namespace mayaMVG {

class MVGMeshItem {

	public:
		MVGMeshItem();
		virtual ~MVGMeshItem();

	private:
		MVGMeshItem(const MVGMeshItem&);

	public:
		MPoint point() const;
		MColor color() const;

	private:
		MPoint _point;
		MColor _color;
		// xxx _hash;
		// std::vector<MVGCamera> _visibleThrough;

};

} // mayaMVG
