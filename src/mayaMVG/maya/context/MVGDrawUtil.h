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
};

} // namespace
