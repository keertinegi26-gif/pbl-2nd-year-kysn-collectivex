#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <string>
#include "patient.h"
using namespace std;

class InputOutput {
public:
    static int readInteger(const string& message);
    static string readText(const string& message);
    static Patient readPatient();
    static void printMenu();
    static void printMessage(const string& message);
};

#endif
//end
