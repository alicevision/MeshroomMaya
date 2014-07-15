#include "mayaMVG/maya/context/MVGManipulatorUtil.h"
#include "mayaMVG/maya/MVGMayaUtil.h"
#include "mayaMVG/maya/context/MVGDrawUtil.h"
#include "mayaMVG/core/MVGProject.h"

#include "mayaMVG/core/MVGLog.h"

namespace mayaMVG {

bool MVGManipulatorUtil::intersectPoint(M3dView& view, DisplayData* displayData, IntersectionData& intersectionData, const short&x, const short& y)
{
	if(!displayData)
		return false;
	double threshold = (2*POINT_RADIUS*displayData->camera.getZoom())/view.portHeight();
	const MPointArray& points = displayData->cameraPoints2D;
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, displayData->camera, x, y, mousePoint);
	for(int i = 0; i < points.length(); ++i) {
		if(points[i].x <= mousePoint.x + threshold && points[i].x >= mousePoint.x - threshold
			&& points[i].y <= mousePoint.y + threshold && points[i].y >= mousePoint.y - threshold)
		{
			intersectionData.pointIndex = i;
			return true;
		}
			
	}
	intersectionData.pointIndex = -1;
	return false;
}

bool MVGManipulatorUtil::intersectEdge(M3dView& view, DisplayData* displayData, IntersectionData& intersectionData, const short&x, const short& y)
{
	if(!displayData)
		return false;
	const MPointArray& points = displayData->cameraPoints2D;
	MPoint mousePoint;
	MVGGeometryUtil::viewToCamera(view, displayData->camera, x, y, mousePoint);
	
	if(points.length() < 2)
		return false;
	
	double minDistanceFound = -1.0;
	double tolerance = 0.001 * displayData->camera.getZoom() * 30;
	double distance;
	MIntArray tmp;
	for(int i = 0; i < points.length() - 1; i++) {
		MVector AB = points[i+1] - points[i];
		MVector PA = points[i] - mousePoint;
		MVector AP = mousePoint - points[i];
		MVector BP = mousePoint - points[i+1];
		MVector BA = points[i] - points[i+1];
		// Dot signs			
		int sign1, sign2;
		((AP*AB) > 0) ? sign1 = 1 : sign1 = -1;
		((BP*BA) > 0) ? sign2 = 1 : sign2 = -1;
		if(sign1 != sign2)
			continue;
		// Lenght of orthogonal projection on edge
		double s = crossProduct2D(AB, PA) / (AB.length()*AB.length());
		if(s < 0)
			s *= -1;
		distance = s * AB.length();
		if(minDistanceFound < 0.0 || distance < minDistanceFound)
		{
			tmp.clear();
			tmp.append(i);
			tmp.append(i+1);
			
			minDistanceFound = distance;
		}
	}
	if(minDistanceFound < -tolerance || minDistanceFound > tolerance)
	{
		intersectionData.edgePointIndexes.clear();
		return false;
	}
		
	
	intersectionData.edgePointIndexes = tmp;
	return true;
}
void MVGManipulatorUtil::drawIntersections(M3dView& view, DisplayData* data, IntersectionData& intersectionData, IntersectionState intersectionState)
{
	short x, y;
	MPointArray cameraPoints = data->cameraPoints2D;
	if(cameraPoints.length() > 0) {
		switch(intersectionState)
		{
			case MVGManipulatorUtil::eIntersectionPoint:
				glColor3f(0.f, 1.f, 0.f);
				MVGGeometryUtil::cameraToView(view, data->camera, cameraPoints[intersectionData.pointIndex], x, y);
				MVGDrawUtil::drawCircle(x, y, POINT_RADIUS, 30);
				break;
			case MVGManipulatorUtil::eIntersectionEdge:
				short x, y;
				glColor3f(0.f, 1.f, 0.f);
				glBegin(GL_LINES);
					MVGGeometryUtil::cameraToView(view, data->camera, cameraPoints[intersectionData.edgePointIndexes[0]], x, y);
					glVertex2f(x, y);
					MVGGeometryUtil::cameraToView(view, data->camera, cameraPoints[intersectionData.edgePointIndexes[1]], x, y);
					glVertex2f(x, y);
				glEnd();
				break;
		}	
	}
}
void MVGManipulatorUtil::drawCameraPoints(M3dView& view, DisplayData* data)
{
	short x, y;
	MPoint wPoint;
	MVector wdir;
	
	// To test reloadProjectFromMaya (since map are not reload for the moment)
//	const MPointArray& points = data->cameraPoints2D;
//	for(int i = 0; i < points.length(); ++i)
//	{
//		MVGGeometryUtil::cameraToView(view, data->camera, points[i], x, y);
//		MVGDrawUtil::drawFullCross(x, y);
//	}
//	
	for(Map3Dto2D::iterator mapIt = MVGProjectWrapper::instance().getMap3Dto2D().begin(); mapIt != MVGProjectWrapper::instance().getMap3Dto2D().end(); ++mapIt)
	{
		for(std::vector<PairStringToPoint>::iterator vecIt = mapIt->second.begin(); vecIt != mapIt->second.end(); ++vecIt)
		{
			if(data->camera.name() == vecIt->first)
			{
				// Draw full cross
				MVGGeometryUtil::cameraToView(view, data->camera, vecIt->second, x, y);
				MVGDrawUtil::drawFullCross(x, y);
				
				// Line toward 3D point
				MPoint point3D_view = MVGGeometryUtil::worldToView(view, mapIt->first.second);	
				glEnable(GL_LINE_STIPPLE);
				glLineStipple(1.f, 0x5555);
				glBegin(GL_LINES);
					glVertex2f(x, y);
					glVertex2f(point3D_view.x, point3D_view.y);
				glEnd();
				glDisable(GL_LINE_STIPPLE);
				
				view.viewToWorld(x + 5, y - 15, wPoint, wdir);
			}
			else
			{
				MPoint point3D_view =  MVGGeometryUtil::worldToView(view, mapIt->first.second);
				MVGDrawUtil::drawEmptyCross(point3D_view.x, point3D_view.y);
				
				view.viewToWorld(point3D_view.x + 5, point3D_view.y - 15, wPoint, wdir);
			}
				
			// Text
			MString str;
			str += (int)mapIt->second.size();
			view.drawText(str, wPoint);			
		}
	}
}
}	// mayaMVG