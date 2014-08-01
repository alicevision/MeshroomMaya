#pragma once

namespace mayaMVG {

struct MVGDrawUtil {
	
	static void drawCircle(int x, int y, int r, int segments) {
		glLineWidth(1.5f);
		glBegin(GL_LINE_LOOP);
			for( int n = 0; n <= segments; ++n ) {
				float const t = 2*M_PI*(float)n/(float)segments;
				glVertex2f(x + sin(t)*r, y + cos(t)*r);
			}
		glEnd();
	}
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
	
	static void drawEmptyCross(float x, float y, float width, float thickness)
	{
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

	static void drawFullCross(float x, float y, float width, float thickness)
	{
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
	
	static void drawArrowsCursor(float x, float y)
	{
		GLfloat step = 8;
		GLfloat width = 4;
		GLfloat height = 4;
		
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
	
	static void drawPointCloudItem(float x, float y)
	{
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
	
	static void drawPlaneItem(float x, float y)
	{
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
