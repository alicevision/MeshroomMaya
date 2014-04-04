#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGCamera.h"
#include <vector>

using namespace mayaMVG;

bool MVGGeometryUtil::projectFace2D(MVGFace3D& face3D, MVGPointCloud& pointCloud, MVGCamera& camera, MVGFace2D& face2D)
{
	std::vector<MVGPointCloudItem> items;
	pointCloud.getItemsFromProjection(items, camera, face2D);
	if(items.size() < 3)
		return false;
	// Ransac
	// ...
	return false;
}
