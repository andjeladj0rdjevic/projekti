#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <cmath>
#include <string>

struct Node {
    float x, y;
};

struct Element {
    int   n1, n2;
    float E;   // Pa (unosi se u GPa, konvertuje se)
    float A;   // m2 (unosi se u cm2, konvertuje se)
};

enum SupportType {
    FIXED,   // Nepokretni oslonac
    ROLLER   // Pokretni oslonac
};

struct Support {
    int         node;
    SupportType type;
    float       angle;  // radijani, visekatnik PI/4
};

struct Force {
    int   node;
    float magnitude; // N
    float angle;     // radijani, visekatnik PI/4
};

enum Mode {
    MODE_DRAW,
    MODE_FORCE,
    MODE_SUPPORT,
    MODE_MATERIAL
};

struct AppState {
    std::vector<Node>    nodes;
    std::vector<Element> elements;
    std::vector<Support> supports;
    std::vector<Force>   forces;

    Mode mode = MODE_DRAW;

    // Podrazumevane vrednosti za nove stapove
    // (azuriraju se nakon svakog terminalnog unosa)
    float currentE = 210e9f;  // Pa
    float currentA = 1e-3f;   // m2

    // Oslonac
    SupportType currentSupportType  = FIXED;
    float       currentSupportAngle = 0.0f;

    // Ugao poslednje potvrdjene sile (za inicijalni preview)
    float currentForceAngleDeg = 270.0f;  // podrazumevano nadole
};

extern AppState app;

float snapToGrid(float v);
float snapAngle(float angle);
int   findClosestNode(float x, float y);

void saveToFile();

#endif
