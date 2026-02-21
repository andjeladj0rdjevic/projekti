#include "window.h"
#include "utils.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

// ─────────────────────────────────────────────
//  Stanje prozora
// ─────────────────────────────────────────────
static int   windowWidth  = 800;
static int   windowHeight = 800;
static float camX = 0.0f, camY = 0.0f, camZoom = 1.0f;

// RMB crtanje stapa
static int rmb_firstNode = -1;

// Pending sila (rotacija strelice pre potvrde)
static int   pendingForceNode     = -1;
static float pendingForceAngleDeg = 270.0f; // podrazumevano nadole

// Pending oslonac (rotacija pre potvrde)
static int   pendingSupNode     = -1;
static float pendingSupAngleDeg = 270.0f; // podrazumevano nadole

// Sprecava pokretanje vise unosa odjednom
static std::atomic<bool> waitingForInput(false);
static std::mutex        appMutex;

// ─────────────────────────────────────────────
//  Labele cvorova (A, B, ..., Z, AA, AB, ...)
// ─────────────────────────────────────────────
static std::string nodeLabel(int i)
{
    std::string s;
    do {
        s = (char)('A' + (i % 26)) + s;
        i = i / 26 - 1;
    } while (i >= 0);
    return s;
}

// ─────────────────────────────────────────────
//  Terminalni unos — pokrece se u posebnoj niti
// ─────────────────────────────────────────────
static void askElementProps(int elemIdx)
{
    if (waitingForInput.exchange(true)) return; // vec ceka unos

    std::thread([elemIdx]()
    {
        double E_GPa = 210.0, A_cm2 = 10.0;

        printf("\n");
        printf("  ╔══════════════════════════════════════╗\n");
        printf("  ║   Stap %d  (cvorovi %s–%s)%*s║\n",
               elemIdx + 1,
               nodeLabel(app.elements[elemIdx].n1).c_str(),
               nodeLabel(app.elements[elemIdx].n2).c_str(),
               (int)(26 - nodeLabel(app.elements[elemIdx].n1).size()
                        - nodeLabel(app.elements[elemIdx].n2).size()), "");
        printf("  ╚══════════════════════════════════════╝\n");
        printf("  Unesite modul elasticnosti E [GPa]: ");
        fflush(stdout);
        if (scanf("%lf", &E_GPa) != 1) E_GPa = 210.0;

        printf("  Unesite povrsinu poprecnog preseka A [cm^2]: ");
        fflush(stdout);
        if (scanf("%lf", &A_cm2) != 1) A_cm2 = 10.0;
        printf("\n");

        {
            std::lock_guard<std::mutex> lock(appMutex);
            if (elemIdx < (int)app.elements.size()) {
                app.elements[elemIdx].E = (float)(E_GPa * 1e9);
                app.elements[elemIdx].A = (float)(A_cm2 * 1e-4);
            }
        }

        waitingForInput = false;
        glutPostRedisplay();
    }).detach();
}

static void askForceProps(int nodeIdx, float angleDeg)
{
    if (waitingForInput.exchange(true)) return;

    std::thread([nodeIdx, angleDeg]()
    {
        double F = 10000.0;

        printf("\n");
        printf("  ╔══════════════════════════════════════╗\n");
        printf("  ║   Sila na cvoru %-22s║\n", nodeLabel(nodeIdx).c_str());
        printf("  ╚══════════════════════════════════════╝\n");
        printf("  Smer sile: %.0f deg\n", angleDeg);
        printf("  Unesite intenzitet sile F [N]: ");
        fflush(stdout);
        if (scanf("%lf", &F) != 1) F = 10000.0;
        printf("\n");

        {
            std::lock_guard<std::mutex> lock(appMutex);
            Force f;
            f.node      = nodeIdx;
            f.magnitude = (float)F;
            f.angle     = angleDeg * (float)M_PI / 180.0f;
            app.forces.push_back(f);
        }

        waitingForInput = false;
        glutPostRedisplay();
    }).detach();
}

static void askSupportType(int nodeIdx, float angle)
{
    if (waitingForInput.exchange(true)) return;

    std::thread([nodeIdx, angle]()
    {
        char odgovor[16] = "ne";

        printf("\n");
        printf("  ╔══════════════════════════════════════╗\n");
        printf("  ║   Oslonac na cvoru %-19s║\n", nodeLabel(nodeIdx).c_str());
        printf("  ╚══════════════════════════════════════╝\n");
        printf("  Da li je oslonac pokretni? (da/ne): ");
        fflush(stdout);
        if (scanf("%15s", odgovor) != 1) {}
        printf("\n");

        SupportType tip = FIXED;
        if (odgovor[0] == 'd' || odgovor[0] == 'D') tip = ROLLER;

        {
            std::lock_guard<std::mutex> lock(appMutex);
            Support s;
            s.node  = nodeIdx;
            s.type  = tip;
            s.angle = angle;
            app.supports.push_back(s);
        }

        waitingForInput = false;
        glutPostRedisplay();
    }).detach();
}

// ─────────────────────────────────────────────
//  piksel → svet
// ─────────────────────────────────────────────
static void screenToWorld(int sx, int sy, float& wx, float& wy)
{
    float aspect = (float)windowWidth / (float)windowHeight;
    float halfH  = 10.0f / camZoom;
    float halfW  = halfH * aspect;
    wx = camX + ((float)sx / windowWidth)  * 2.0f * halfW - halfW;
    wy = camY - (((float)sy / windowHeight) * 2.0f * halfH - halfH);
}

// ─────────────────────────────────────────────
//  drawGrid
// ─────────────────────────────────────────────
void drawGrid()
{
    float aspect = (float)windowWidth / (float)windowHeight;
    float halfH  = 10.0f / camZoom;
    float halfW  = halfH * aspect;
    float xMin = camX - halfW, xMax = camX + halfW;
    float yMin = camY - halfH, yMax = camY + halfH;

    glColor3f(0.88f, 0.88f, 0.88f);
    glBegin(GL_LINES);
    for (float x = floorf(xMin); x <= xMax; x += 1.0f) {
        glVertex2f(x, yMin); glVertex2f(x, yMax);
    }
    for (float y = floorf(yMin); y <= yMax; y += 1.0f) {
        glVertex2f(xMin, y); glVertex2f(xMax, y);
    }
    glEnd();

    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(xMin, 0.0f); glVertex2f(xMax, 0.0f);
    glVertex2f(0.0f, yMin); glVertex2f(0.0f, yMax);
    glEnd();
    glLineWidth(1.0f);

    glColor3f(0.45f, 0.45f, 0.45f);
    for (float x = floorf(xMin); x <= xMax; x += 1.0f) {
        if (fabsf(x) < 0.1f) continue;
        char buf[8]; snprintf(buf, sizeof(buf), "%.0f", x);
        glRasterPos2f(x + 0.05f, 0.1f);
        for (const char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
    for (float y = floorf(yMin) + 1.0f; y <= yMax; y += 1.0f) {
        if (fabsf(y) < 0.1f) continue;
        char buf[8]; snprintf(buf, sizeof(buf), "%.0f", y);
        glRasterPos2f(0.1f, y);
        for (const char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
}

// Forward deklaracija (definicija je u drawSupports sekciji)
static void drawSupportGeometry(SupportType type);

// ─────────────────────────────────────────────
//  drawTruss
// ─────────────────────────────────────────────
void drawTruss()
{
    // Stapovi (oznaka: broj iznad sredine)
    for (int i = 0; i < (int)app.elements.size(); i++) {
        const Element& e = app.elements[i];
        float x1 = app.nodes[e.n1].x, y1 = app.nodes[e.n1].y;
        float x2 = app.nodes[e.n2].x, y2 = app.nodes[e.n2].y;

        glColor3f(0.15f, 0.15f, 0.15f);
        glLineWidth(2.5f);
        glBegin(GL_LINES);
        glVertex2f(x1, y1); glVertex2f(x2, y2);
        glEnd();
        glLineWidth(1.0f);

        float mx = (x1+x2)/2.0f, my = (y1+y2)/2.0f;

        // Samo broj stapa iznad sredine
        char numBuf[8]; snprintf(numBuf, sizeof(numBuf), "%d", i+1);
        glColor3f(0.0f, 0.0f, 0.0f);
        glRasterPos2f(mx + 0.06f, my + 0.18f);
        for (const char* c = numBuf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Cvorovi (oznaka: slovo iznad)
    for (int i = 0; i < (int)app.nodes.size(); i++) {
        float x = app.nodes[i].x, y = app.nodes[i].y;
        bool  sel  = (app.mode == MODE_DRAW && i == rmb_firstNode);
        bool  pend = (app.mode == MODE_FORCE  && i == pendingForceNode) ||
                     (app.mode == MODE_SUPPORT && i == pendingSupNode);
        int   segs = 20;

        // Boja cvora
        if (pend)
            glColor3f(0.9f, 0.5f, 0.0f);         // narandzast = pending sila
        else if (sel)
            glColor3f(1.0f, 0.65f, 0.0f);         // zut = selektovan za stap
        else
            glColor3f(0.1f, 0.45f, 0.9f);         // plav = normalan

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x, y);
        for (int s = 0; s <= segs; s++) {
            float a = s * 2.0f * (float)M_PI / segs;
            glVertex2f(x + 0.14f*cosf(a), y + 0.14f*sinf(a));
        }
        glEnd();

        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
        for (int s = 0; s < segs; s++) {
            float a = s * 2.0f * (float)M_PI / segs;
            glVertex2f(x + 0.14f*cosf(a), y + 0.14f*sinf(a));
        }
        glEnd();

        // Slovo cvora iznad
        std::string lbl = nodeLabel(i);
        glColor3f(0.05f, 0.05f, 0.55f);
        glRasterPos2f(x - 0.08f, y + 0.22f);
        for (const char* c = lbl.c_str(); *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Preview rotacije oslonca — isti simbol, narandzast, isprekidan
    if (pendingSupNode >= 0 && pendingSupNode < (int)app.nodes.size()) {
        float x = app.nodes[pendingSupNode].x;
        float y = app.nodes[pendingSupNode].y;

        glPushMatrix();
        glTranslatef(x, y, 0.0f);
        glRotatef(pendingSupAngleDeg, 0.0f, 0.0f, 1.0f);

        glColor3f(0.9f, 0.5f, 0.0f);
        glLineWidth(2.0f);
        glLineStipple(4, 0xAAAA);
        glEnable(GL_LINE_STIPPLE);
        // Tip ce biti odredjen u terminalu — prikazujemo FIXED kao default preview
        drawSupportGeometry(FIXED);
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0f);
        glPopMatrix();

        // Ugao
        char buf[32]; snprintf(buf, sizeof(buf), "%.0f deg", pendingSupAngleDeg);
        glColor3f(0.7f, 0.35f, 0.0f);
        glRasterPos2f(x + 0.6f, y - 0.8f);
        for (const char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Preview strelica za pending silu
    if (pendingForceNode >= 0 && pendingForceNode < (int)app.nodes.size()) {
        float x   = app.nodes[pendingForceNode].x;
        float y   = app.nodes[pendingForceNode].y;
        float rad = pendingForceAngleDeg * (float)M_PI / 180.0f;
        float dx  = cosf(rad), dy = sinf(rad);
        float L   = 1.2f;

        // Isprekidana linija (preview)
        glColor3f(0.9f, 0.5f, 0.0f);
        glLineWidth(2.0f);
        glLineStipple(4, 0xAAAA);
        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINES);
        glVertex2f(x - dx*L, y - dy*L); glVertex2f(x, y);
        glEnd();
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0f);

        // Vrh strelice
        float hL = 0.28f, hA = 0.38f;
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y);
        glVertex2f(x - hL*cosf(rad - hA), y - hL*sinf(rad - hA));
        glVertex2f(x - hL*cosf(rad + hA), y - hL*sinf(rad + hA));
        glEnd();

        // Ugao
        char buf[32]; snprintf(buf, sizeof(buf), "%.0f deg", pendingForceAngleDeg);
        glColor3f(0.7f, 0.35f, 0.0f);
        glRasterPos2f(x - dx*L + 0.1f, y - dy*L - 0.25f);
        for (const char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// ─────────────────────────────────────────────
//  drawForces
// ─────────────────────────────────────────────
void drawForces()
{
    for (const Force& f : app.forces) {
        float x  = app.nodes[f.node].x;
        float y  = app.nodes[f.node].y;
        float dx = cosf(f.angle), dy = sinf(f.angle);
        float L  = 1.2f;

        glColor3f(0.85f, 0.1f, 0.05f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(x - dx*L, y - dy*L); glVertex2f(x, y);
        glEnd();
        glLineWidth(1.0f);

        float hL = 0.28f, hA = 0.38f;
        glBegin(GL_TRIANGLES);
        glVertex2f(x, y);
        glVertex2f(x - hL*cosf(f.angle - hA), y - hL*sinf(f.angle - hA));
        glVertex2f(x - hL*cosf(f.angle + hA), y - hL*sinf(f.angle + hA));
        glEnd();

        float deg = f.angle * 180.0f / (float)M_PI;
        char buf[48];
        snprintf(buf, sizeof(buf), "%.0f N @ %.0f deg", (double)f.magnitude, (double)deg);
        glColor3f(0.7f, 0.0f, 0.0f);
        glRasterPos2f(x - dx*L - 0.05f, y - dy*L - 0.28f);
        for (const char* c = buf; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
}

// ─────────────────────────────────────────────
//  drawSupports
// ─────────────────────────────────────────────
// Crta simbol oslonca:
//   FIXED  – trougao + kose srafure ispod
//   ROLLER – trougao + dve horizontalne crte ispod
// Moze se koristiti i za preview (stipple=true → isprekidano, boja se zadaje spolja)
static void drawSupportGeometry(SupportType type)
{
    // Trougao (vrh = cvor)
    glBegin(GL_LINE_LOOP);
    glVertex2f( 0.0f,   0.0f);
    glVertex2f(-0.45f, -0.55f);
    glVertex2f( 0.45f, -0.55f);
    glEnd();

    if (type == FIXED) {
        // Horizontalna linija + kose srafure centrirane ispod nje
        glBegin(GL_LINES);
        glVertex2f(-0.52f, -0.55f); glVertex2f(0.52f, -0.55f);
        for (int k = -3; k <= 3; k++) {
            float cx = k * 0.15f;      // centar svake crtice
            glVertex2f(cx + 0.11f, -0.55f);
            glVertex2f(cx - 0.11f, -0.82f);
        }
        glEnd();
    } else {
        // Dve horizontalne crte (pokretni)
        glBegin(GL_LINES);
        glVertex2f(-0.52f, -0.60f); glVertex2f(0.52f, -0.60f);
        glVertex2f(-0.52f, -0.72f); glVertex2f(0.52f, -0.72f);
        glEnd();
    }
}

static void drawSupportSymbol(float x, float y, SupportType type, float angle)
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glRotatef(angle * 180.0f / (float)M_PI, 0.0f, 0.0f, 1.0f);
    glColor3f(0.0f, 0.55f, 0.15f);
    glLineWidth(2.0f);
    drawSupportGeometry(type);
    glLineWidth(1.0f);
    glPopMatrix();
}

void drawSupports()
{
    for (const Support& s : app.supports) {
        drawSupportSymbol(app.nodes[s.node].x, app.nodes[s.node].y, s.type, s.angle);
    }
}

// ─────────────────────────────────────────────
// ─────────────────────────────────────────────
//  drawUI – jednostavna tekstualna legenda
// ─────────────────────────────────────────────
void drawUI()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    float aspect = (float)windowWidth / (float)windowHeight;
    gluOrtho2D(-aspect, aspect, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    const char* controls[] = {
        "--- KONTROLE ---",
        "B - Mod Crtanja (LMB: Cvor, RMB: Stap)",
        "F - Mod Sila (LMB na cvor: Dodaj/Rotiraj)",
        "S - Mod Oslonca (LMB: Dodaj → unos tipa   LMB opet: Rotiraj)",
        "E - Unos Materijala",
        "G - Generisi MKE-2D.ulz",
        "Q - Izlaz"
    };

    float yPos = -0.70f;
    for (int i = 0; i < 7; i++) {
        bool active = (i==1 && app.mode==MODE_DRAW)     ||
                      (i==2 && app.mode==MODE_FORCE)    ||
                      (i==3 && app.mode==MODE_SUPPORT)  ||
                      (i==4 && app.mode==MODE_MATERIAL);
        if (i == 0)
            glColor3f(0.1f, 0.1f, 0.1f);   // naslov
        else if (active)
            glColor3f(0.1f, 0.2f, 0.75f);  // aktivan mod
        else
            glColor3f(0.3f, 0.3f, 0.3f);

        glRasterPos2f(-aspect + 0.03f, yPos);
        for (const char* c = controls[i]; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        yPos -= 0.05f;
    }

    // Hint kad je pending sila ili oslonac
    if (pendingForceNode >= 0 || pendingSupNode >= 0) {
        glColor3f(0.75f, 0.35f, 0.0f);
        glRasterPos2f(-aspect + 0.03f, yPos - 0.02f);
        const char* hint = ">> Strelica L/D = rotiraj   LMB na isti cvor = potvrdi";
        for (const char* c = hint; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Indikator terminalnog unosa — gore desno, crven
    if (waitingForInput) {
        const char* hint = ">>> Unesite vrednosti u terminal <<<";
        int textW = glutBitmapLength(GLUT_BITMAP_HELVETICA_12,
                                     (const unsigned char*)hint);
        // Konvertujemo pikselsku sirinu u ortho koordinate
        float charW = 2.0f * aspect * textW / windowWidth;
        glColor3f(0.85f, 0.05f, 0.05f);
        glRasterPos2f(aspect - charW - 0.03f, 0.92f);
        for (const char* c = hint; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


// ─────────────────────────────────────────────
//  display
// ─────────────────────────────────────────────
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    float aspect = (float)windowWidth / (float)windowHeight;
    float halfH  = 10.0f / camZoom;
    float halfW  = halfH * aspect;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(camX-halfW, camX+halfW, camY-halfH, camY+halfH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawGrid();
    drawTruss();
    drawForces();
    drawSupports();
    drawUI();

    glutSwapBuffers();
}

// ─────────────────────────────────────────────
//  reshape
// ─────────────────────────────────────────────
void reshape(int w, int h)
{
    windowWidth  = w;
    windowHeight = (h > 0) ? h : 1;
    glViewport(0, 0, windowWidth, windowHeight);
    glutPostRedisplay();
}

// ─────────────────────────────────────────────
//  handleMouse
// ─────────────────────────────────────────────
void handleMouse(int button, int state, int sx, int sy)
{
    if (state != GLUT_DOWN) return;

    if (button == 3) { camZoom *= 1.1f;  glutPostRedisplay(); return; }
    if (button == 4) { camZoom /= 1.1f;  if (camZoom<0.05f) camZoom=0.05f; glutPostRedisplay(); return; }
    if (button != GLUT_LEFT_BUTTON && button != GLUT_RIGHT_BUTTON) return;

    // Blokiraj sve dok je aktivan terminalni unos
    if (waitingForInput) return;

    float wx, wy;
    screenToWorld(sx, sy, wx, wy);
    wx = snapToGrid(wx);
    wy = snapToGrid(wy);

    // ── MODE_DRAW ────────────────────────────────────────────
    if (app.mode == MODE_DRAW)
    {
        if (button == GLUT_LEFT_BUTTON) {
            int idx = findClosestNode(wx, wy);
            if (idx == -1) {
                Node n; n.x = wx; n.y = wy;
                app.nodes.push_back(n);
            }
            rmb_firstNode = -1;
        }
        else if (button == GLUT_RIGHT_BUTTON) {
            int idx = findClosestNode(wx, wy);
            if (idx == -1) { glutPostRedisplay(); return; }

            if (rmb_firstNode == -1) {
                rmb_firstNode = idx;
            } else if (rmb_firstNode == idx) {
                rmb_firstNode = -1;
            } else {
                bool exists = false;
                for (const Element& e : app.elements)
                    if ((e.n1==rmb_firstNode&&e.n2==idx)||(e.n1==idx&&e.n2==rmb_firstNode))
                    { exists = true; break; }
                if (!exists) {
                    Element e;
                    e.n1 = rmb_firstNode; e.n2 = idx;
                    e.E  = app.currentE;
                    e.A  = app.currentA;
                    int newIdx = (int)app.elements.size();
                    app.elements.push_back(e);
                    // Automatski pitaj za E i A u terminalu
                    askElementProps(newIdx);
                }
                rmb_firstNode = idx;
            }
        }
    }
    // ── MODE_FORCE ───────────────────────────────────────────
    // Tok: (1) LMB na cvor = selektuj + postavi preview strelice
    //      (2) rotiraj strelicu strelicama L/D
    //      (3) LMB na isti cvor = potvrdi smer → terminal pita F
    else if (app.mode == MODE_FORCE && button == GLUT_LEFT_BUTTON)
    {
        int idx = findClosestNode(wx, wy);
        if (idx == -1) { glutPostRedisplay(); return; }

        if (pendingForceNode == -1) {
            // Korak 1: selektuj cvor
            pendingForceNode     = idx;
            pendingForceAngleDeg = app.currentForceAngleDeg;
        } else if (pendingForceNode == idx) {
            // Korak 3: potvrdi smer, tek sad pitaj F
            float confirmedAngle = pendingForceAngleDeg;
            int   confirmedNode  = pendingForceNode;
            app.currentForceAngleDeg = confirmedAngle;
            pendingForceNode = -1;
            askForceProps(confirmedNode, confirmedAngle);
        } else {
            // Klik na drugi cvor dok je jedan vec selektovan:
            // odustani od prethodnog, selektuj novi
            pendingForceNode     = idx;
            pendingForceAngleDeg = app.currentForceAngleDeg;
        }
    }
    // ── MODE_SUPPORT ─────────────────────────────────────────
    // Tok: (1) LMB na cvor = selektuj, pojavljuje se preview
    //      (2) rotiraj strelicama L/D
    //      (3) LMB na isti cvor = potvrdi ugao → terminal pita tip
    //      Klik na drugi cvor = odustani, selektuj novi
    else if (app.mode == MODE_SUPPORT && button == GLUT_LEFT_BUTTON)
    {
        int idx = findClosestNode(wx, wy);
        if (idx == -1) { glutPostRedisplay(); return; }

        if (pendingSupNode == -1) {
            // Korak 1: selektuj cvor
            pendingSupNode     = idx;
            pendingSupAngleDeg = 270.0f;
        } else if (pendingSupNode == idx) {
            // Korak 3: potvrdi ugao → terminal pita tip
            float confirmedAngle = pendingSupAngleDeg;
            int   confirmedNode  = pendingSupNode;
            pendingSupNode = -1;
            askSupportType(confirmedNode, confirmedAngle * (float)M_PI / 180.0f);
        } else {
            // Klik na drugi cvor — odustani, selektuj novi
            pendingSupNode     = idx;
            pendingSupAngleDeg = 270.0f;
        }
    }

    glutPostRedisplay();
}

// ─────────────────────────────────────────────
//  keyboard
// ─────────────────────────────────────────────
void keyboard(unsigned char key, int, int)
{
    // Blokiraj promenu moda dok terminal ceka unos
    if (waitingForInput) return;

    float panStep = 0.8f / camZoom;

    // Promena moda = potvrdi pending silu / oslonac
    auto confirmPending = [&]() {
        if (pendingForceNode >= 0) {
            int   n = pendingForceNode;
            float a = pendingForceAngleDeg;
            app.currentForceAngleDeg = a;
            pendingForceNode = -1;
            askForceProps(n, a);
        }
        if (pendingSupNode >= 0) {
            int   n = pendingSupNode;
            float a = pendingSupAngleDeg;
            pendingSupNode = -1;
            askSupportType(n, a * (float)M_PI / 180.0f);
        }
    };

    switch (key)
    {
    case 'q': case 'Q': exit(0);

    case 'b': case 'B':
        confirmPending();
        app.mode = MODE_DRAW;
        rmb_firstNode = -1;
        break;

    case 'f': case 'F':
        app.mode = MODE_FORCE;
        rmb_firstNode = -1;
        break;

    case 's': case 'S':
        confirmPending();
        app.mode = MODE_SUPPORT;
        rmb_firstNode = -1;
        break;

    case 'e': case 'E':
        confirmPending();
        app.mode = MODE_MATERIAL;
        rmb_firstNode = -1;
        break;

    case 'g': case 'G':
        confirmPending();
        saveToFile();
        break;

    case '+': case '=': camZoom *= 1.2f; break;
    case '-': case '_': camZoom /= 1.2f; if (camZoom<0.05f) camZoom=0.05f; break;

    case 'w': camY += panStep; break;
    case 'a': camX -= panStep; break;
    case 'd': camX += panStep; break;
    case 'z': camY -= panStep; break;

    case 127: case 8: // Delete / Backspace
    {
        if (app.nodes.empty()) break;
        int last = (int)app.nodes.size()-1;
        for (int i=(int)app.elements.size()-1;i>=0;i--)
            if (app.elements[i].n1==last||app.elements[i].n2==last)
                app.elements.erase(app.elements.begin()+i);
        for (int i=(int)app.forces.size()-1;i>=0;i--)
            if (app.forces[i].node==last)
                app.forces.erase(app.forces.begin()+i);
        for (int i=(int)app.supports.size()-1;i>=0;i--)
            if (app.supports[i].node==last)
                app.supports.erase(app.supports.begin()+i);
        app.nodes.pop_back();
        if (rmb_firstNode == last) rmb_firstNode = -1;
        if (pendingForceNode == last) pendingForceNode = -1;
        break;
    }

    case 'r': case 'R': camX=0.0f; camY=0.0f; camZoom=1.0f; break;
    }
    glutPostRedisplay();
}

// ─────────────────────────────────────────────
//  specialKeys
//  L/D strelica: rotira pending silu (ako postoji), inace pan
//  Gore/Dole: uvek pan
// ─────────────────────────────────────────────
void specialKeys(int key, int, int)
{
    if (key == GLUT_KEY_LEFT || key == GLUT_KEY_RIGHT)
    {
        float delta = (key == GLUT_KEY_LEFT) ? 45.0f : -45.0f;
        if (pendingForceNode >= 0) {
            pendingForceAngleDeg += delta;
            while (pendingForceAngleDeg >= 360.0f) pendingForceAngleDeg -= 360.0f;
            while (pendingForceAngleDeg <    0.0f) pendingForceAngleDeg += 360.0f;
        } else if (pendingSupNode >= 0) {
            pendingSupAngleDeg += delta;
            while (pendingSupAngleDeg >= 360.0f) pendingSupAngleDeg -= 360.0f;
            while (pendingSupAngleDeg <    0.0f) pendingSupAngleDeg += 360.0f;
        } else {
            float panStep = 0.8f / camZoom;
            if (key == GLUT_KEY_LEFT)  camX -= panStep;
            else                       camX += panStep;
        }
    }
    else {
        float panStep = 0.8f / camZoom;
        if (key == GLUT_KEY_UP)   camY += panStep;
        else                      camY -= panStep;
    }
    glutPostRedisplay();
}

// ─────────────────────────────────────────────
//  initWindow
// ─────────────────────────────────────────────
void initWindow(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("2D Resetka – Postavka problema");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(handleMouse);
}
