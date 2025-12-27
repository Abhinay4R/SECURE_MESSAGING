#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

std::string generateRandomHex(int length = 50) {
    const char* hexDigits = "0123456789";
    std::string hex = "";
    for (int i = 0; i < length; ++i) {
        hex += hexDigits[rand() % 10];
    }
    return hex;
}

void generateDataset(const std::string& filename, int lines = 10000000) {
    std::ofstream fout(filename);
    if (!fout) {
        std::cerr << "Failed to open " << filename << "\n";
        return;
    }

    for (int i = 0; i < lines; ++i) {
        std::string num1 = generateRandomHex();
        std::string num2 = generateRandomHex();
        fout << num1 << ";" << num2 << "\n";
    }

    fout.close();
}

int main() {
    srand(static_cast<unsigned>(time(0)));

    generateDataset("BigDataDeciAdd", 100000);
    generateDataset("BigDataDeciSub", 100000);
    generateDataset("BigDataDeciMul", 10000); // Less because mul is heavier

    std::cout << "Datasets generated.\n";
    return 0;
}
