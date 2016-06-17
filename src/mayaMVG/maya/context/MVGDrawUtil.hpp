#pragma once

#include <maya/MColor.h>
#include <maya/M3dView.h>
#include <maya/MPointArray.h>

namespace mayaMVG
{

struct MVGDrawUtil
{
    static void begin2DDrawing(const int portWidth, const int portHeight);
    static void end2DDrawing();

    static void drawLine2D(const MPoint& A, const MPoint& B, const MColor& color,
                           const float lineWidth = 1.5f, const float alpha = 1.f,
                           bool stipple = false);
    static void drawLine3D(const MPoint& A, const MPoint& B, const MColor& color,
                           const float lineWidth = 1.5f, const float alpha = 1.f,
                           bool stipple = false);
    static void drawLineLoop2D(const MPointArray& points, const MColor& color,
                               const float lineWidth = 1.f, const float alpha = 1.f);
    static void drawLineLoop3D(const MPointArray& points, const MColor& color,
                               const float lineWidth = 1.f, const float alpha = 1.f);
    static void drawPolygon2D(const MPointArray& points, const MColor& color,
                              const float alpha = 1.f);
    static void drawPolygon3D(const MPointArray& points, const MColor& color,
                              const float alpha = 1.f);
    static void drawPoint2D(const MPoint& point, const MColor& color, const float pointSize = 1.f,
                            const float alpha = 1.f);
    static void drawPoint3D(const MPoint& point, const MColor& color, const float pointSize = 4.f,
                            const float alpha = 1.f);
    static void drawPoints2D(const MPointArray& points, const MColor& color,
                             const float pointSize = 1.f, const float alpha = 1.f);
    static void drawPoints3D(const MPointArray& points, const MColor& color,
                             const float pointSize = 4.f, const float alpha = 1.f);
    static void drawCircle2D(const MPoint& center, const MColor& color, const int r,
                             const int segments);
    static void drawEmptyCross(const MPoint& originVS, const float width, const float thickness,
                               const MColor& color, const float lineWidth = 1.0);
    static void drawFullCross(const MPoint& originVS, const float width, const float thickness,
                              const MColor& color);

    // Cursors
    static void drawArrowsCursor(const MPoint& originVS, const MColor& color);
    static void drawTargetCursor(const MPoint& originVS, const MColor& color);
    static void drawExtendCursorItem(const MPoint& originVS, const MColor& color);
    static void drawPointCloudCursorItem(const MPoint& originVS, const MColor& color);
    static void drawPlaneCursorItem(const MPoint& originVS, const MColor& color);
    static void drawLocatorCursorItem(const MPoint& originVS);

    // Points
    // Lines and polygons on create
    static void drawClickedPoints(const MPointArray& clickedVSPoints, const MColor color);
    // Association between 2D and 3D point
    static void drawTriangulatedPoint(M3dView& view, const MPoint& draggedVSPoint,
                                      const MPoint& worldPoint, MColor color);
    static void drawTriangulatedPoints(M3dView& view, const MPointArray& onPressWSPoints,
                                       const MPointArray& onDragVSPositions);
    static void drawFinalWSPoints(const MPointArray& finalWSPositions,
                                  const MPointArray& onPressWSPositions);

    // Colors
    static const MColor _okayColor;
    static const MColor _errorColor;
    static const MColor _cursorColor;
    static const MColor _createColor;
    static const MColor _triangulateColor;
    static const MColor _placedInOtherViewColor;
    static const MColor _pointCloudColor;
    static const MColor _adjacentFaceColor;
    static const MColor _intersectionColor;
    static const MColor _selectionColor;
};

} // namespace
