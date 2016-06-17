#pragma once

#include "mayaMVG/core/MVGPlaneKernel.hpp"
#include "mayaMVG/core/MVGLineConstrainedPlaneKernel.hpp"
#include <maya/MVector.h>
#include <map>

class MPoint;
class MPointArray;
class M3dView;

#define TO_MPOINT(a) MPoint(a(0), a(1), a(2))
#define TO_VEC3(a) openMVG::Vec3(a.x, a.y, a.z)

namespace mayaMVG
{

class MVGCamera;

struct MVGGeometryUtil
{
    // space conversion
    static void viewToCameraSpace(M3dView& view, const MPoint& viewPoint, MPoint& cameraPoint);
    static MPoint viewToCameraSpace(M3dView& view, const MPoint& viewPoint);
    static void viewToCameraSpace(M3dView& view, const MPointArray& viewPoints,
                                  MPointArray& cameraPoints);
    static MPointArray viewToCameraSpace(M3dView& view, const MPointArray& viewPoint);

    static void cameraToViewSpace(M3dView& view, const MPoint& cameraPoint, MPoint& viewPoint);
    static MPoint cameraToViewSpace(M3dView& view, const MPoint& cameraPoint);
    static void cameraToViewSpace(M3dView& view, const MPointArray& cameraPoints,
                                  MPointArray& viewPoints);
    static MPointArray cameraToViewSpace(M3dView& view, const MPointArray& cameraPoint);

    static void worldToViewSpace(M3dView& view, const MPoint& worldPoint, MPoint& viewPoint);
    static MPoint worldToViewSpace(M3dView& view, const MPoint& worldPoint);
    static void worldToViewSpace(M3dView& view, const MPointArray& worldPoints,
                                 MPointArray& viewPoints);
    static MPointArray worldToViewSpace(M3dView& view, const MPointArray& worldPoints);

    static void viewToWorldSpace(M3dView& view, const MPoint& viewPoint, MPoint& worldPoint);
    static MPoint viewToWorldSpace(M3dView& view, const MPoint& viewPoint);
    static void viewToWorldSpace(M3dView& view, const MPointArray& viewPoints,
                                 MPointArray& worldPoints);
    static MPointArray viewToWorldSpace(M3dView& view, const MPointArray& viewPoints);

    static void worldToCameraSpace(M3dView& view, const MPoint& worldPoint, MPoint& cameraPoint);
    static MPoint worldToCameraSpace(M3dView& view, const MPoint& worldPoint);
    static void worldToCameraSpace(M3dView& view, const MPointArray& worldPoints,
                                   MPointArray& cameraPoints);
    static MPointArray worldToCameraSpace(M3dView& view, const MPointArray& worldPoints);

    static void cameraToWorldSpace(M3dView& view, const MPoint& cameraPoint, MPoint& worldPoint);
    static MPoint cameraToWorldSpace(M3dView& view, const MPoint& cameraPoint);
    static void cameraToWorldSpace(M3dView& view, const MPointArray& cameraPoints,
                                   MPointArray& worldPoints);
    static MPointArray cameraToWorldSpace(M3dView& view, const MPointArray& cameraPoints);

    static void cameraToImageSpace(MVGCamera& camera, const MPoint& cameraPoint,
                                   MPoint& imagePoint);
    static MPoint cameraToImageSpace(MVGCamera& camera, const MPoint& cameraPoint);
    static void cameraToImageSpace(MVGCamera& camera, const MPointArray& cameraPoints,
                                   MPointArray& imagePoints);
    static MPointArray cameraToImageSpace(MVGCamera& camera, const MPointArray& cameraPoint);

    // projections
    static bool computePlane(const MPointArray& points, PlaneKernel::Model& model);
    static bool computePlaneWithLineConstraint(const MPointArray& pointsWS,
                                               const MPointArray& constraintPoints,
                                               LineConstrainedPlaneKernel::Model& model);
    static bool projectPointsOnPlane(M3dView& view, const MPointArray& toProjectCSPoints,
                                     const PlaneKernel::Model& planeModel,
                                     MPointArray& projectedWSPoints);
    static bool projectPointOnPlane(M3dView& view, const MPoint& toProjectCSPoint,
                                    const PlaneKernel::Model& planeModel, MPoint& projectedWSPoint);

    // triangulation
    static void triangulatePoint(const std::map<int, MPoint>& point2dPerCamera_CS,
                                 MPoint& outTriangulatedPoint_WS);

    // intersections
    static double crossProduct2D(MVector& A, MVector& B);
    static bool doEdgesIntersect(MPoint A, MPoint B, MVector AD, MVector BC);
};

} // namespace
