#include "utils.h"
#include <cstdio>
#include <cmath>
#include <string>

static std::string nodeLabel(int i)
{
    std::string s;
    do {
        s = (char)('A' + (i % 26)) + s;
        i = i / 26 - 1;
    } while (i >= 0);
    return s;
}

void saveToFile()
{
    FILE* f = fopen("MKE-2D.ulz", "w");
    if (!f) {
        printf("  [GRESKA] Nije moguce otvoriti MKE-2D.ulz za pisanje!\n");
        return;
    }

    // ── Cvorovi ───────────────────────────────────────────────
    fprintf(f, "CVOROVI %d\n", (int)app.nodes.size());
    fprintf(f, "# naziv   x [m]       y [m]\n");
    for (int i = 0; i < (int)app.nodes.size(); i++) {
        fprintf(f, "%s        %.6f [m]   %.6f [m]\n",
                nodeLabel(i).c_str(),
                (double)app.nodes[i].x,
                (double)app.nodes[i].y);
    }

    // ── Stapovi ───────────────────────────────────────────────
    fprintf(f, "\nSTAPOVI %d\n", (int)app.elements.size());
    fprintf(f, "# br   n1  n2   E [Pa]          A [m^2]\n");
    for (int i = 0; i < (int)app.elements.size(); i++) {
        const Element& e = app.elements[i];
        fprintf(f, "%d      %s   %s   %.6e [Pa]   %.6e [m^2]\n",
                i + 1,
                nodeLabel(e.n1).c_str(),
                nodeLabel(e.n2).c_str(),
                (double)e.E,
                (double)e.A);
    }

    // ── Oslonci ───────────────────────────────────────────────
    fprintf(f, "\nOSLONCI %d\n", (int)app.supports.size());
    fprintf(f, "# cvor   tip           ugao [deg]\n");
    for (int i = 0; i < (int)app.supports.size(); i++) {
        const Support& s = app.supports[i];
        float angleDeg = s.angle * 180.0f / (float)M_PI;
        fprintf(f, "%s       %-12s  %.2f [deg]\n",
                nodeLabel(s.node).c_str(),
                (s.type == FIXED) ? "NEPOKRETNI" : "POKRETNI",
                (double)angleDeg);
    }

    // ── Sile ──────────────────────────────────────────────────
    fprintf(f, "\nSILE %d\n", (int)app.forces.size());
    fprintf(f, "# cvor   Fx [N]          Fy [N]          |F| [N]       ugao [deg]\n");
    for (int i = 0; i < (int)app.forces.size(); i++) {
        const Force& fc = app.forces[i];
        float Fx  = fc.magnitude * cosf(fc.angle);
        float Fy  = fc.magnitude * sinf(fc.angle);
        float deg = fc.angle * 180.0f / (float)M_PI;
        fprintf(f, "%s       %.6f [N]   %.6f [N]   %.6f [N]   %.2f [deg]\n",
                nodeLabel(fc.node).c_str(),
                (double)Fx, (double)Fy,
                (double)fc.magnitude, (double)deg);
    }

    fclose(f);
    printf("\n  [OK] Sacuvano u MKE-2D.ulz\n\n");
}
