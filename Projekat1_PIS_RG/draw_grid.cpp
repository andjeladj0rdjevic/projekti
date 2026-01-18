#include <GL/freeglut.h>
#include "utils.h"

void drawGrid()
{
    const float step = 0.1f;

    glColor3f(0.85f, 0.85f, 0.85f);
    glLineWidth(1.0f);

    glBegin(GL_LINES);

    for (float i = -1.0f; i <= 1.0f; i += step)
    {
        // vertical lines
        glVertex2f(i, -1.0f);
        glVertex2f(i,  1.0f);

        // horizontal lines
        glVertex2f(-1.0f, i);
        glVertex2f( 1.0f, i);
    }

    glEnd();

    // axes
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
        glVertex2f(-1.0f, 0.0f);
        glVertex2f( 1.0f, 0.0f);

        glVertex2f(0.0f, -1.0f);
        glVertex2f(0.0f,  1.0f);
    glEnd();
}
