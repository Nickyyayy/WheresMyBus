#include "WebBridge.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

static atomic<bool> g_webBridgeRunning{true};

static void handleSignal(int) {
    g_webBridgeRunning = false;
}

WebBridge::WebBridge(BusStatusEngine& engine, HtmlGenerator& html,
                     string requestFile, string responseFile)
    : engine(engine), html(html),
      requestFile(move(requestFile)), responseFile(move(responseFile)) {}

string WebBridge::parseJsonField(const string& json, const string& key) {
    string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == string::npos) return "";
    pos = json.find(':', pos + search.length());
    if (pos == string::npos) return "";
    pos = json.find('\"', pos);
    if (pos == string::npos) return "";
    size_t endPos = json.find('\"', pos + 1);
    if (endPos == string::npos) return "";
    return json.substr(pos + 1, endPos - pos - 1);
}

void WebBridge::writeResponse(const string& path, bool ok, const string& message) {
    string tmpPath = path + ".tmp";
    {
        ofstream out(tmpPath, ios::trunc);
        out << "{\n";
        out << "  \"ok\": " << (ok ? "true" : "false") << ",\n";
        out << "  \"message\": \"";
        for (char c : message) {
            if (c == '"') out << "\\\"";
            else if (c == '\\') out << "\\\\";
            else if (c == '\n') out << "\\n";
            else if (c == '\r') continue;
            else out << c;
        }
        out << "\"\n";
        out << "}\n";
    }
    remove(path.c_str());
    rename(tmpPath.c_str(), path.c_str());
}

ActionResult WebBridge::executeAction(const string& busId, const string& action) {
    if (busId.empty()) {
        return {false, "Missing bus ID."};
    }
    Bus* bus = engine.getBusRegistry().getBusById(busId);
    if (bus == nullptr) {
        return {false, "Unknown bus number: " + busId};
    }
    Conductor conductor("COND-" + busId, "Conductor of bus " + busId, busId);

    if (action == "startShiftCollege") {
        return engine.startShiftAtCollege(conductor);
    } else if (action == "startShiftRest") {
        return engine.startShiftAtRestStop(conductor);
    } else if (action == "reachedNextStop") {
        return engine.reachedNextStop(conductor);
    } else if (action == "toggleSeats") {
        return engine.setSeatsAvailable(conductor, !bus->getHasFreeSeats());
    } else if (action == "setMaintenance") {
        return engine.setMaintenance(conductor, bus->getState() != BusState::MAINTENANCE);
    } else if (action == "undo") {
        return engine.undoLastUpdate(conductor);
    } else if (action == "endShift") {
        return engine.goOffDuty(conductor);
    } else {
        return {false, "Unrecognized action: " + action};
    }
}

void WebBridge::refreshAll(const string& message) {
    html.writeHomePage(engine);
    html.writeStudentPage(engine);
    html.writeConductorPage(engine);
    html.writeHodPage(engine);
    html.writeStateJson(engine, message);
}

void WebBridge::run() {
    signal(SIGINT, handleSignal);
#ifdef SIGTERM
    signal(SIGTERM, handleSignal);
#endif
#ifdef SIGBREAK
    signal(SIGBREAK, handleSignal);
#endif

    cout << "\n=======================================================" << endl;
    cout << "     Where's My Bus                                    " << endl;
    cout << "=======================================================" << endl;
    cout << "Portal Homepage: " << html.getHomePagePath() << endl;
    cout << "Press Ctrl+C to stop." << endl;
    cout << "-------------------------------------------------------" << endl;

    // Clean up stale response from prior executions
    remove(responseFile.c_str());

    // Initial page generation
    refreshAll("Transit system initialized.");

    while (g_webBridgeRunning) {
        // 1. Check for request file
        ifstream in(requestFile);
        if (in.is_open()) {
            string content((istreambuf_iterator<char>(in)), istreambuf_iterator<char>());
            in.close();
            remove(requestFile.c_str());

            if (!content.empty()) {
                string busId = parseJsonField(content, "busId");
                string action = parseJsonField(content, "action");

                ActionResult res = executeAction(busId, action);

                // Regenerate all pages and write updated state.json
                refreshAll(res.message);

                // Write atomic response for web server
                writeResponse(responseFile, res.ok, res.message);

                cout << "[" << Update::currentTimestamp() << "] "
                     << (res.ok ? "SUCCESS" : "REJECTED") << " | Bus " << busId
                     << " (" << action << "): " << res.message << endl;
            }
        }

        // Sleep 50ms to yield CPU
#ifdef _WIN32
        Sleep(50);
#else
        this_thread::sleep_for(chrono::milliseconds(50));
#endif
    }

    cout << "\nStopping server." << endl;
}
