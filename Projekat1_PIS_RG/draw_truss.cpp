#include "utils.h"
#include <iostream>
#include <iomanip>

// --- DEFINICIJA GLOBALNIH PROMENLJIVIH ---
InteractionMode currentMode = MODE_BUILD;

Node nodes[100];
int nodeCount = 0;

Element elements[200];
int elementCount = 0;

int selectedNode = -1;

// --- IMPLEMENTACIJA ---

void drawText(float x, float y, std::string text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

int findClosestNode(float x, float y) {
    const float threshold = 0.05f;
    for (int i = 0; i < nodeCount; i++) {
        float dx = x - nodes[i].x;
        float dy = y - nodes[i].y;
        if (sqrtf(dx * dx + dy * dy) < threshold) return i;
    }
    return -1;
}

void addNode(float x, float y) {
    if (nodeCount < 100) {
        // Snap to grid (0.1)
        float snappedX = roundf(x * 10.0f) / 10.0f;
        float snappedY = roundf(y * 10.0f) / 10.0f;

        // Provera da li cvor vec postoji na tom mestu
        if (findClosestNode(snappedX, snappedY) != -1) return;

        nodes[nodeCount].x = snappedX;
        nodes[nodeCount].y = snappedY;
        nodeCount++;
    }
}

void addElement(int startIdx, int endIdx) {
    if (elementCount < 200 && startIdx != endIdx) {
        // Provera duplikata
        for(int i=0; i<elementCount; i++) {
            if((elements[i].nodeStart == startIdx && elements[i].nodeEnd == endIdx) ||
               (elements[i].nodeStart == endIdx && elements[i].nodeEnd == startIdx))
               return;
        }
        elements[elementCount].nodeStart = startIdx;
        elements[elementCount].nodeEnd = endIdx;
        elementCount++;
    }
}

void drawTruss() {
    glLineWidth(2.0f);
    glColor3f(0.0f, 0.0f, 0.0f); // Crna boja za stapove

    glBegin(GL_LINES);
    for (int i = 0; i < elementCount; i++) {
        Node n1 = nodes[elements[i].nodeStart];
        Node n2 = nodes[elements[i].nodeEnd];
        glVertex2f(n1.x, n1.y);
        glVertex2f(n2.x, n2.y);
    }
    glEnd();

    // Crtanje duzina stapova
    glColor3f(0.3f, 0.3f, 0.3f);
    for (int i = 0; i < elementCount; i++) {
        Node n1 = nodes[elements[i].nodeStart];
        Node n2 = nodes[elements[i].nodeEnd];
        float midX = (n1.x + n2.x) / 2.0f;
        float midY = (n1.y + n2.y) / 2.0f;
        float len = sqrtf(pow(n2.x - n1.x, 2) + pow(n2.y - n1.y, 2));

        // Formatiranje na 2 decimale
        std::string lenStr = std::to_string(len);
        lenStr = lenStr.substr(0, lenStr.find(".") + 3);
        drawText(midX + 0.01f, midY + 0.01f, lenStr);
    }

    // Crtanje cvorova
    glPointSize(8.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < nodeCount; i++) {
        if (i == selectedNode) glColor3f(1.0f, 0.0f, 0.0f); // Crveno ako je selektovan
        else glColor3f(0.0f, 0.0f, 1.0f); // Plavo inace
        glVertex2f(nodes[i].x, nodes[i].y);
    }
    glEnd();

    // ID cvorova
    glColor3f(0.0f, 0.0f, 0.5f);
    for (int i = 0; i < nodeCount; i++) {
        drawText(nodes[i].x + 0.02f, nodes[i].y + 0.02f, std::to_string(i+1));
    }
}

// GLAVNA MOUSE FUNKCIJA
void handleMouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;

    // Konverzija koordinata
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    float aspect = (float)w / h;

    float worldX, worldY;

    if (aspect >= 1.0f) {
        worldX = ((float)x / w * 2.0f - 1.0f) * aspect;
        worldY = 1.0f - (float)y / h * 2.0f;
    } else {
        worldX = (float)x / w * 2.0f - 1.0f;
        worldY = (1.0f - (float)y / h * 2.0f) / aspect; // Ispravka za vertikalni aspect
    }

    int clickedNode = findClosestNode(worldX, worldY);

    if (currentMode == MODE_BUILD) {
        if (button == GLUT_LEFT_BUTTON) {
            if (clickedNode == -1) addNode(worldX, worldY);
        }
        else if (button == GLUT_RIGHT_BUTTON) {
            if (clickedNode != -1) {
                if (selectedNode == -1) selectedNode = clickedNode;
                else {
                    addElement(selectedNode, clickedNode);
                    selectedNode = -1;
                }
            } else {
                selectedNode = -1; // Deselektuj ako kliknes u prazno
            }
        }
    }
    else if (currentMode == MODE_FORCE) {
        if (button == GLUT_LEFT_BUTTON && clickedNode != -1) {
            addOrRotateForce(clickedNode);
        }
    }
    else if (currentMode == MODE_SUPPORT) {
        if (button == GLUT_LEFT_BUTTON && clickedNode != -1) {
            addOrRotateSupport(clickedNode);
        }
    }

    glutPostRedisplay();
}
