#include "mayaMVG/core/MVGGeometryUtil.hpp" // Included first because of preprocessor symbol error
#include "mayaMVG/maya/context/MVGDrawUtil.hpp"
#include "mayaMVG/core/MVGLog.hpp"
#include <maya/M3dView.h>
#include <cassert>

namespace mayaMVG
{
MColor const MVGDrawUtil::_okayColor = MColor(0.5f, 0.7f, 0.4f);
MColor const MVGDrawUtil::_errorColor = MColor(0.8f, 0.5f, 0.4f);
MColor const MVGDrawUtil::_cursorColor = MColor(0.f, 0.f, 0.f);
MColor const MVGDrawUtil::_createColor = MColor(0.9f, 0.9f, 0.1f);
MColor const MVGDrawUtil::_triangulateColor = MColor(0.9f, 0.5f, 0.4f);
MColor const MVGDrawUtil::_placedInOtherViewColor = MColor(0.2f, 0.7f, 0.8f);
MColor const MVGDrawUtil::_pointCloudColor = MColor(0.f, 1.f, 1.f);
MColor const MVGDrawUtil::_adjacentFaceColor = MColor(0.f, 0.f, 1.f);
MColor const MVGDrawUtil::_intersectionColor = MColor(1.f, 1.f, 1.f);
MColor const MVGDrawUtil::_selectionColor = MColor(0.4f, 1.f, 0.7f);

// static
void MVGDrawUtil::begin2DDrawing(const int portWidth, const int portHeight)
{
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, portWidth, 0, portHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// static
void MVGDrawUtil::end2DDrawing()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
    glPopMatrix();
}

// static
void MVGDrawUtil::drawLine2D(const MPoint& A, const MPoint& B, const MColor& color,
                             const float lineWidth, const float alpha, bool stipple)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    if(stipple)
    {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple((GLint)1.f, (GLushort)0x5555);
    }
    glColor4f(color.r, color.g, color.b, alpha);
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex2f(A.x, A.y);
    glVertex2f(B.x, B.y);
    glEnd();
    if(stipple)
        glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
}

// static
void MVGDrawUtil::drawLine3D(const MPoint& A, const MPoint& B, const MColor& color,
                             const float lineWidth, const float alpha, bool stipple)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    if(stipple)
    {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple((GLint)1.f, (GLushort)0x5555);
    }
    glColor4f(color.r, color.g, color.b, alpha);
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex3f(A.x, A.y, A.z);
    glVertex3f(B.x, B.y, B.z);
    glEnd();
    if(stipple)
        glDisable(GL_LINE_STIPPLE);
    glPopAttrib();
}

// static
void MVGDrawUtil::drawLineLoop2D(const MPointArray& points, const MColor& color,
                                 const float lineWidth, const float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor4f(color.r, color.g, color.b, alpha);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < points.length(); ++i)
        glVertex2f(points[i].x, points[i].y);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawLineLoop3D(const MPointArray& points, const MColor& color,
                                 const float lineWidth, const float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor4f(color.r, color.g, color.b, alpha);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < points.length(); ++i)
        glVertex3f(points[i].x, points[i].y, points[i].z);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPolygon2D(const MPointArray& points, const MColor& color, const float alpha)
{
    assert(points.length() > 2);
    //    glXQueryVersion(NULL, NULL, NULL);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_POLYGON);
    for(int i = 0; i < points.length(); ++i)
        glVertex2f(points[i].x, points[i].y);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPolygon3D(const MPointArray& points, const MColor& color, const float alpha)
{
    assert(points.length() > 2);
    //    glXQueryVersion(NULL, NULL, NULL);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_POLYGON);
    for(int i = 0; i < points.length(); ++i)
        glVertex3f(points[i].x, points[i].y, points[i].z);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPoint2D(const MPoint& point, const MColor& color, const float pointSize,
                              const float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPointSize(pointSize);
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_POINTS);
    glVertex2f(point.x, point.y);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPoint3D(const MPoint& point, const MColor& color, const float pointSize,
                              const float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPointSize(pointSize);
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_POINTS);
    glVertex3f(point.x, point.y, point.z);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPoints2D(const MPointArray& points, const MColor& color,
                               const float pointSize, const float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPointSize(pointSize);
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_POINTS);
    for(int i = 0; i < points.length(); ++i)
        glVertex2f(points[i].x, points[i].y);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPoints3D(const MPointArray& points, const MColor& color,
                               const float pointSize, const float alpha)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPointSize(pointSize);
    glColor4f(color.r, color.g, color.b, alpha);
    glBegin(GL_POINTS);
    for(int i = 0; i < points.length(); ++i)
        glVertex3f(points[i].x, points[i].y, points[i].z);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawCircle2D(const MPoint& center, const MColor& color, const int r,
                               const int segments)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for(int n = 0; n <= segments; ++n)
    {
        float const t = 2 * M_PI * (float)n / (float)segments;
        glVertex2f(center.x + sin(t) * r, center.y + cos(t) * r);
    }
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawEmptyCross(const MPoint& originVS, const float width, const float thickness,
                                 const MColor& color, const float lineWidth)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    glLineWidth(lineWidth);
    glBegin(GL_LINE_LOOP);
    glVertex2f(originVS.x + width, originVS.y - thickness);
    glVertex2f(originVS.x + width, originVS.y + thickness);
    glVertex2f(originVS.x + thickness, originVS.y + thickness);
    glVertex2f(originVS.x + thickness, originVS.y + width);
    glVertex2f(originVS.x - thickness, originVS.y + width);
    glVertex2f(originVS.x - thickness, originVS.y + thickness);
    glVertex2f(originVS.x - width, originVS.y + thickness);
    glVertex2f(originVS.x - width, originVS.y - thickness);
    glVertex2f(originVS.x - thickness, originVS.y - thickness);
    glVertex2f(originVS.x - thickness, originVS.y - width);
    glVertex2f(originVS.x + thickness, originVS.y - width);
    glVertex2f(originVS.x + thickness, originVS.y - thickness);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawFullCross(const MPoint& originVS, const float width, const float thickness,
                                const MColor& color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    glBegin(GL_POLYGON);
    glVertex2f(originVS.x + thickness, originVS.y - width);
    glVertex2f(originVS.x + thickness, originVS.y + width);
    glVertex2f(originVS.x - thickness, originVS.y + width);
    glVertex2f(originVS.x - thickness, originVS.y - width);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(originVS.x + width, originVS.y + thickness);
    glVertex2f(originVS.x - width, originVS.y + thickness);
    glVertex2f(originVS.x - width, originVS.y - thickness);
    glVertex2f(originVS.x + width, originVS.y - thickness);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawArrowsCursor(const MPoint& originVS, const MColor& color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    GLfloat step = 8;
    GLfloat width = 4;
    GLfloat height = 4;

    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(originVS.x - step, originVS.y);
    glVertex2f(originVS.x + step, originVS.y);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(originVS.x, originVS.y - step);
    glVertex2f(originVS.x, originVS.y + step);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(originVS.x + step, originVS.y + height);
    glVertex2f(originVS.x + step, originVS.y - height);
    glVertex2f(originVS.x + step + width, originVS.y);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(originVS.x + height, originVS.y - step);
    glVertex2f(originVS.x - height, originVS.y - step);
    glVertex2f(originVS.x, originVS.y - (step + width));
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(originVS.x - step, originVS.y + height);
    glVertex2f(originVS.x - step, originVS.y - height);
    glVertex2f(originVS.x - (step + width), originVS.y);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(originVS.x + height, originVS.y + step);
    glVertex2f(originVS.x - height, originVS.y + step);
    glVertex2f(originVS.x, originVS.y + step + width);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawTargetCursor(const MPoint& originVS, const MColor& color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    GLfloat width = 8;
    GLfloat space = 2;
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(originVS.x - width, originVS.y);
    glVertex2f(originVS.x - space, originVS.y);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(originVS.x + space, originVS.y);
    glVertex2f(originVS.x + width, originVS.y);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(originVS.x, originVS.y + width);
    glVertex2f(originVS.x, originVS.y + space);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(originVS.x, originVS.y - space);
    glVertex2f(originVS.x, originVS.y - width);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawExtendCursorItem(const MPoint& originVS, const MColor& color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    GLfloat width = 4;
    // Cross shape
    glLineWidth(1.f);
    glBegin(GL_LINES);
    glVertex2f(originVS.x - width, originVS.y);
    glVertex2f(originVS.x + width, originVS.y);
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(originVS.x, originVS.y - width);
    glVertex2f(originVS.x, originVS.y + width);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPointCloudCursorItem(const MPoint& originVS, const MColor& color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    glPointSize(2.f);
    glBegin(GL_POINTS);
    glVertex2f(originVS.x, originVS.y);
    glVertex2f(originVS.x + 2, originVS.y + 4);
    glVertex2f(originVS.x - 2, originVS.y + 4);
    glVertex2f(originVS.x + 4, originVS.y);
    glVertex2f(originVS.x - 4, originVS.y);
    glVertex2f(originVS.x + 2, originVS.y - 4);
    glVertex2f(originVS.x - 2, originVS.y - 4);
    glEnd();
    glPopAttrib();
}

// static
void MVGDrawUtil::drawPlaneCursorItem(const MPoint& originVS, const MColor& color)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glColor3f(color.r, color.g, color.b);
    GLfloat width = 3;
    GLfloat height = 3;
    GLfloat step = 3;
    glLineWidth(1.f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(originVS.x + width + step, originVS.y + height);
    glVertex2f(originVS.x - width, originVS.y + height);
    glVertex2f(originVS.x - width - step, originVS.y - height);
    glVertex2f(originVS.x + width, originVS.y - height);
    glEnd();
    glPopAttrib();
}

void MVGDrawUtil::drawLocatorCursorItem(const MPoint& originVS)
{
    const MColor red = MColor(1.0, 0.0, 0.0, 1.0);
    const MColor green = MColor(0.0, 1.0, 0.0, 0.0);
    const MColor blue = MColor(0.0, 0.0, 1.0);

    GLfloat width = 10;
    GLfloat lineWidth = 1.0;

    const MPoint A(originVS.x, originVS.y);
    const MPoint B(originVS.x, originVS.y + width);
    const MPoint C(originVS.x + width, originVS.y);
    const MPoint D(originVS.x + width / 4 * 3, originVS.y + width / 4 * 3);
    MVGDrawUtil::drawLine2D(A, B, green, lineWidth);
    MVGDrawUtil::drawLine2D(A, C, red, lineWidth);
    MVGDrawUtil::drawLine2D(A, D, blue, lineWidth);
}

// Create manipulator
// static
void MVGDrawUtil::drawClickedPoints(const MPointArray& clickedVSPoints, const MColor color)
{
    MVGDrawUtil::drawPoints2D(clickedVSPoints, color, 4.0);
    if(clickedVSPoints.length() == 2)
        MVGDrawUtil::drawLine2D(clickedVSPoints[0], clickedVSPoints[1], color, 3.0);
    // TODO : draw alpha poly
    if(clickedVSPoints.length() > 2)
        MVGDrawUtil::drawLineLoop2D(clickedVSPoints, color, 3.0);
}

// Move manipulator
// static
void MVGDrawUtil::drawTriangulatedPoint(M3dView& view, const MPoint& draggedVSPoint,
                                        const MPoint& worldPoint, MColor color)
{
    drawFullCross(draggedVSPoint, 10, 1.5f, color);
    drawLine2D(draggedVSPoint, MVGGeometryUtil::worldToViewSpace(view, worldPoint), color, 1.5f,
               1.f, true);
}

// static
void MVGDrawUtil::drawTriangulatedPoints(M3dView& view, const MPointArray& onPressWSPoints,
                                         const MPointArray& onDragVSPositions)
{
    assert(onPressWSPoints.length() == onDragVSPositions.length());
    if(onPressWSPoints.length() > 0)
        drawTriangulatedPoint(view, onDragVSPositions[0], onPressWSPoints[0],
                              MVGDrawUtil::_triangulateColor);

    if(onPressWSPoints.length() > 1)
        drawTriangulatedPoint(view, onDragVSPositions[1], onPressWSPoints[1],
                              MVGDrawUtil::_triangulateColor);
}

// static
void MVGDrawUtil::drawFinalWSPoints(const MPointArray& finalWSPositions,
                                    const MPointArray& onPressWSPositions)
{
    if(finalWSPositions.length() < 1)
        return;

    MVGDrawUtil::drawPoints3D(finalWSPositions, MColor(1, 0, 0));
    if(finalWSPositions.length() > 0)
        MVGDrawUtil::drawLine3D(onPressWSPositions[0], finalWSPositions[0], MColor(1, 0, 0), 1.5f,
                                1.f, true);
    if(finalWSPositions.length() > 1)
        MVGDrawUtil::drawLine3D(onPressWSPositions[1], finalWSPositions[1], MColor(1, 0, 0), 1.5f,
                                1.f, true);
}

} // namespace
