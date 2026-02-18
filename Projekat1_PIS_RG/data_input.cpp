#include "data_input.h"
#include "utils.h"
#include <fstream>
#include <iostream>
#include <string>

MaterialData materialData = {210000.0f, 0.01f};
bool dataEntered = false;

static std::string statusMessage = "MOD: Crtanje (LMB: Cvor, RMB: Stap)";
static float messageTimer = 0.0f;

void displayMessages() {
    // Prikaz trenutnog statusa/moda
    glColor3f(0.0f, 0.0f, 0.0f);
    glRasterPos2f(-0.95f, 0.92f);
    for (char c : statusMessage) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    if (messageTimer > 0.0f) {
        messageTimer -= 0.01f;
    }

    // Legenda
    glColor3f(0.3f, 0.3f, 0.3f);
    float yPos = -0.7f;
    std::string controls[] = {
        "--- KONTROLE ---",
        "B - Mod Crtanja (LMB: Cvor, RMB: Stap)",
        "F - Mod Sila (LMB na cvor: Dodaj/Rotiraj)",
        "S - Mod Oslonca (LMB na cvor: Dodaj/Rotiraj)",
        "E - Unos Materijala",
        "G - Generisi MKE-2D.ulz",
        "Q - Izlaz"
    };

    for (const auto& line : controls) {
        glRasterPos2f(-0.95f, yPos);
        for (char c : line) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        yPos -= 0.05f;
    }
}

void showInputDialog() {
    std::cout << "\n--- UNOS PODATAKA ---" << std::endl;
    std::cout << "Unesite Modul elasticnosti E [MPa]: ";
    std::cin >> materialData.E;
    std::cout << "Unesite Povrsinu preseka A [m^2]: ";
    std::cin >> materialData.A;
    dataEntered = true;
    std::cout << "Podaci uspesno uneti.\n" << std::endl;
}

void generateMKEFile() {
    std::ofstream outFile("MKE-2D.ulz");
    if (!outFile.is_open()) {
        statusMessage = "GRESKA: Ne mogu da kreiram fajl!";
        return;
    }

    // 1. Zaglavlje
    outFile << "MKE-2D INPUT FILE" << std::endl;
    outFile << "TYPE TRUSS" << std::endl;

    // 2. Materijali
    outFile << "MATERIALS 1" << std::endl; // Jedan materijal za sve
    outFile << "1 " << materialData.E << " " << materialData.A << std::endl;

    // 3. Cvorovi
    outFile << "NODES " << nodeCount << std::endl;
    for (int i = 0; i < nodeCount; i++) {
        // ID, X, Y
        outFile << (i + 1) << " " << nodes[i].x << " " << nodes[i].y << std::endl;
    }

    // 4. Elementi
    outFile << "ELEMENTS " << elementCount << std::endl;
    for (int i = 0; i < elementCount; i++) {
        // ID, Tip(1), MatID(1), N1, N2
        outFile << (i + 1) << " 1 1 " << (elements[i].nodeStart + 1) << " " << (elements[i].nodeEnd + 1) << std::endl;
    }

    // 5. Oslonci
    outFile << "SUPPORTS " << supportCount << std::endl;
    for (int i = 0; i < supportCount; i++) {
        // ID cvora, Tip (pretpostavimo 111 za fiksni, ili kodove)
        // Format: NodeID Code (npr 1 = fix X, 2 = fix Y, 3 = fix XY)
        // Ovde pisemo pojednostavljeno za domaci
        outFile << (supports[i].nodeId + 1) << " " << "11" << std::endl;
    }

    // 6. Sile
    outFile << "FORCES " << forceCount << std::endl;
    for (int i = 0; i < forceCount; i++) {
        float Fx = cosf(forces[i].angle) * forces[i].magnitude;
        float Fy = sinf(forces[i].angle) * forces[i].magnitude;
        // NodeID, Fx, Fy
        outFile << (forces[i].nodeId + 1) << " " << Fx << " " << Fy << std::endl;
    }

    outFile.close();
    statusMessage = "USPEH: Fajl MKE-2D.ulz generisan!";
    std::cout << "Fajl uspesno generisan!" << std::endl;
}

void handleKeyboardExtended(unsigned char key, int x, int y) {
    switch (key) {
        case 'q':
        case 'Q':
            exit(0);
            break;

        case 'e':
        case 'E':
            showInputDialog();
            break;

        case 'g':
        case 'G':
            generateMKEFile();
            break;

        case 'b':
        case 'B':
            currentMode = MODE_BUILD;
            statusMessage = "MOD: Crtanje (LMB: Cvor, RMB: Stap)";
            break;

        case 'f':
        case 'F':
            currentMode = MODE_FORCE;
            statusMessage = "MOD: Sile (LMB na cvor dodaje/rotira)";
            break;

        case 's':
        case 'S':
            currentMode = MODE_SUPPORT;
            statusMessage = "MOD: Oslonci (LMB na cvor dodaje/rotira)";
            break;
    }
    glutPostRedisplay();
}
