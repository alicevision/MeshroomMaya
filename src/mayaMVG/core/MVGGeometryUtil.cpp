#include "mayaMVG/core/MVGGeometryUtil.h"
#include "mayaMVG/core/MVGPointCloud.h"
#include "mayaMVG/core/MVGPointCloudItem.h"
#include "mayaMVG/core/MVGProject.h"
#include "mayaMVG/core/MVGCamera.h"
#include "mayaMVG/core/MVGLog.h"
#include "openMVG/robust_estimation/robust_estimator_LMeds.hpp"
#include "openMVG/multiview/triangulation.hpp"
#include <maya/MPointArray.h>
#include <maya/M3dView.h>
#include <maya/MPlug.h>
#include <maya/MFnDagNode.h>
#include <maya/MMatrix.h>

namespace mayaMVG {

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
	// Don't use view.worldToView() because of the short values
    MStatus status;
	MMatrix modelViewMatrix, projectionMatrix;
	status = view.modelViewMatrix(modelViewMatrix);
	status = view.projectionMatrix(projectionMatrix);
	CHECK(status)
	MPoint point = world*(modelViewMatrix*projectionMatrix);
	
	unsigned int viewportX, viewportY, viewportWidth, viewportHeight;
	view.viewport(viewportX, viewportY, viewportWidth, viewportHeight);
	
	MPoint viewPoint;
	viewPoint.x =static_cast<int>(static_cast<double>(viewportWidth) * (point.x / point.w + 1.0) / 2.0);
	viewPoint.y =static_cast<int>(static_cast<double>(viewportHeight) * (point.y / point.w + 1.0) / 2.0);

	return viewPoint;
}

MPoint MVGGeometryUtil::viewToWorld(M3dView& view, const MPoint& screen)
{
    MStatus status;
	MPoint wpoint, wdir;
	status = view.viewToWorld(screen.x, screen.y, wpoint, wdir);
    CHECK(status)
	return wpoint;
}

void MVGGeometryUtil::viewToCamera(M3dView& view, const MPoint& viewPoint, MPoint& point)
{
	double portHeight = (double)view.portHeight();
	double portWidth = (double)view.portWidth();
	
	MDagPath dagPath;
	view.getCamera(dagPath);
    MVGCamera camera(dagPath);
	point.x = (viewPoint.x / portWidth) - 0.5;
	point.y = (viewPoint.y / portWidth) - 0.5 - 0.5 * (portHeight / portWidth - 1.0 );
	point.z = 0.;
	// zoom
	point =  point * camera.getHorizontalFilmAperture() * camera.getZoom();
	// pan
	point.x += camera.getHorizontalPan();
	point.y += camera.getVerticalPan();  
}

 void MVGGeometryUtil::worldToCamera(M3dView& view, const MPoint& worldPoint, MPoint& point)
{
	MPoint viewPoint = worldToView(view, worldPoint);
	viewToCamera(view, viewPoint, point);
}

void MVGGeometryUtil::cameraToView(M3dView& view, const MPoint& point, MPoint& viewPoint)
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
	viewPoint.x = round((newX + 0.5) * view.portWidth());
	viewPoint.y = round((newY + 0.5 + 0.5 * (view.portHeight() / (float)view.portWidth() - 1.0)) * view.portWidth());	
}

void MVGGeometryUtil::cameraToImage(const MVGCamera& camera, const MPoint& point, MPoint& image)
{
	// Get image size
	MFnDagNode fnImage(camera.getImagePath());
	const double width = fnImage.findPlug("coverageX").asDouble();
	const double height = fnImage.findPlug("coverageY").asDouble();

	MPoint pointCenteredNorm = point / camera.getHorizontalFilmAperture();
	
	const double verticalMargin = (width - height) / 2.0;
	image.x = (pointCenteredNorm.x + 0.5 ) * width;
	image.y = (-pointCenteredNorm.y + 0.5) * width - verticalMargin;
}

bool MVGGeometryUtil::projectFace2D(M3dView& view, MPointArray& face3DPoints, const MVGCamera& camera, const MPointArray& face2DPoints, MVector height)
{
	if(!camera.isValid())
		return false;

	std::vector<MVGPointCloudItem> items = camera.getVisibleItems();
	if(items.size() < 3)
		return false;

	std::vector<MVGPointCloudItem>::const_iterator it = items.begin();
	std::vector<MPoint> facePoints_view;

	// Camera -> View
	MPoint viewPoint;
	for(size_t i = 0; i < face2DPoints.length(); ++i) {
		cameraToView(view, face2DPoints[i], viewPoint);
		facePoints_view.push_back(viewPoint);
	}
	cameraToView(view, face2DPoints[0], viewPoint);	// Extra point
	facePoints_view.push_back(viewPoint);

	// Get selected points in pointCloud
	std::vector<MVGPointCloudItem> selectedItems;
	int windingNumber = 0;
	for(; it != items.end(); ++it){
		windingNumber = wn_PnPoly(worldToView(view, it->_position), facePoints_view);
		if(windingNumber != 0)
			selectedItems.push_back(*it);
	}
	if(selectedItems.size() < 3)
		return false;

	// 3D plane estimation
	// w/ a variant of RANSAC using Least Median of Squares
	openMVG::Mat selectedItemsMat(3, selectedItems.size());
	for (size_t i = 0; i < selectedItems.size(); ++i)
		selectedItemsMat.col(i) = AS_VEC3(selectedItems[i]._position);

	PlaneKernel kernel(selectedItemsMat);
	PlaneKernel::Model model;
	double outlierThreshold = std::numeric_limits<double>::infinity();
	double dBestMedian = openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);

	// Retrieve projected Face3D vertices from this model
	MPoint P;
	MPoint cameraCenter = AS_MPOINT(camera.getPinholeCamera()._C);
	MVGPointCloud cloud(MVGProject::_CLOUD);
	MMatrix inclusiveMatrix = MMatrix::identity;
	if(cloud.isValid() && cloud.getDagPath().isValid())
		inclusiveMatrix = cloud.getDagPath().inclusiveMatrix();
	cameraCenter *= inclusiveMatrix;

	MPoint worldPoint;
	if(height.length() != 0) {
		// First points projection
		for(size_t i = 0; i < facePoints_view.size()-2; ++i) { // remove extra point
			worldPoint = viewToWorld(view, facePoints_view[i]);
			plane_line_intersect(model, cameraCenter, worldPoint, P);
			face3DPoints.append(P);
		}
		// Last point projection (keep 3D lengths)
		MPoint lastPoint3D = face3DPoints[2] + height; // TODO : warning
		MPoint viewPoint = worldToView(view, lastPoint3D);
		worldPoint = viewToWorld(view, viewPoint);
		plane_line_intersect(model, cameraCenter, worldPoint, P);
		face3DPoints.append(P);
	}
	else {
		for(size_t i = 0; i < facePoints_view.size()-1; ++i) { // remove extra point
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
	openMVG::Mat facePointsMat(3, facePoints3D.length());
	for (size_t i = 0; i <facePoints3D.length(); ++i)
		facePointsMat.col(i) = AS_VEC3(facePoints3D[i]);
	PlaneKernel kernel(facePointsMat);
	double outlierThreshold = std::numeric_limits<double>::infinity();
	openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);
	
	return true;
}

void MVGGeometryUtil::projectPointOnPlane(const MPoint& point, M3dView& view, PlaneKernel::Model& model, const MVGCamera& camera, MPoint& projectedPoint)
{
	MPoint cameraCenter = AS_MPOINT(camera.getPinholeCamera()._C);
	MVGPointCloud cloud(MVGProject::_CLOUD);
	MMatrix inclusiveMatrix = MMatrix::identity;
	if(cloud.isValid() && cloud.getDagPath().isValid())
		inclusiveMatrix = cloud.getDagPath().inclusiveMatrix();
	cameraCenter *= inclusiveMatrix;

    MPoint viewPoint;
	MVector dir;
	cameraToView(view, point, viewPoint);
	MPoint worldPoint;
	view.viewToWorld(viewPoint.x, viewPoint.y, worldPoint, dir);
	plane_line_intersect(model, cameraCenter, worldPoint, projectedPoint);
}

void MVGGeometryUtil::triangulatePoint(MPointArray& points2D, std::vector<MVGCamera>& cameras, MPoint& resultPoint3D)
{
	// 2 views
	MPoint imagePoint;
	MVGGeometryUtil::cameraToImage(cameras[0], points2D[0], imagePoint);
	const openMVG::Vec2 xL_(imagePoint.x,	imagePoint.y);
	MVGGeometryUtil::cameraToImage(cameras[1], points2D[1], imagePoint);
	const openMVG::Vec2 xR_(imagePoint.x, imagePoint.y);		

	// Retrieve pinhole cameras
	MVGPointCloud cloud(MVGProject::_CLOUD);
	MMatrix inclusiveMatrix = MMatrix::identity;
	if(cloud.isValid() && cloud.getDagPath().isValid())
		inclusiveMatrix = cloud.getDagPath().inclusiveMatrix();	
	MTransformationMatrix transformMatrix(inclusiveMatrix);
	MMatrix rotationMatrix = transformMatrix.asRotateMatrix();
	openMVG::Mat3 rotation;
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
			rotation(i, j) = rotationMatrix[i][j];
	}
	MPoint cameraCenterL = AS_MPOINT(cameras[0].getPinholeCamera()._C);
	cameraCenterL *= inclusiveMatrix;
	const openMVG::Mat3 KL = cameras[0].getPinholeCamera()._K;
	const openMVG::Mat3 RL = cameras[0].getPinholeCamera()._R;
	openMVG::Vec3 CL = AS_VEC3(cameraCenterL);
	const openMVG::Vec3 tL = -RL* rotation *CL;
	MPoint cameraCenterR = AS_MPOINT(cameras[1].getPinholeCamera()._C);
	cameraCenterR *= inclusiveMatrix;
	const openMVG::Mat3 KR = cameras[1].getPinholeCamera()._K;
	const openMVG::Mat3 RR = cameras[1].getPinholeCamera()._R;
	openMVG::Vec3 CR = AS_VEC3(cameraCenterR);
	const openMVG::Vec3 tR = -RR* rotation * CR;
	openMVG::Mat34 PL;
	openMVG::P_From_KRt(KL, RL* rotation, tL, &PL);
	openMVG::Mat34 PR;
	openMVG::P_From_KRt(KR, RR* rotation, tR, &PR); 

	openMVG::Vec3 result;
	openMVG::TriangulateDLT(PL, xL_, PR, xR_, &result);

	resultPoint3D.x = result(0);
	resultPoint3D.y = result(1);
	resultPoint3D.z = result(2);
}

double MVGGeometryUtil::crossProduct2D(MVector& A, MVector& B) {
    return A.x*B.y - A.y*B.x;
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
} // namespace
