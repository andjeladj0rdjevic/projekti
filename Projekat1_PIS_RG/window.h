#ifndef WINDOW_H
#define WINDOW_H

#include <GL/freeglut.h>

void initWindow(int argc, char** argv);

void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void specialKeys(int key, int x, int y);
void handleMouse(int button, int state, int x, int y);

void drawGrid();
void drawTruss();
void drawForces();
void drawSupports();
void drawUI();

#endif
