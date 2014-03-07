#pragma once

#include <maya/MPoint.h>
#include <maya/MColor.h>

namespace mayaMVG {

class MVGPointCloudItem {

	public:
		MVGPointCloudItem();
		virtual ~MVGPointCloudItem();

	private:
		MVGPointCloudItem(const MVGPointCloudItem&);

	public:
		MPoint point() const;
		MColor color() const;

	private:
		MPoint _point;
		MColor _color;
		// xxx _sift;

};

} // mayaMVG
