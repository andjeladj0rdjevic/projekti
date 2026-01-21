#include <GL/freeglut.h>
#include <cmath>

struct Node {
    float x, y;
};

extern Node nodes[100];
extern int nodeCount;

struct Support {
    int nodeId;
    float angle;
};

static Support supports[100];
static int supportCount = 0;

float snapAngle(float angle)
{
    float step = M_PI / 4.0f;
    return round(angle / step) * step;
}

void addSupport(float mx, float my, float angle)
{
    int node = -1;
    const float eps = 0.05f;

    for (int i = 0; i < nodeCount; i++) {
        float dx = mx - nodes[i].x;
        float dy = my - nodes[i].y;
        if (dx * dx + dy * dy < eps * eps)
            node = i;
    }

    if (node == -1)
        return;

    supports[supportCount].nodeId = node;
    supports[supportCount].angle = snapAngle(angle);
    supportCount++;
}

void drawSupports()
{
    glColor3f(0.0f, 0.0f, 1.0f);

    for (int i = 0; i < supportCount; i++) {
        Node n = nodes[supports[i].nodeId];
        float s = 0.05f;

        glBegin(GL_TRIANGLES);
            glVertex2f(n.x, n.y);
            glVertex2f(n.x - s, n.y - s);
            glVertex2f(n.x + s, n.y - s);
        glEnd();
    }
}
