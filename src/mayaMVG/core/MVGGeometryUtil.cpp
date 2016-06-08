#include "mayaMVG/core/MVGGeometryUtil.hpp"
#include "mayaMVG/core/MVGPointCloud.hpp"
#include "mayaMVG/core/MVGProject.hpp"
#include "mayaMVG/core/MVGCamera.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include "mayaMVG/core/MVGPlaneKernel.hpp"
#include "mayaMVG/core/MVGLineConstrainedPlaneKernel.hpp"
#include "mayaMVG/maya/MVGMayaUtil.hpp"
#include <openMVG/multiview/triangulation_nview.hpp>
#include <maya/MPointArray.h>
#include <maya/M3dView.h>
#include <maya/MPlug.h>
#include <maya/MFnDagNode.h>
#include <maya/MMatrix.h>

namespace mayaMVG
{

void MVGGeometryUtil::viewToCameraSpace(M3dView& view, const MPoint& viewPoint, MPoint& cameraPoint)
{
    double portHeight = (double)view.portHeight();
    double portWidth = (double)view.portWidth();
    MDagPath dagPath;
    view.getCamera(dagPath);
    MVGCamera camera(dagPath);
    // center
    cameraPoint.x = (viewPoint.x / portWidth) - 0.5;
    cameraPoint.y = (viewPoint.y / portWidth) - 0.5 - 0.5 * (portHeight / portWidth - 1.0);
    cameraPoint.z = 0.;
    // zoom
    cameraPoint = cameraPoint * camera.getHorizontalFilmAperture() * camera.getZoom();
    // pan
    cameraPoint.x += camera.getHorizontalPan();
    cameraPoint.y += camera.getVerticalPan();
}

MPoint MVGGeometryUtil::viewToCameraSpace(M3dView& view, const MPoint& viewPoint)
{
    MPoint point;
    viewToCameraSpace(view, viewPoint, point);
    return point;
}

void MVGGeometryUtil::viewToCameraSpace(M3dView& view, const MPointArray& viewPoints,
                                        MPointArray& cameraPoints)
{
    cameraPoints.setLength(viewPoints.length());
    for(size_t i = 0; i < viewPoints.length(); ++i)
        viewToCameraSpace(view, viewPoints[i], cameraPoints[i]);
}

MPointArray MVGGeometryUtil::viewToCameraSpace(M3dView& view, const MPointArray& viewPoints)
{
    MPointArray points;
    viewToCameraSpace(view, viewPoints, points);
    return points;
}

void MVGGeometryUtil::cameraToViewSpace(M3dView& view, const MPoint& cameraPoint, MPoint& viewPoint)
{
    MDagPath dagPath;
    view.getCamera(dagPath);
    MVGCamera camera(dagPath);
    float x = cameraPoint.x;
    float y = cameraPoint.y;
    // pan
    x -= camera.getHorizontalPan();
    y -= camera.getVerticalPan();
    // zoom
    x /= (camera.getHorizontalFilmAperture() * camera.getZoom());
    y /= (camera.getHorizontalFilmAperture() * camera.getZoom());
    // center
    viewPoint.x = round((x + 0.5) * view.portWidth());
    viewPoint.y = round((y + 0.5 + 0.5 * (view.portHeight() / (float)view.portWidth() - 1.0)) *
                        view.portWidth());
}

MPoint MVGGeometryUtil::cameraToViewSpace(M3dView& view, const MPoint& cameraPoint)
{
    MPoint point;
    cameraToViewSpace(view, cameraPoint, point);
    return point;
}

void MVGGeometryUtil::cameraToViewSpace(M3dView& view, const MPointArray& cameraPoints,
                                        MPointArray& viewPoints)
{
    viewPoints.setLength(cameraPoints.length());
    for(size_t i = 0; i < viewPoints.length(); ++i)
        cameraToViewSpace(view, cameraPoints[i], viewPoints[i]);
}

MPointArray MVGGeometryUtil::cameraToViewSpace(M3dView& view, const MPointArray& cameraPoints)
{
    MPointArray points;
    cameraToViewSpace(view, cameraPoints, points);
    return points;
}

void MVGGeometryUtil::worldToViewSpace(M3dView& view, const MPoint& worldPoint, MPoint& viewPoint)
{
    // don't use M3dView::worldToView() because of the cast to short values
    MMatrix modelViewMatrix, projectionMatrix;
    CHECK(view.modelViewMatrix(modelViewMatrix))
    CHECK(view.projectionMatrix(projectionMatrix))
    MPoint point = worldPoint * (modelViewMatrix * projectionMatrix);
    unsigned int viewportX, viewportY, viewportWidth, viewportHeight;
    view.viewport(viewportX, viewportY, viewportWidth, viewportHeight);
    viewPoint.x =
        static_cast<int>(static_cast<double>(viewportWidth) * (point.x / point.w + 1.0) / 2.0);
    viewPoint.y =
        static_cast<int>(static_cast<double>(viewportHeight) * (point.y / point.w + 1.0) / 2.0);
    viewPoint.z = 0.0;
}

MPoint MVGGeometryUtil::worldToViewSpace(M3dView& view, const MPoint& worldPoint)
{
    MPoint point;
    worldToViewSpace(view, worldPoint, point);
    return point;
}

void MVGGeometryUtil::worldToViewSpace(M3dView& view, const MPointArray& worldPoints,
                                       MPointArray& viewPoints)
{
    viewPoints.setLength(worldPoints.length());
    for(size_t i = 0; i < worldPoints.length(); ++i)
        worldToViewSpace(view, worldPoints[i], viewPoints[i]);
}

MPointArray MVGGeometryUtil::worldToViewSpace(M3dView& view, const MPointArray& worldPoints)
{
    MPointArray points;
    worldToViewSpace(view, worldPoints, points);
    return points;
}

void MVGGeometryUtil::viewToWorldSpace(M3dView& view, const MPoint& viewPoint, MPoint& worldPoint)
{
    MPoint worldDir;
    CHECK(view.viewToWorld((short)viewPoint.x, (short)viewPoint.y, worldPoint, worldDir))
}

MPoint MVGGeometryUtil::viewToWorldSpace(M3dView& view, const MPoint& viewPoint)
{
    MPoint point;
    viewToWorldSpace(view, viewPoint, point);
    return point;
}

void MVGGeometryUtil::viewToWorldSpace(M3dView& view, const MPointArray& viewPoints,
                                       MPointArray& worldPoints)
{
    worldPoints.setLength(viewPoints.length());
    MPoint worldDir;
    for(size_t i = 0; i < viewPoints.length(); ++i)
        CHECK(view.viewToWorld((short)viewPoints[i].x, (short)viewPoints[i].y, worldPoints[i],
                               worldDir))
}

MPointArray MVGGeometryUtil::viewToWorldSpace(M3dView& view, const MPointArray& viewPoints)
{
    MPointArray points;
    viewToWorldSpace(view, viewPoints, points);
    return points;
}

void MVGGeometryUtil::worldToCameraSpace(M3dView& view, const MPoint& worldPoint,
                                         MPoint& cameraPoint)
{
    viewToCameraSpace(view, worldToViewSpace(view, worldPoint), cameraPoint);
}

MPoint MVGGeometryUtil::worldToCameraSpace(M3dView& view, const MPoint& worldPoint)
{
    MPoint point;
    viewToCameraSpace(view, worldToViewSpace(view, worldPoint), point);
    return point;
}

void MVGGeometryUtil::worldToCameraSpace(M3dView& view, const MPointArray& worldPoints,
                                         MPointArray& cameraPoints)
{
    cameraPoints.setLength(worldPoints.length());
    for(size_t i = 0; i < worldPoints.length(); ++i)
        viewToCameraSpace(view, worldToViewSpace(view, worldPoints[i]), cameraPoints[i]);
}

MPointArray MVGGeometryUtil::worldToCameraSpace(M3dView& view, const MPointArray& worldPoints)
{
    MPointArray points;
    viewToCameraSpace(view, worldToViewSpace(view, worldPoints), points);
    return points;
}

void MVGGeometryUtil::cameraToWorldSpace(M3dView& view, const MPoint& cameraPoint,
                                         MPoint& worldPoint)
{
    viewToWorldSpace(view, cameraToViewSpace(view, cameraPoint), worldPoint);
}

MPoint MVGGeometryUtil::cameraToWorldSpace(M3dView& view, const MPoint& cameraPoint)
{
    MPoint point;
    cameraToWorldSpace(view, cameraPoint, point);
    return point;
}

void MVGGeometryUtil::cameraToWorldSpace(M3dView& view, const MPointArray& cameraPoints,
                                         MPointArray& worldPoints)
{
    worldPoints.setLength(cameraPoints.length());
    for(size_t i = 0; i < worldPoints.length(); ++i)
        cameraToWorldSpace(view, cameraPoints[i], worldPoints[i]);
}

MPointArray MVGGeometryUtil::cameraToWorldSpace(M3dView& view, const MPointArray& cameraPoints)
{
    MPointArray points;
    cameraToWorldSpace(view, cameraPoints, points);
    return points;
}

void MVGGeometryUtil::cameraToImageSpace(MVGCamera& camera, const MPoint& cameraPoint,
                                         MPoint& imagePoint)
{
    MFnDagNode fnImage(camera.getImagePlaneShapeDagPath());

    MIntArray sensorSize;
    camera.getSensorSize(sensorSize);
    const double width = sensorSize[0];
    const double height = sensorSize[1];
    assert(camera.getHorizontalFilmAperture() != 0.0);
    MPoint pointCenteredNorm = cameraPoint / camera.getHorizontalFilmAperture();
    const double verticalMargin = (width - height) / 2.0;
    imagePoint.x = (pointCenteredNorm.x + 0.5) * width;
    imagePoint.y = (-pointCenteredNorm.y + 0.5) * width - verticalMargin;
}

MPoint MVGGeometryUtil::cameraToImageSpace(MVGCamera& camera, const MPoint& cameraPoint)
{
    MPoint point;
    cameraToImageSpace(camera, cameraPoint, point);
    return point;
}

void MVGGeometryUtil::cameraToImageSpace(MVGCamera& camera, const MPointArray& cameraPoints,
                                         MPointArray& imagePoints)
{
    imagePoints.setLength(cameraPoints.length());
    for(size_t i = 0; i < cameraPoints.length(); ++i)
        cameraToImageSpace(camera, cameraPoints[i], imagePoints[i]);
}

MPointArray MVGGeometryUtil::cameraToImageSpace(MVGCamera& camera, const MPointArray& cameraPoints)
{
    MPointArray points;
    cameraToImageSpace(camera, cameraPoints, points);
    return points;
}

/**
 *
 * @param[in] pointsWS : all points used to compute plane in World Space coordinates
 * @param[out] model : computed plane
 * @return
 */
bool MVGGeometryUtil::computePlane(const MPointArray& pointsWS, PlaneKernel::Model& model)
{
    if(pointsWS.length() < 3)
        return false;

    openMVG::Mat facePointsMat(3, pointsWS.length());
    for(size_t i = 0; i < pointsWS.length(); ++i)
        facePointsMat.col(i) = TO_VEC3(pointsWS[i]);
    PlaneKernel kernel(facePointsMat);
    double outlierThreshold = std::numeric_limits<double>::infinity();
    openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);

    return true;
}

/**
 *
 * @param[in] pointsWS: all points used to compute plane in World Space coordinates
 * @param[in] constraintPoints : points describing the line constraint
 * @param[out] model : computed plane
 * @return
 */
bool MVGGeometryUtil::computePlaneWithLineConstraint(const MPointArray& pointsWS,
                                                     const MPointArray& constraintPoints,
                                                     LineConstrainedPlaneKernel::Model& model)
{
    if(pointsWS.length() < 3)
        return false;
    if(constraintPoints.length() < 2)
        return false;
    openMVG::Mat facePointsMat(3, pointsWS.length());
    for(size_t i = 0; i < pointsWS.length(); ++i)
        facePointsMat.col(i) = TO_VEC3(pointsWS[i]);
    LineConstrainedPlaneKernel kernel(facePointsMat, TO_VEC3(constraintPoints[0]),
                                      TO_VEC3(constraintPoints[1]));
    double outlierThreshold = std::numeric_limits<double>::infinity();
    openMVG::robust::LeastMedianOfSquares(kernel, &model, &outlierThreshold);

    return true;
}

/**
 *
 * @param view
 * @param[in] toProjectCSPoints : points to project in the computed plane in Camera Space
 *coordinates
 * @param[in] faceWSPoints : all points used to compute plane in World Space coordinates
 * @param[out] projectedWSPoints : toPojectCSPoints projected in plane in World Space coordinates
 * @return
 */
bool MVGGeometryUtil::projectPointsOnPlane(M3dView& view, const MPointArray& toProjectCSPoints,
                                           const PlaneKernel::Model& planeModel,
                                           MPointArray& projectedWSPoints)
{
    // Retrieve camera center
    MDagPath cameraPath;
    view.getCamera(cameraPath);
    MVGCamera camera(cameraPath);
    MPoint cameraCenter = camera.getCenter();

    // project points on computed plane
    MPoint projectedWSPoint;
    for(size_t i = 0; i < toProjectCSPoints.length(); ++i)
    {
        plane_line_intersect(planeModel, cameraCenter,
                             MVGGeometryUtil::cameraToWorldSpace(view, toProjectCSPoints[i]),
                             projectedWSPoint);
        projectedWSPoints.append(projectedWSPoint);
    }
    assert(toProjectCSPoints.length() == projectedWSPoints.length());
    return true;
}

bool MVGGeometryUtil::projectPointOnPlane(M3dView& view, const MPoint& toProjectCSPoint,
                                          const PlaneKernel::Model& planeModel,
                                          MPoint& projectedWSPoint)
{
    MPointArray toProjectCSPoints;
    MPointArray projectedWSPoints;
    toProjectCSPoints.append(toProjectCSPoint);
    if(projectPointsOnPlane(view, toProjectCSPoints, planeModel, projectedWSPoints))
    {
        assert(projectedWSPoints.length() == 1);
        projectedWSPoint = projectedWSPoints[0];
        return true;
    }
    return false;
}

/**
 * @brief N-View triangulation.
 *
 * @param point2dPerCamera_CS map of 2d points per camera in Camera Space
 * @param outTriangulatedPoint_WS 3D triangulated point in World Space
 */
void MVGGeometryUtil::triangulatePoint(const std::map<int, MPoint>& point2dPerCamera_CS,
                                       MPoint& outTriangulatedPoint_WS)
{
    const size_t cameraCount = point2dPerCamera_CS.size();
    assert(cameraCount > 1);
    // prepare n-view triangulation data
    openMVG::Mat2X imagePoints(2, cameraCount);

    std::vector<openMVG::Mat34> projectiveCameras;
    {
        std::map<int, MPoint>::const_iterator it = point2dPerCamera_CS.begin();
        for(size_t i = 0; it != point2dPerCamera_CS.end(); ++i, ++it)
        {
            MVGCamera camera(it->first);
            const MPoint& point2d_CS = it->second;

            // Retrieve the intrinsic matrix from 'pinholeProjectionMatrix' attribute
            //
            // K Matrix:
            // f*k_u     0      c_u
            //   0     f*k_v    c_v
            //   0       0       1
            // c_u, c_v : the principal point, which would be ideally in the centre of the image.
            //
            MDoubleArray intrinsicsArray;
            MVGMayaUtil::getDoubleArrayAttribute(camera.getDagPath().node(), "mvg_intrinsicParams",
                                                 intrinsicsArray);
            MIntArray sensorSize;
            camera.getSensorSize(sensorSize);

            // Keep ideal matrix with principal point centered
            openMVG::Mat3 K;
            K << intrinsicsArray[0], 0.0, sensorSize[0] / 2.0, 0.0, intrinsicsArray[0],
                sensorSize[1] / 2.0, 0.0, 0.0, 1.0;

            // Retrieve transformation matrix
            const MMatrix inclusiveMatrix = camera.getDagPath().inclusiveMatrix();
            const MTransformationMatrix transformMatrix(inclusiveMatrix);
            openMVG::Mat3 R;
            MMatrix rotationMatrix = transformMatrix.asRotateMatrix();
            for(int m = 0; m < 3; ++m)
            {
                for(int j = 0; j < 3; ++j)
                {
                    // Maya has inverted Y and Z axes
                    int sign = 1;
                    if(m > 0)
                        sign = -1;
                    R(m, j) = sign * rotationMatrix[m][j];
                }
            }

            // Retrieve translation vector
            const openMVG::Vec3 C = TO_VEC3(camera.getCenter());
            const openMVG::Vec3 t = -R * C;

            // Compute projection matrix
            openMVG::Mat34 P;
            openMVG::P_From_KRt(K, R, t, &P);
            projectiveCameras.push_back(P);

            // clicked point matrix (image space)
            MPoint clickedISPosition;
            MVGGeometryUtil::cameraToImageSpace(camera, point2d_CS, clickedISPosition);
            imagePoints.col(i) = openMVG::Vec2(clickedISPosition.x, clickedISPosition.y);
        }
    }

    // call n-view triangulation function
    openMVG::Vec4 result;
    openMVG::TriangulateNViewAlgebraic(imagePoints, projectiveCameras, &result);
    outTriangulatedPoint_WS.x = result(0);
    outTriangulatedPoint_WS.y = result(1);
    outTriangulatedPoint_WS.z = result(2);
    if(result(3) == 0.0)
    {
        LOG_ERROR("Triangulated point w = 0")
        return;
    }
    outTriangulatedPoint_WS = outTriangulatedPoint_WS / result(3);
}

double MVGGeometryUtil::crossProduct2D(MVector& A, MVector& B)
{
    return A.x * B.y - A.y * B.x;
}

bool MVGGeometryUtil::doEdgesIntersect(MPoint A, MPoint B, MVector AD, MVector BC)
{
    // r x s = 0
    double cross = crossProduct2D(AD, BC);
    double eps = 0.00001;
    if(cross < eps && cross > -eps)
        return false;

    MVector AB = B - A;

    double x = crossProduct2D(AB, BC) / crossProduct2D(AD, BC);
    double y = crossProduct2D(AB, AD) / crossProduct2D(AD, BC);

    if(x >= 0 && x <= 1 && y >= 0 && y <= 1)
    {
        return true;
    }
    return false;
}

} // namespace
