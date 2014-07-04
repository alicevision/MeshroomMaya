#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include <openMVG/robust_estimation/robust_estimator_LMeds.hpp>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MFnCamera.h>
#include <vector>

using namespace mayaMVG;

namespace { // empty namespace

	// isLeft(): tests if a point is Left|On|Right of an infinite line.
	//    Input:  three points P0, P1, and P2
	//    Return: >0 for P2 left of the line through P0 and P1
	//            =0 for P2  on the line
	//            <0 for P2  right of the line
	//    See: Algorithm 1 "Area of Triangles and Polygons"
	int isLeft(MPoint P0, MPoint P1, MPoint P2)
	{
		return ((P1.x-P0.x)*(P2.y-P0.y)-(P2.x-P0.x)*(P1.y-P0.y));
	}

	// wn_PnPoly(): winding number test for a point in a polygon
	//      Input:   P = a point,
	//               V[] = vertex points of a polygon V[n+1] with V[n]=V[0]
	//      Return:  wn = the winding number (=0 only when P is outside)
	int wn_PnPoly(MPoint P, std::vector<MPoint> V)
	{
		int wn = 0;
		for(int i=0; i<V.size()-1; i++) {
			if(V[i].y <= P.y) {
				if (V[i+1].y  > P.y)
					if (isLeft(V[i], V[i+1], P) > 0)
						++wn;
			}
			else {
				if(V[i+1].y <= P.y)
					if(isLeft(V[i], V[i+1], P) < 0)
						--wn;
			}
		}
		return wn;
	}

	bool plane_line_intersect(const PlaneKernel::Model& model, 
									const MPoint& P1, 
									const MPoint& P2, 
									MPoint& P)
	{
		double u = (model(0)*P1.x+model(1)*P1.y+model(2)*P1.z+model(3))
						/ (model(0)*(P1.x-P2.x)+model(1)*(P1.y-P2.y)+model(2)*(P1.z-P2.z));
		P = P1 + u * (P2-P1);
		return (0<u && u<1);
	}

} // empty namespace


MPoint MVGGeometryUtil::worldToView(M3dView& view, const MPoint& world)
{
	short x, y;
	view.worldToView(world, x, y);
	return MPoint(x, y);
}

MPoint MVGGeometryUtil::viewToWorld(M3dView& view, const MPoint& screen)
{
	MPoint wpoint, wdir;
	view.viewToWorld(screen.x, screen.y, wpoint, wdir);
	return wpoint;
}

void MVGGeometryUtil::viewToCamera(M3dView& view, const MVGCamera& camera, const short& x, const short& y, MPoint& point)
{
	MFnCamera fnCamera(camera.dagPath().node()); 
	point.x = ((float)x / view.portWidth()) - 0.5;
	point.y = ((float)y / view.portWidth()) - 0.5 - 0.5 * ((view.portHeight() / (float)view.portWidth()) - 1.0 );
	point.z = 0.f;
	// zoom
	point =  point * fnCamera.horizontalFilmAperture() * fnCamera.zoom();
	// pan
	point.x += fnCamera.horizontalPan();
	point.y += fnCamera.verticalPan();
}

void MVGGeometryUtil::worldToCamera(M3dView& view, MVGCamera& camera, MPoint& worldPoint, MPoint& point)
{
	short x, y;
	view.worldToView(worldPoint, x, y);
	viewToCamera(view, camera,  x, y, point);
}

void MVGGeometryUtil::cameraToView(M3dView& view, MVGCamera& camera, MPoint& point, short& x, short& y)
{
	MFnCamera fnCamera(camera.dagPath().node()); 
	float newX = point.x;
	float newY = point.y;
	// pan
	newX -= fnCamera.horizontalPan();
	newY -= fnCamera.verticalPan();
	// zoom
	newX /= (fnCamera.horizontalFilmAperture() * fnCamera.zoom());
	newY /= (fnCamera.horizontalFilmAperture() * fnCamera.zoom());
	// center	
	x = round((newX + 0.5) * view.portWidth());
	y = round((newY + 0.5 + 0.5 * (view.portHeight() / (float)view.portWidth() - 1.0)) * view.portWidth());	
}

bool MVGGeometryUtil::projectFace2D(M3dView& view, MPointArray& face3DPoints, MVGCamera& camera, MPointArray& face2DPoints, bool compute, MVector height)
{
	std::vector<MVGPointCloudItem> items = camera.visibleItems();
	if(items.size() < 3) {
		LOG_ERROR("Need more than " << items.size() << " point cloud items. Abort.");
		return false;
	}
	
	std::vector<MVGPointCloudItem>::const_iterator it = items.begin();
	std::vector<MPoint> facePoints_view;
	
	// Camera -> View
	short x, y;
	for(size_t i = 0; i < face2DPoints.length(); ++i)
	{
		cameraToView(view, camera, face2DPoints[i], x, y);
		facePoints_view.push_back(MPoint(x, y));
	}
	cameraToView(view, camera, face2DPoints[0], x, y);	// Extra point
	facePoints_view.push_back(MPoint(x, y));
	
	// Get selected points in pointCloud
	std::vector<MVGPointCloudItem> selectedItems;
	int windingNumber = 0;
	for(; it != items.end(); ++it){
		windingNumber = wn_PnPoly(worldToView(view, it->_position), facePoints_view);
		if(windingNumber != 0)
			selectedItems.push_back(*it);
	}
	if(selectedItems.size() < 3) {
		LOG_ERROR("Need more than " << selectedItems.size() << " selected points. Abort.");
		return false;
	}

	// 3D plane estimation
	// w/ a variant of RANSAC using Least Median of Squares
	openMVG::Mat selectedItemsMat(3, selectedItems.size());
	for (size_t i = 0; i < selectedItems.size(); ++i)
		selectedItemsMat.col(i) = AS_VEC3(selectedItems[i]._position);

	PlaneKernel kernel(selectedItemsMat);
	PlaneKernel::Model model;
	double outlierThreshold = std::numeric_limits<double>::infinity();
	double dBestMedian = openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);

	// Retrieve Face3D vertices from this model
	MPoint P;
	MPoint cameraCenter = AS_MPOINT(camera.pinholeCamera()._C);
	//std::vector<MPoint> face3DPoints;
	MPoint worldPoint;
	
	if(compute) {	
		// Three first points are retrieved from the selection points
		for(size_t i = 0; i < facePoints_view.size()-2; ++i) // remove extra point
		{
			worldPoint = viewToWorld(view, facePoints_view[i]);
			plane_line_intersect(model, cameraCenter, worldPoint, P);
			face3DPoints.append(P);
		}
		// Compute last point to keep 3D lenghts
		if(height.length() == 0)
		{
			height = face3DPoints[0]- face3DPoints[1];
		}			
		MPoint lastPoint3D = face3DPoints[2] + height; // TODO : warning
		MPoint viewPoint = worldToView(view, lastPoint3D);
		worldPoint = viewToWorld(view, viewPoint);
		plane_line_intersect(model, cameraCenter, worldPoint, P);
		face3DPoints.append(P);
	}
	else {
		for(size_t i = 0; i < facePoints_view.size()-1; ++i) // remove extra point
		{
			worldPoint = viewToWorld(view, facePoints_view[i]);
			plane_line_intersect(model, cameraCenter, worldPoint, P);
			face3DPoints.append(P);
		}
	}
	
	return true;
}
//
//void MVGGeometryUtil::computePlane(MVGFace3D& face3D, PlaneKernel::Model& model)
//{
//	openMVG::Mat facePointsMat(3, 4);
//	for (size_t i = 0; i <4; ++i)
//		facePointsMat.col(i) = AS_VEC3(face3D._p[i]);
//	PlaneKernel kernel(facePointsMat);
//	double outlierThreshold = std::numeric_limits<double>::infinity();
//	openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);
//}

void MVGGeometryUtil::projectPointOnPlane(MPoint& point, M3dView& view, PlaneKernel::Model& model, MVGCamera& camera, MPoint& projectedPoint)
{
	MPoint cameraCenter = AS_MPOINT(camera.pinholeCamera()._C);
	short x, y;
	MVector dir;
	cameraToView(view, camera, point, x, y);
	view.viewToWorld(x, y, point, dir);
	plane_line_intersect(model, cameraCenter, point, projectedPoint);
}
