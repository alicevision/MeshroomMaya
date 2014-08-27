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
		int _id;
		MPoint _position;
		MVector _color;
		float _weight;
};

} // namespace
