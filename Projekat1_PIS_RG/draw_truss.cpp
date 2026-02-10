#include "utils.h"
#include <string>
#include <cmath>

// Inicijalizacija globalnih promenljivih
int nodeCount = 0;
int elementCount = 0;
Element elements[200];
int selectedNode = -1; // Za praćenje selekcije prvog čvora pri crtanju štapa

// Pomoćna funkcija za ispis ID-jeva i dužina [cite: 22]
void drawText(float x, float y, std::string text) {
    glRasterPos2f(x + 0.03f, y + 0.03f);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

// Pronalaženje čvora u blizini klika (tolerancija 0.05)
int findClosestNode(float x, float y) {
    const float threshold = 0.05f;
    for (int i = 0; i < nodeCount; i++) {
        float dx = x - nodes[i].x;
        float dy = y - nodes[i].y;
        if (sqrt(dx * dx + dy * dy) < threshold) return i;
    }
    return -1;
}

// Dodavanje čvora uz "Snap-to-Grid" logiku (mreža 0.1 x 0.1) 
void addNode(float x, float y) {
    if (nodeCount < 100) {
        float snappedX = round(x / 0.1f) * 0.1f;
        float snappedY = round(y / 0.1f) * 0.1f;

        if (findClosestNode(snappedX, snappedY) == -1) {
            nodes[nodeCount].x = snappedX;
            nodes[nodeCount].y = snappedY;
            nodeCount++;
        }
    }
}

// Povezivanje dva čvora štapom 
void addElement(int startIdx, int endIdx) {
    if (elementCount < 200 && startIdx != endIdx) {
        // Provera da li štap već postoji (sprečavanje duplikata)
        for (int i = 0; i < elementCount; i++) {
            if ((elements[i].nodeStart == startIdx && elements[i].nodeEnd == endIdx) ||
                (elements[i].nodeStart == endIdx && elements[i].nodeEnd == startIdx))
                return;
        }
        elements[elementCount].nodeStart = startIdx;
        elements[elementCount].nodeEnd = endIdx;
        elementCount++;
    }
}

// Glavna funkcija za crtanje rešetke [cite: 5, 11]
void drawTruss() {
    // 1. CRTANJE ŠTAPOVA
    glLineWidth(2.0f);
    for (int i = 0; i < elementCount; i++) {
        Node n1 = nodes[elements[i].nodeStart];
        Node n2 = nodes[elements[i].nodeEnd];
        float length = sqrt(pow(n2.x - n1.x, 2) + pow(n2.y - n1.y, 2));

        glColor3f(0.6f, 0.6f, 0.6f); // Siva boja štapova
        glBegin(GL_LINES);
            glVertex2f(n1.x, n1.y);
            glVertex2f(n2.x, n2.y);
        glEnd();

        // Prikaz dužine štapa [cite: 12]
        glColor3f(0.0f, 0.4f, 0.0f);
        drawText((n1.x + n2.x) / 2, (n1.y + n2.y) / 2, std::to_string(length).substr(0, 4));
    }

    // 2. CRTANJE ČVOROVA (ZGLOBNO SPOJENI) 
    glPointSize(10.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < nodeCount; i++) {
        if (i == selectedNode) glColor3f(1.0f, 0.5f, 0.0f); // Narandžasto ako je selektovan
        else glColor3f(0.0f, 0.0f, 0.0f); 
        glVertex2f(nodes[i].x, nodes[i].y);
    }
    glEnd();

    // 3. OZNAKE ČVOROVA (ID)
    glColor3f(0.0f, 0.0f, 1.0f);
    for (int i = 0; i < nodeCount; i++) {
        drawText(nodes[i].x, nodes[i].y, "N" + std::to_string(i));
    }
}

// Obrada unosa mišem [cite: 22]
void handleMouse(int button, int state, int x, int y) {
    if (state != GLUT_DOWN) return;

    // Normalizacija koordinata miša na opseg [-1, 1]
    float mouseX = (float)x / (glutGet(GLUT_WINDOW_WIDTH) / 2.0f) - 1.0f;
    float mouseY = 1.0f - (float)y / (glutGet(GLUT_WINDOW_HEIGHT) / 2.0f);

    if (button == GLUT_LEFT_BUTTON) {
        addNode(mouseX, mouseY); // Levi klik postavlja čvor na mrežu
    } 
    else if (button == GLUT_RIGHT_BUTTON) {
        int clickedNode = findClosestNode(mouseX, mouseY);
        if (clickedNode != -1) {
            if (selectedNode == -1) {
                selectedNode = clickedNode; // Prva selekcija
            } else {
                addElement(selectedNode, clickedNode); // Povezivanje
                selectedNode = -1; // Reset selekcije
            }
        }
    }
    glutPostRedisplay();
}
