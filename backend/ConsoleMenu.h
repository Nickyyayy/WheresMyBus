#ifndef CONSOLE_MENU_H
#define CONSOLE_MENU_H

#include "BusStatusEngine.h"
#include "HtmlGenerator.h"
#include <string>

using namespace std;

// ConsoleMenu provides a plain ASCII interactive terminal interface for Student, Conductor, and HOD
class ConsoleMenu {
private:
    BusStatusEngine& engine;
    HtmlGenerator& html;
    bool inputClosed;

    string readLine();
    int readChoice(const string& prompt, int low, int high, int valueIfInputCloses);
    void refreshPages();

    void runStudentMenu();
    void runConductorMenu();
    void runHodMenu();

public:
    ConsoleMenu(BusStatusEngine& engine, HtmlGenerator& html);

    void run();
};

#endif // CONSOLE_MENU_H
