#ifndef DATA_INPUT_H
#define DATA_INPUT_H

// Strukture za materijalne karakteristike
struct MaterialData {
    float E;  // Modul elastičnosti (Young's modulus)
    float A;  // Površina poprečnog preseka
};

// Globalni podaci
extern MaterialData materialData;
extern bool dataEntered;

// Funkcije
void showInputDialog();
void generateMKEFile();
void displayMessages();
void handleKeyboardExtended(unsigned char key, int x, int y);

#endif
