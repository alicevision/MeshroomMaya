#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include <openMVG/robust_estimation/robust_estimator_LMeds.hpp>
#include <openMVG/multiview/triangulation_nview.hpp>
#include <openMVG/multiview/triangulation.hpp>
#include <openMVG/multiview/projection.hpp>
#include <maya/MPointArray.h>
#include <maya/MVectorArray.h>
#include <maya/MPlug.h>
#include <maya/MFnDagNode.h>
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

void MVGGeometryUtil::viewToCamera(M3dView& view, const short x, const short y, MPoint& point)
{
	MDagPath dagPath;
	view.getCamera(dagPath);
    MVGCamera camera(dagPath);
	point.x = ((float)x / view.portWidth()) - 0.5;
	point.y = ((float)y / view.portWidth()) - 0.5 - 0.5 * ((view.portHeight() / (float)view.portWidth()) - 1.0 );
	point.z = 0.f;
	// zoom  
	point =  point * camera.getHorizontalFilmAperture() * camera.getZoom();
	// pan
	point.x += camera.getHorizontalPan();
	point.y += camera.getVerticalPan();
}

 void MVGGeometryUtil::worldToCamera(M3dView& view, const MPoint& worldPoint, MPoint& point)
{
	short x, y;
	view.worldToView(worldPoint, x, y);
	viewToCamera(view, x, y, point);
}

void MVGGeometryUtil::cameraToView(M3dView& view, const MPoint& point, short& x, short& y)
{
	MDagPath dagPath;
	view.getCamera(dagPath);
	MVGCamera camera(dagPath);
	float newX = point.x;
	float newY = point.y;
	// pan
	newX -= camera.getHorizontalPan();
	newY -= camera.getVerticalPan();
	// zoom
	newX /= (camera.getHorizontalFilmAperture() * camera.getZoom());
	newY /= (camera.getHorizontalFilmAperture() * camera.getZoom());
	// center	
	x = round((newX + 0.5) * view.portWidth());
	y = round((newY + 0.5 + 0.5 * (view.portHeight() / (float)view.portWidth() - 1.0)) * view.portWidth());	
}

void MVGGeometryUtil::cameraToImage(const MVGCamera& camera, const MPoint& point, short& x, short& y)
{	
	// Get image size 
	MFnDagNode fnImage(camera.imagePath());
	const double width = fnImage.findPlug("coverageX").asDouble();
	const double height = fnImage.findPlug("coverageY").asDouble();

	MPoint pointCenteredNorm = point / camera.getHorizontalFilmAperture();
	
	const double verticalMargin = (width - height) / 2.0;
	x = (pointCenteredNorm.x + 0.5 ) * width;
	y = (-pointCenteredNorm.y + 0.5) * width - verticalMargin;
}

bool MVGGeometryUtil::projectFace2D(M3dView& view, MPointArray& face3DPoints, const MVGCamera& camera, const MPointArray& face2DPoints, bool compute, MVector height)
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
		cameraToView(view, face2DPoints[i], x, y);
		facePoints_view.push_back(MPoint(x, y));
	}
	cameraToView(view, face2DPoints[0], x, y);	// Extra point
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

bool MVGGeometryUtil::computePlane(MPointArray& facePoints3D, PlaneKernel::Model& model)
{
	if(facePoints3D.length() < 3)
	{
		LOG_ERROR("Need at least 3 points to compute a plane")
		return false;
	}
	openMVG::Mat facePointsMat(3, 4);
	for (size_t i = 0; i <facePoints3D.length(); ++i)
		facePointsMat.col(i) = AS_VEC3(facePoints3D[i]);
	PlaneKernel kernel(facePointsMat);
	double outlierThreshold = std::numeric_limits<double>::infinity();
	openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);
	
	return true;
}

void MVGGeometryUtil::projectPointOnPlane(const MPoint& point, M3dView& view, PlaneKernel::Model& model, const MVGCamera& camera, MPoint& projectedPoint)
{
	MPoint cameraCenter = AS_MPOINT(camera.pinholeCamera()._C);
	short x, y;
	MVector dir;
	cameraToView(view, point, x, y);
	MPoint worldPoint;
	view.viewToWorld(x, y, worldPoint, dir);
	plane_line_intersect(model, cameraCenter, worldPoint, projectedPoint);
}

void MVGGeometryUtil::triangulatePoint(MPointArray& points2D, std::vector<MVGCamera>& cameras, MPoint& resultPoint3D)
{
	// Result
	openMVG::Vec3 result;
	short x, y;
	
	// N views
//	openMVG::Mat2X A(2, points2D.length());
//	
//	for(size_t i = 0; i <points2D.length(); ++i)
//	{
//		MVGGeometryUtil::cameraToImage(cameras[i], points2D[i], x, y);
//		A(0, i) = points2D[i].x;
//		A(1, i) = points2D[i].y;
//	}
	
	// Projective cameras
//	std::vector<openMVG::Mat34> projectiveCameras;
//	for(size_t i = 0; i <cameras.size(); ++i)
//	{
//		projectiveCameras.push_back(cameras[i].pinholeCamera()._P);
//	}
	
	/// Compute a 3D position of a point from several images of it. In particular,
	///  compute the projective point X in R^4 such that x = PX.
	/// Algorithm is the standard DLT; for derivation see appendix of Keir's thesis.
	//openMVG::TriangulateNView(A, projectiveCameras, &result4);
	
	// 2 views
	openMVG::Vec2 xL_;
	MVGGeometryUtil::cameraToImage(cameras[0], points2D[0], x, y);
	xL_(0) = x;
	xL_(1) = y;
	openMVG::Vec2 xR_;
	MVGGeometryUtil::cameraToImage(cameras[1], points2D[1], x, y);
	xR_(0) = x;
	xR_(1) = y;
	
	const openMVG::Mat34 PL = cameras[0].pinholeCamera()._P;
	const openMVG::Mat34 PR = cameras[1].pinholeCamera()._P;
		
	openMVG::TriangulateDLT(PL, xL_, PR, xR_, &result);
	
	resultPoint3D.x = result(0);
	resultPoint3D.y = result(1);
	resultPoint3D.z = result(2);
}

double MVGGeometryUtil::crossProduct2D(MVector& A, MVector& B) {
    return A.x*B.y - A.y*B.x;
}

double MVGGeometryUtil::dotProduct2D(MVector& A, MVector& B) {
    return A.x*B.x - A.y*B.y;
}

bool MVGGeometryUtil::edgesIntersection(MPoint A, MPoint B, MVector AD,  MVector BC)
{		
    // r x s = 0
    double cross = crossProduct2D(AD, BC);
    double eps = 0.00001;
    if(cross < eps && cross > -eps)
        return false;

    MVector AB = B - A;

    double x =  crossProduct2D(AB, BC) / crossProduct2D(AD, BC);
    double y = crossProduct2D(AB, AD) / crossProduct2D(AD, BC);

    if( x >= 0 
        && x <= 1 
        && y >= 0 
        && y <= 1)
    {
        return true;
    }
    return false;
}
