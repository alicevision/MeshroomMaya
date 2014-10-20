#pragma once

#include <maya/M3dView.h>
#include <maya/MPointArray.h>

namespace mayaMVG
{

struct MVGDrawUtil
{

    static void begin2DDrawing(int portWidth, int portHeight)
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

    static void end2DDrawing()
    {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopAttrib();
        glPopMatrix();
    }

    static void drawLine2D(const MPoint& A, const MPoint& B, const MColor& color,
                           const float lineWidth = 1.5f, const float alpha = 1.f,
                           bool stipple = false)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        if(stipple)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1.f, 0x5555);
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

    static void drawLine3D(const MPoint& A, const MPoint& B, const MColor& color,
                           const float lineWidth = 1.5f, const float alpha = 1.f,
                           bool stipple = false)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        if(stipple)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1.f, 0x5555);
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

    static void drawLineLoop2D(const MPointArray& points, const MColor& color,
                               const float lineWidth = 1.f, const float alpha = 1.f)
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

    static void drawLineLoop3D(const MPointArray& points, const MColor& color,
                               const float lineWidth = 1.f, const float alpha = 1.f)
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

    static void drawPolygon2D(const MPointArray& points, const MColor& color,
                              const float alpha = 1.f)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor4f(color.r, color.g, color.b, alpha);
        glBegin(GL_POLYGON);
        for(int i = 0; i < points.length(); ++i)
            glVertex2f(points[i].x, points[i].y);
        glEnd();
        glPopAttrib();
    }

    static void drawPolygon3D(const MPointArray& points, const MColor& color,
                              const float alpha = 1.f)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor4f(color.r, color.g, color.b, alpha);
        glBegin(GL_POLYGON);
        for(int i = 0; i < points.length(); ++i)
            glVertex3f(points[i].x, points[i].y, points[i].z);
        glEnd();
        glPopAttrib();
    }

    static void drawPoints2D(const MPointArray& points, const MColor& color,
                             const float pointSize = 1.f, const float alpha = 1.f)
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

    static void drawCircle2D(const MPoint& center, const MColor& color, const int r,
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

    static void drawCircle3D(const MPoint& center, const MColor& color, const int r,
                             const int segments)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        glLineWidth(1.5f);
        glBegin(GL_LINE_LOOP);
        for(int n = 0; n <= segments; ++n)
        {
            float const t = 2 * M_PI * (float)n / (float)segments;
            glVertex3f(center.x + sin(t) * r, center.y + cos(t) * r, center.z);
        }
        glEnd();
        glPopAttrib();
    }

    static void drawEmptyCross(const float x, const float y, const float width,
                               const float thickness, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + width, y - thickness);
        glVertex2f(x + width, y + thickness);
        glVertex2f(x + thickness, y + thickness);
        glVertex2f(x + thickness, y + width);
        glVertex2f(x - thickness, y + width);
        glVertex2f(x - thickness, y + thickness);
        glVertex2f(x - width, y + thickness);
        glVertex2f(x - width, y - thickness);
        glVertex2f(x - thickness, y - thickness);
        glVertex2f(x - thickness, y - width);
        glVertex2f(x + thickness, y - width);
        glVertex2f(x + thickness, y - thickness);
        glEnd();
        glPopAttrib();
    }

    static void drawFullCross(const float x, const float y, const float width,
                              const float thickness, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        glBegin(GL_POLYGON);
        glVertex2f(x + thickness, y - width);
        glVertex2f(x + thickness, y + width);
        glVertex2f(x - thickness, y + width);
        glVertex2f(x - thickness, y - width);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(x + width, y + thickness);
        glVertex2f(x - width, y + thickness);
        glVertex2f(x - width, y - thickness);
        glVertex2f(x + width, y - thickness);
        glEnd();
        glPopAttrib();
    }

    static void drawArrowsCursor(const float x, const float y, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        GLfloat step = 8;
        GLfloat width = 4;
        GLfloat height = 4;

        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex2f(x - step, y);
        glVertex2f(x + step, y);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x, y - step);
        glVertex2f(x, y + step);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex2f(x + step, y + height);
        glVertex2f(x + step, y - height);
        glVertex2f(x + step + width, y);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(x + height, y - step);
        glVertex2f(x - height, y - step);
        glVertex2f(x, y - (step + width));
        glEnd();

        glBegin(GL_POLYGON);
        glVertex2f(x - step, y + height);
        glVertex2f(x - step, y - height);
        glVertex2f(x - (step + width), y);
        glEnd();

        glBegin(GL_POLYGON);
        glVertex2f(x + height, y + step);
        glVertex2f(x - height, y + step);
        glVertex2f(x, y + step + width);
        glEnd();
        glPopAttrib();
    }

    static void drawTargetCursor(const float x, const float y, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        GLfloat width = 8;
        GLfloat space = 2;
        glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex2f(x - width, y);
        glVertex2f(x - space, y);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x + space, y);
        glVertex2f(x + width, y);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x, y + width);
        glVertex2f(x, y + space);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x, y - space);
        glVertex2f(x, y - width);
        glEnd();
        glPopAttrib();
    }

    static void drawExtendItem(const float x, const float y, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        GLfloat width = 4;
        // Cross shape
        glLineWidth(1.f);
        glBegin(GL_LINES);
        glVertex2f(x - width, y);
        glVertex2f(x + width, y);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(x, y - width);
        glVertex2f(x, y + width);
        glEnd();
        glPopAttrib();
    }

    static void drawPointCloudItem(const float x, const float y, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        glPointSize(2.f);
        glBegin(GL_POINTS);
        glVertex2f(x, y);
        glVertex2f(x + 2, y + 4);
        glVertex2f(x - 2, y + 4);
        glVertex2f(x + 4, y);
        glVertex2f(x - 4, y);
        glVertex2f(x + 2, y - 4);
        glVertex2f(x - 2, y - 4);
        glEnd();
        glPopAttrib();
    }

    static void drawPlaneItem(const float x, const float y, const MColor& color)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glColor3f(color.r, color.g, color.b);
        GLfloat width = 3;
        GLfloat height = 3;
        GLfloat step = 3;
        glLineWidth(1.f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x + width + step, y + height);
        glVertex2f(x - width, y + height);
        glVertex2f(x - width - step, y - height);
        glVertex2f(x + width, y - height);
        glEnd();
        glPopAttrib();
    }
};

} // namespace
