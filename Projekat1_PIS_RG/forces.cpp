#include <GL/freeglut.h>
#include <cmath>

struct Node {
    float x, y;
};

extern Node nodes[100];
extern int nodeCount;

struct Force {
    int nodeId;
    float magnitude;
    float angle;
};

static Force forces[100];
static int forceCount = 0;

int findNode(float mx, float my)
{
    const float eps = 0.05f;

    for (int i = 0; i < nodeCount; i++) {
        float dx = mx - nodes[i].x;
        float dy = my - nodes[i].y;
        if (dx * dx + dy * dy < eps * eps)
            return i;
    }
    return -1;
}

float snapAngle(float angle)
{
    float step = M_PI / 4.0f;
    return round(angle / step) * step;
}

void addForce(float mx, float my, float angle)
{
    int node = findNode(mx, my);
    if (node == -1)
        return;

    forces[forceCount].nodeId = node;
    forces[forceCount].magnitude = 1.0f;
    forces[forceCount].angle = snapAngle(angle);
    forceCount++;
}

void drawForces()
{
    glColor3f(1.0f, 0.0f, 0.0f);

    for (int i = 0; i < forceCount; i++) {
        Node n = nodes[forces[i].nodeId];

        float dx = cos(forces[i].angle) * 0.15f;
        float dy = sin(forces[i].angle) * 0.15f;

        glBegin(GL_LINES);
            glVertex2f(n.x, n.y);
            glVertex2f(n.x + dx, n.y + dy);
        glEnd();
    }
}
