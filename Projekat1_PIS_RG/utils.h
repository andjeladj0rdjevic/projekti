#ifndef UTILS_H
#define UTILS_H
#include <GL/freeglut.h>
#include <vector>

void drawGrid();

void addForce(float mx, float my, float angle);
void drawForces();

void addSupport(float mx, float my, float angle);
void drawSupports();

struct Node {
    float x, y;

struct Element {
    int nodeStart;
    int nodeEnd;

extern Node nodes[100];
extern int nodeCount;
extern Element elements[200];
extern int elementCount;

void drawTruss();
void handleMouse(int button, int state, int x, int y);
int findClosestNode(float x, float y);
void addNode(float x, float y);
void addElement(int startIdx, int endIdx);

#endif
