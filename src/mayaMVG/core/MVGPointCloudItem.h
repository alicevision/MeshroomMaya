#pragma once

#include <maya/MPoint.h>
#include <maya/MColor.h>
#include <vector>

namespace mayaMVG {

class MVGPointCloudItem {

	public:
		MVGPointCloudItem();
		virtual ~MVGPointCloudItem();

	public:
		MPoint _position;
		MColor _color;
		std::vector<size_t> _visibility;
		float _weight;
		// xxx _sift;
};

} // mayaMVG
