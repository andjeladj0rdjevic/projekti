#include "utils.h"
#include <cmath>

Support supports[100];
int supportCount = 0;

void addOrRotateSupport(int nodeIdx) {
    // 1. Proveri da li oslonac postoji
    for (int i = 0; i < supportCount; i++) {
        if (supports[i].nodeId == nodeIdx) {
            // Rotiraj za 45 stepeni
            supports[i].angle += (M_PI / 4.0f);
            return;
        }
    }

    // 2. Dodaj novi
    if (supportCount < 100) {
        supports[supportCount].nodeId = nodeIdx;
        supports[supportCount].angle = 0.0f; // Default gore
        supports[supportCount].type = 1;     // Nepokretni
        supportCount++;
    }
}

void drawSupports() {
    glColor3f(0.0f, 0.5f, 0.0f); // Zelena boja za oslonce
    float size = 0.08f;

    for (int i = 0; i < supportCount; i++) {
        Node n = nodes[supports[i].nodeId];

        glPushMatrix();
        glTranslatef(n.x, n.y, 0.0f);
        glRotatef(supports[i].angle * 180.0f / M_PI, 0.0f, 0.0f, 1.0f);

        // Crtanje trougla
        glBegin(GL_LINE_LOOP);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(-size/2, -size);
            glVertex2f(size/2, -size);
        glEnd();

        // Crtanje "zemlje" ispod trougla
        glBegin(GL_LINES);
            glVertex2f(-size, -size);
            glVertex2f(size, -size);
            // Srafura
            for(float k=-size; k<size; k+=size/3) {
                glVertex2f(k, -size);
                glVertex2f(k-0.02f, -size-0.02f);
            }
        glEnd();

        glPopMatrix();
    }
}
