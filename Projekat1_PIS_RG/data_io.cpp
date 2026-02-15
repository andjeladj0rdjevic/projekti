#include "utils.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>

// =====================
// Global storage
// =====================

Node nodes[100];
int nodeCount = 0;

Element elements[200];
int elementCount = 0;

Force forces[100];
int forceCount = 0;

Support supports[100];
int supportCount = 0;

float g_defaultE = 2.1e11f;
float g_defaultA = 1.0e-4f;
float g_currentForceMagnitude = 1.0f;

// =====================
// Status message overlay
// =====================

static std::string g_status = "H - pomoc | Klik LMB: cvor | RMB: stap | S: sacuvaj MKE-2D.ulz | E/A/M: unos";

void setStatusMessage(const std::string& msg) { g_status = msg; }
const std::string& getStatusMessage() { return g_status; }

void drawStatusOverlay()
{
    // Jednostavan overlay u NDC koordinatama (radi uz ortho projekciju)
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.05f, 0.05f, 0.05f);
    glRasterPos2f(-0.98f, 0.92f);

    for (char c : g_status) {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, c);
    }

    glPopMatrix();
}

// =====================
// Export
// =====================

static float elementLength(const Element& e)
{
    if (e.nodeStart < 0 || e.nodeEnd < 0 || e.nodeStart >= nodeCount || e.nodeEnd >= nodeCount) return 0.0f;
    const Node& a = nodes[e.nodeStart];
    const Node& b = nodes[e.nodeEnd];
    const float dx = b.x - a.x;
    const float dy = b.y - a.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool exportULZ(const std::string& filename)
{
    std::ofstream out(filename);
    if (!out.is_open()) {
        setStatusMessage("Greska: ne mogu da otvorim fajl za upis.");
        return false;
    }

    // Napomena: format .ulz zavisi od MKE alata. Ovde je konzistentan tekstualni format
    // koji sadrzi sve potrebne podatke (cvorovi, elementi+E/A/duzina, oslonci, sile).
    out << "# MKE-2D.ulz (generisano iz OpenGL editor-a)\n";
    out << "# Jedinice: koordinate - relativne (kao u crtezu), E[Pa], A[m^2], F[N], ugao[rad]\n\n";

    out << "NODES " << nodeCount << "\n";
    out << std::fixed << std::setprecision(6);
    for (int i = 0; i < nodeCount; i++) {
        out << i << " " << nodes[i].x << " " << nodes[i].y << "\n";
    }
    out << "\n";

    out << "ELEMENTS " << elementCount << "\n";
    out << std::scientific << std::setprecision(6);
    for (int i = 0; i < elementCount; i++) {
        const Element& e = elements[i];
        out << i << " " << e.nodeStart << " " << e.nodeEnd
            << " " << e.E << " " << e.A
            << " " << std::fixed << std::setprecision(6) << elementLength(e) << std::scientific << "\n";
    }
    out << "\n";

    out << "SUPPORTS " << supportCount << "\n";
    out << std::fixed << std::setprecision(6);
    for (int i = 0; i < supportCount; i++) {
        out << i << " " << supports[i].nodeId << " " << supports[i].type << " " << supports[i].angle << "\n";
    }
    out << "\n";

    out << "FORCES " << forceCount << "\n";
    out << std::fixed << std::setprecision(6);
    for (int i = 0; i < forceCount; i++) {
        out << i << " " << forces[i].nodeId << " " << forces[i].magnitude << " " << forces[i].angle << "\n";
    }

    setStatusMessage("Sacuvano: " + filename + " (" + std::to_string(nodeCount) + " cvorova, " + std::to_string(elementCount) + " elemenata)");
    return true;
}
