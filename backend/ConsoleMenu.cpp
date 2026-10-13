#include "ConsoleMenu.h"
#include "BusPriority.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

ConsoleMenu::ConsoleMenu(BusStatusEngine& engine, HtmlGenerator& html)
    : engine(engine), html(html), inputClosed(false) {}

string ConsoleMenu::readLine() {
    string line;
    if (!getline(cin, line)) {
        inputClosed = true;
        return "";
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

int ConsoleMenu::readChoice(const string& prompt, int low, int high, int valueIfInputCloses) {
    while (true) {
        if (inputClosed) {
            return valueIfInputCloses;
        }
        cout << prompt;
        string line = readLine();
        if (inputClosed) {
            return valueIfInputCloses;
        }

        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            cout << "  Please type a number from " << low << " to " << high << "." << endl;
            continue;
        }
        size_t last = line.find_last_not_of(" \t\r\n");
        string trimmed = line.substr(first, last - first + 1);

        try {
            size_t idx = 0;
            int val = stoi(trimmed, &idx);
            if (idx == trimmed.length() && val >= low && val <= high) {
                return val;
            }
        } catch (...) {}

        cout << "  Please type a number from " << low << " to " << high << "." << endl;
    }
}

void ConsoleMenu::refreshPages() {
    bool ok1 = html.writeStudentPage(engine);
    bool ok2 = html.writeHodPage(engine);
    if (!ok1 || !ok2) {
        cout << "  Warning: could not update the web pages in the output folder." << endl;
    }
}

void ConsoleMenu::run() {
    while (!inputClosed) {
        cout << "\n=========== Where's My Bus ===========" << endl;
        cout << "1. Student" << endl;
        cout << "2. Conductor" << endl;
        cout << "3. Department Head" << endl;
        cout << "0. Exit" << endl;

        int choice = readChoice("Enter your choice: ", 0, 3, 0);
        if (choice == 0 || inputClosed) {
            cout << "Exiting Where's My Bus. Goodbye!" << endl;
            break;
        } else if (choice == 1) {
            runStudentMenu();
        } else if (choice == 2) {
            runConductorMenu();
        } else if (choice == 3) {
            runHodMenu();
        }
    }
}

void ConsoleMenu::runStudentMenu() {
    cout << "\n--- Student View ---" << endl;
    cout << "Student page: " << html.getStudentPagePath() << endl;

    vector<pair<string, string>> stops = engine.getRouteRegistry().getDistinctStops();
    while (!inputClosed) {
        cout << "\nStops:" << endl;
        for (size_t i = 0; i < stops.size(); i++) {
            cout << (i + 1) << ". " << stops[i].second << " (" << stops[i].first << ")" << endl;
        }
        cout << "0. Back" << endl;

        int choice = readChoice("Pick a stop number (0 to go back): ", 0, static_cast<int>(stops.size()), 0);
        if (choice == 0 || inputClosed) {
            break;
        }

        const auto& selectedStop = stops[choice - 1];
        Student student("STUD-1", "Student", selectedStop.first);

        cout << "\nBuses for " << selectedStop.second << " (" << selectedStop.first << "):" << endl;
        cout << "Bus | Route | Status | Bus is at" << endl;

        vector<Bus*> sortedBuses = sortBusesForDisplay(engine.getBusRegistry().getAllBuses());
        for (const Bus* bus : sortedBuses) {
            cout << "Bus " << bus->getBusId() << " | " << bus->getRouteId() << " | ";

            Route* route = engine.getRouteRegistry().getRouteById(bus->getRouteId());
            bool servesStop = (route != nullptr && route->findStopById(selectedStop.first) != nullptr);

            if (!servesStop) {
                cout << "BLACK - Not available (doesn't serve this stop)";
            } else {
                StopColor col = engine.getStatusForBusAtStop(*bus, selectedStop.first);
                cout << stopColorToString(col) << " - " << stopColorMeaning(col);
            }

            cout << " | ";
            string curStop = engine.getCurrentStopName(*bus);
            cout << (curStop.empty() ? "-" : curStop) << endl;
        }
    }
}

void ConsoleMenu::runConductorMenu() {
    cout << "\n--- Conductor Menu ---" << endl;
    cout << "Buses by route:" << endl;
    for (const Route* route : engine.getRouteRegistry().getAllRoutes()) {
        cout << "  " << route->getRouteId() << ":";
        for (const Bus* bus : engine.getBusRegistry().getBusesForRoute(route->getRouteId())) {
            cout << " " << bus->getBusId();
        }
        cout << endl;
    }

    string busId;
    while (!inputClosed) {
        cout << "\nEnter your bus number (or 0 to go back): ";
        string busNumInput = readLine();
        if (inputClosed) {
            return;
        }

        size_t first = busNumInput.find_first_not_of(" \t\r\n");
        if (first == string::npos) {
            continue;
        }
        size_t last = busNumInput.find_last_not_of(" \t\r\n");
        string trimmed = busNumInput.substr(first, last - first + 1);

        if (trimmed == "0") {
            return;
        }

        Bus* bus = engine.getBusRegistry().getBusById(trimmed);
        if (bus == nullptr) {
            cout << "  Unknown bus number. Please try again." << endl;
            continue;
        }
        busId = trimmed;
        break;
    }

    if (busId.empty() || inputClosed) {
        return;
    }

    Bus* bus = engine.getBusRegistry().getBusById(busId);
    Route* route = engine.getRouteRegistry().getRouteById(bus->getRouteId());
    if (route == nullptr) {
        cout << "  Error: Bus route not found." << endl;
        return;
    }

    Conductor conductor("COND-" + busId, "Conductor of bus " + busId, busId);

    // Conductor operational loop
    while (!inputClosed) {
        cout << "\n--- Conductor of bus " << busId << " ---" << endl;
        cout << *bus << endl;

        if (bus->getState() == BusState::NOT_AVAILABLE) {
            string headName = (route->getHead() != nullptr) ? route->getHead()->getStopName() : "start";
            RouteStopNode* restNode = route->findStopById(route->getRestStopId());
            string restName = (restNode != nullptr) ? restNode->getStopName() : "rest stop";

            cout << "0. Start shift: bus is at " << headName << endl;
            cout << "1. Start shift: bus is at the rest stop (" << restName << ")" << endl;
            cout << "2. Report maintenance / malfunction" << endl;
            cout << "3. Undo my last update" << endl;
            cout << "4. Back" << endl;

            int opt = readChoice("Choice: ", 0, 4, 4);
            if (opt == 4 || inputClosed) {
                return;
            }

            ActionResult res{false, ""};
            if (opt == 0) {
                res = engine.startShiftAtCollege(conductor);
            } else if (opt == 1) {
                res = engine.startShiftAtRestStop(conductor);
            } else if (opt == 2) {
                res = engine.setMaintenance(conductor, true);
            } else if (opt == 3) {
                res = engine.undoLastUpdate(conductor);
            }

            if (res.ok) {
                cout << "  OK: " << res.message << endl;
                refreshPages();
            } else {
                cout << "  Problem: " << res.message << endl;
            }
        } else {
            // Bus is IN_SERVICE, RESTING, or MAINTENANCE
            int nextPos = bus->getCurrentSequenceIndex() + 1;
            RouteStopNode* nextNode = route->findStopByIndex(nextPos);
            if (nextNode != nullptr) {
                if (nextNode->hasEtaToNext()) {
                    cout << "Next stop: " << nextNode->getStopName()
                         << " (stored ETA " << nextNode->getEtaToNextMinutes() << " min)" << endl;
                } else {
                    cout << "Next stop: " << nextNode->getStopName() << endl;
                }
            }

            cout << "1. Reached next stop" << endl;
            cout << "2. Seats: mark bus " << (bus->getHasFreeSeats() ? "FULL" : "FREE") << endl;
            cout << "3. Maintenance: " << (bus->getState() == BusState::MAINTENANCE ? "mark FIXED" : "report problem") << endl;
            cout << "4. Undo my last update" << endl;
            cout << "5. End shift" << endl;
            cout << "0. Back" << endl;

            int opt = readChoice("Choice: ", 0, 5, 0);
            if (opt == 0 || inputClosed) {
                return;
            }

            ActionResult res{false, ""};
            if (opt == 1) {
                res = engine.reachedNextStop(conductor);
            } else if (opt == 2) {
                res = engine.setSeatsAvailable(conductor, !bus->getHasFreeSeats());
            } else if (opt == 3) {
                res = engine.setMaintenance(conductor, bus->getState() != BusState::MAINTENANCE);
            } else if (opt == 4) {
                res = engine.undoLastUpdate(conductor);
            } else if (opt == 5) {
                res = engine.goOffDuty(conductor);
            }

            if (res.ok) {
                cout << "  OK: " << res.message << endl;
                refreshPages();
            } else {
                cout << "  Problem: " << res.message << endl;
            }
        }
    }
}

void ConsoleMenu::runHodMenu() {
    while (!inputClosed) {
        cout << "\n--- Department Head Overview ---" << endl;
        cout << "1. Show all buses here" << endl;
        cout << "2. Update the report page and show where it is" << endl;
        cout << "0. Back" << endl;

        int choice = readChoice("Choice: ", 0, 2, 0);
        if (choice == 0 || inputClosed) {
            break;
        }

        if (choice == 1) {
            cout << "\nAll Buses (Priority Order):" << endl;
            vector<Bus*> sortedBuses = sortBusesForDisplay(engine.getBusRegistry().getAllBuses());
            for (const Bus* b : sortedBuses) {
                cout << "  " << *b << endl;
            }
        } else if (choice == 2) {
            if (html.writeHodPage(engine)) {
                cout << "Report page updated: " << html.getHodPagePath() << endl;
            } else {
                cout << "  Warning: could not update the report page." << endl;
            }
        }
    }
}
