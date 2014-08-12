#pragma once

namespace mayaMVG {

struct MVGDrawUtil {
	
	static void begin2DDrawing(M3dView& view) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, view.portWidth(), 0, view.portHeight(), -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}

	static void end2DDrawing() {
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glPopAttrib();
	}

    static void drawLine2D(const MPoint& A, const MPoint& B, const MVector& color, const float lineWidth=1.5f, const float alpha=1.f, bool stipple=false)
    {
        if(stipple)
        {
            glEnable(GL_LINE_STIPPLE);
            glLineStipple(1.f, 0x5555);  
        }
        
        glColor4f(color.x, color.y, color.z, alpha);
        glLineWidth(lineWidth);
        glBegin(GL_LINES);
            glVertex2f(A.x, A.y);
            glVertex2f(B.x, B.y);
        glEnd();	
        
        if(stipple)
            glDisable(GL_LINE_STIPPLE);
    }

    static void drawLineLoop2D(const MPointArray& points, const MVector& color, const float lineWidth=1.f, const float alpha=1.f)
    {
        glColor4f(color.x, color.y, color.z, alpha);
        glLineWidth(lineWidth);
		glBegin(GL_LINE_LOOP);
			for(int i = 0; i < points.length(); ++i) {			
				glVertex2f(points[i].x, points[i].y);
			}
		glEnd();
    }

    static void drawPolygon2D(const MPointArray& points, const MVector& color, const float alpha=1.f)
    {
        glColor4f(color.x, color.y, color.z, alpha);
		glBegin(GL_POLYGON);
			for(int i = 0; i < points.length(); ++i) {			
				glVertex2f(points[i].x, points[i].y);
			}
		glEnd();
    }

    static void drawLineLoop3D(const MPointArray& points, const MVector& color, const float lineWidth=1.f, const float alpha=1.f)
    {
        glColor4f(color.x, color.y, color.z, alpha);
        glLineWidth(lineWidth);
		glBegin(GL_LINE_LOOP);
			for(int i = 0; i < points.length(); ++i) {			
				glVertex3f(points[i].x, points[i].y, points[i].z);
			}
		glEnd();
    }

    static void drawPolygon3D(const MPointArray& points, const MVector& color, const float alpha=1.f)
    {
        glColor4f(color.x, color.y, color.z, alpha);
		glBegin(GL_POLYGON);
			for(int i = 0; i < points.length(); ++i) {			
				glVertex3f(points[i].x, points[i].y, points[i].z);
			}
		glEnd();
    }

	static void drawCircle2D(const MPoint& center, const MVector& color, const int r, const int segments) {
        glColor3f(color.x, color.y, color.z);
		glLineWidth(1.5f);
		glBegin(GL_LINE_LOOP);
			for( int n = 0; n <= segments; ++n ) {
				float const t = 2*M_PI*(float)n/(float)segments;
				glVertex2f(center.x + sin(t)*r, center.y + cos(t)*r);
			}
		glEnd();
	}

	static void drawEmptyCross(const float x, const float y, const float width, const float thickness, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}

	static void drawFullCross(const float x, const float y, const float width, const float thickness, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}

	static void drawArrowsCursor(const float x, const float y, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}

	static void drawTargetCursor(const float x, const float y, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}

	static void drawExtendItem(const float x, const float y, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}

	static void drawPointCloudItem(const float x, const float y, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}

	static void drawPlaneItem(const float x, const float y, const MVector& color)
	{
        glColor3f(color.x, color.y, color.z);
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
	}
};

} // namespace
