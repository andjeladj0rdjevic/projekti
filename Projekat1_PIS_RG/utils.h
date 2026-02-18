#ifndef UTILS_H
#define UTILS_H

#include <GL/freeglut.h>
#include <vector>
#include <string>
#include <cmath>

// --- KONSTANTE I MODOVI ---
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum InteractionMode {
    MODE_BUILD,     // Crtanje cvorova i stapova
    MODE_FORCE,     // Dodavanje sila
    MODE_SUPPORT    // Dodavanje oslonaca
};

// --- STRUKTURE ---
struct Node {
    float x, y;
};

struct Element {
    int nodeStart;
    int nodeEnd;
};

struct Force {
    int nodeId;
    float magnitude;
    float angle; // U radijanima
};

struct Support {
    int nodeId;
    float angle; // U radijanima
    int type;    // 1 = nepokretni, 2 = pokretni (po potrebi)
};

// --- GLOBALNE PROMENLJIVE (EXTERN) ---
extern InteractionMode currentMode;

extern Node nodes[100];
extern int nodeCount;

extern Element elements[200];
extern int elementCount;

extern Force forces[100];
extern int forceCount;

extern Support supports[100];
extern int supportCount;

extern int selectedNode; // Za crtanje linije

// --- FUNKCIJE ---

// Crtanje
void drawGrid();
void drawTruss();
void drawForces();
void drawSupports();
void drawText(float x, float y, std::string text);

// Logika
int findClosestNode(float x, float y);
void addNode(float x, float y);
void addElement(int startIdx, int endIdx);

// Ove funkcije sada primaju ID cvora, jer mis odredjuje cvor
void addOrRotateForce(int nodeIdx);
void addOrRotateSupport(int nodeIdx);

// Input/Output
void showInputDialog();
void generateMKEFile();
void displayMessages();
void handleKeyboardExtended(unsigned char key, int x, int y);
void handleMouse(int button, int state, int x, int y);

#endif
