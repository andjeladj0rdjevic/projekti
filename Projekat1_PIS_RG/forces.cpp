#include "utils.h"
#include <cmath>

Force forces[100];
int forceCount = 0;

void addOrRotateForce(int nodeIdx) {
    // 1. Proveri da li sila na ovom cvoru vec postoji
    for (int i = 0; i < forceCount; i++) {
        if (forces[i].nodeId == nodeIdx) {
            // Postoji -> Rotiraj je za 45 stepeni (PI/4)
            forces[i].angle -= (M_PI / 4.0f);
            return;
        }
    }

    // 2. Ne postoji -> Dodaj novu silu (default dole, 270 stepeni)
    if (forceCount < 100) {
        forces[forceCount].nodeId = nodeIdx;
        forces[forceCount].magnitude = 10.0f; // Primer vrednost
        forces[forceCount].angle = 3.0f * M_PI / 2.0f; // Dole
        forceCount++;
    }
}

void drawForces() {
    glColor3f(1.0f, 0.0f, 0.0f); // Crvena boja za sile
    glLineWidth(2.0f);

    float scale = 0.2f; // Duzina strelice

    for (int i = 0; i < forceCount; i++) {
        Node n = nodes[forces[i].nodeId];

        float dx = cosf(forces[i].angle) * scale;
        float dy = sinf(forces[i].angle) * scale;

        // Crtanje linije sile (od vrha ka cvoru ili obrnuto, ovde crtamo da "udara" u cvor)
        float startX = n.x - dx;
        float startY = n.y - dy;

        glBegin(GL_LINES);
            glVertex2f(startX, startY);
            glVertex2f(n.x, n.y);
        glEnd();

        // Crtanje vrha strelice (kod cvora)
        float arrowSize = 0.05f;
        float angle1 = forces[i].angle + M_PI - 0.5f;
        float angle2 = forces[i].angle + M_PI + 0.5f;

        glBegin(GL_TRIANGLES);
            glVertex2f(n.x, n.y);
            glVertex2f(n.x + cosf(angle1) * arrowSize, n.y + sinf(angle1) * arrowSize);
            glVertex2f(n.x + cosf(angle2) * arrowSize, n.y + sinf(angle2) * arrowSize);
        glEnd();
    }
}
