#include "window.h"
#include "utils.h"

// Dimenzije prozora
static int windowWidth  = 800;
static int windowHeight = 800;

void display()
{
    glClearColor(0.95f, 0.95f, 0.95f, 1.0f); // Svetlo siva pozadina
    glClear(GL_COLOR_BUFFER_BIT);

    drawGrid();
    drawTruss();
    drawSupports();    // Oslonci pre sila
    drawForces();
    displayMessages();

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    windowWidth = w;
    windowHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)w / (float)h;

    // Odrzavanje aspect ratia da se slika ne deformise
    if (aspect >= 1.0f)
        gluOrtho2D(-aspect, aspect, -1.0, 1.0);
    else
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);

    glMatrixMode(GL_MODELVIEW);
}

void initWindow(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("2D Resetka - Postavka problema");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    // Povezivanje input funkcija iz data_input.cpp i utils.h
    glutKeyboardFunc(handleKeyboardExtended);
    glutMouseFunc(handleMouse);

    // Podesavanje osnovnih GL parametara
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}
