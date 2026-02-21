#include "utils.h"

AppState app;

float snapToGrid(float v)
{
    float grid = 1.0f;
    return round(v / grid) * grid;
}

float snapAngle(float angle)
{
    float step = M_PI / 4.0f;
    return round(angle / step) * step;
}

int findClosestNode(float x, float y)
{
    float threshold = 0.3f;
    for (int i = 0; i < app.nodes.size(); i++)
    {
        float dx = app.nodes[i].x - x;
        float dy = app.nodes[i].y - y;
        if (sqrt(dx*dx + dy*dy) < threshold)
            return i;
    }
    return -1;
}
