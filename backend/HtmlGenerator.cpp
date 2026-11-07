#include "HtmlGenerator.h"
#include "BusPriority.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <algorithm>

#if defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#define HAS_STD_FILESYSTEM 1
#else
#define HAS_STD_FILESYSTEM 0
#endif

using namespace std;

HtmlGenerator::HtmlGenerator(string outputDir, string cssHref)
    : outputDir(move(outputDir)), cssHref(move(cssHref)) {}

string HtmlGenerator::escapeHtml(const string& text) {
    string out;
    for (char c : text) {
        if (c == '&') out += "&amp;";
        else if (c == '<') out += "&lt;";
        else if (c == '>') out += "&gt;";
        else if (c == '"') out += "&quot;";
        else out.push_back(c);
    }
    return out;
}

string HtmlGenerator::colorToClass(StopColor color) {
    switch (color) {
        case StopColor::GREEN: return "c-green";
        case StopColor::BLUE: return "c-blue";
        case StopColor::RED: return "c-red";
        case StopColor::BROWN: return "c-brown";
        case StopColor::PURPLE: return "c-purple";
        case StopColor::BLACK: return "c-black";
        default: return "c-black";
    }
}

bool HtmlGenerator::atomicWrite(const string& targetPath, const string& content) const {
    string tmpPath = targetPath + ".tmp";
    {
        ofstream out(tmpPath, ios::trunc);
        if (!out.is_open()) {
            return false;
        }
        out << content;
    }

#if HAS_STD_FILESYSTEM
    try {
        fs::rename(tmpPath, targetPath);
        return true;
    } catch (...) {
        fs::remove(targetPath);
        try {
            fs::rename(tmpPath, targetPath);
            return true;
        } catch (...) {
            return false;
        }
    }
#else
    remove(targetPath.c_str());
    if (rename(tmpPath.c_str(), targetPath.c_str()) == 0) {
        return true;
    }
    return false;
#endif
}

string HtmlGenerator::getHomePagePath() const {
    return outputDir + "/index.html";
}

string HtmlGenerator::getStudentPagePath() const {
    return outputDir + "/student.html";
}

string HtmlGenerator::getConductorPagePath() const {
    return outputDir + "/conductor.html";
}

string HtmlGenerator::getHodPagePath() const {
    return outputDir + "/hod.html";
}

string HtmlGenerator::renderNavBar(const string& activePage) {
    ostringstream nav;
    nav << "<nav class=\"nav-bar\">\n";
    nav << "  <a href=\"index.html\" class=\"nav-brand\">&#128652; Where's My Bus</a>\n";
    nav << "  <div class=\"nav-links\">\n";
    nav << "    <a href=\"index.html\" class=\"nav-link" << (activePage == "home" ? " active" : "") << "\">Home</a>\n";
    nav << "    <a href=\"student.html\" class=\"nav-link" << (activePage == "student" ? " active" : "") << "\">Student Portal</a>\n";
    nav << "    <a href=\"conductor.html\" class=\"nav-link" << (activePage == "conductor" ? " active" : "") << "\">Conductor Portal</a>\n";
    nav << "    <a href=\"hod.html\" class=\"nav-link" << (activePage == "hod" ? " active" : "") << "\">HOD Report</a>\n";
    nav << "  </div>\n";
    nav << "</nav>\n";
    return nav.str();
}

bool HtmlGenerator::writeHomePage(const BusStatusEngine& engine) const {
    const RouteRegistry& routeRegistry = engine.getRouteRegistry();
    const BusRegistry& busRegistry = engine.getBusRegistry();

    int totalBuses = 0;
    int inServiceCount = 0;
    int restingCount = 0;
    for (const Bus* b : busRegistry.getAllBuses()) {
        totalBuses++;
        if (b->getState() == BusState::IN_SERVICE) inServiceCount++;
        else if (b->getState() == BusState::RESTING) restingCount++;
    }

    ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>Where's My Bus - Transit Portal</title>\n";
    html << "<link rel=\"stylesheet\" href=\"" << escapeHtml(cssHref) << "\">\n";
    html << "<noscript><meta http-equiv=\"refresh\" content=\"15\"></noscript>\n";
    html << "<script>setTimeout(function(){ location.reload(); }, 15000);</script>\n";
    html << "</head>\n<body>\n";
    html << "<div class=\"container\">\n";

    html << renderNavBar("home");

    html << "<h1>Where's My Bus</h1>\n";
    html << "<p class=\"subtitle\">Campus Transit Management System &bull; Graphic Era University</p>\n";

    // Metrics summary
    html << "<div class=\"cards\">\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << totalBuses << "</div><div class=\"card-label\">Total Fleet</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\" style=\"color:var(--green);\">" << inServiceCount << "</div><div class=\"card-label\">Buses in Service</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\" style=\"color:var(--brown);\">" << restingCount << "</div><div class=\"card-label\">Buses Resting</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\" style=\"color:var(--blue);\">" << routeRegistry.getRouteCount() << "</div><div class=\"card-label\">Active Routes</div></div>\n";
    html << "</div>\n";

    // 3 Role Cards
    html << "<div class=\"role-cards\">\n";

    html << "  <a href=\"student.html\" class=\"role-card\">\n";
    html << "    <div class=\"role-icon\">&#127891;</div>\n";
    html << "    <div class=\"role-title\">Student Portal</div>\n";
    html << "    <div class=\"role-desc\">Pick your stop to check live approaching buses, view seat availability, check stored ETAs, and track both outbound drop and inbound pickup journeys.</div>\n";
    html << "    <div class=\"role-btn\">Open Student Dashboard &rarr;</div>\n";
    html << "  </a>\n";

    html << "  <a href=\"conductor.html\" class=\"role-card\">\n";
    html << "    <div class=\"role-icon\">&#128652;</div>\n";
    html << "    <div class=\"role-title\">Conductor Portal</div>\n";
    html << "    <div class=\"role-desc\">Monitor your assigned bus in real time, view upcoming stop names, check segment ETAs, and review drop & pickup turnaround progress.</div>\n";
    html << "    <div class=\"role-btn\">Open Conductor Dashboard &rarr;</div>\n";
    html << "  </a>\n";

    html << "  <a href=\"hod.html\" class=\"role-card\">\n";
    html << "    <div class=\"role-icon\">&#128084;</div>\n";
    html << "    <div class=\"role-title\">Department Head (HOD)</div>\n";
    html << "    <div class=\"role-desc\">Supervisory administrative overview of complete fleet metrics, operational health, completed trips, and chronological audit history.</div>\n";
    html << "    <div class=\"role-btn\">Open HOD Analytics &rarr;</div>\n";
    html << "  </a>\n";

    html << "</div>\n";

    // Notice box
    html << "<div class=\"help-box\">\n";
    html << "  <strong>System Architecture:</strong> Powered by a deterministic C++ core featuring a Doubly Linked List route backbone, Queue buffer, Memento undo Stack, and Binary Max-Heap priority sort. All web pages refresh dynamically every 15 seconds.\n";
    html << "</div>\n";

    html << "</div>\n";
    html << "</body>\n</html>\n";

    return atomicWrite(getHomePagePath(), html.str());
}

bool HtmlGenerator::writeStudentPage(const BusStatusEngine& engine) const {
    const RouteRegistry& routeRegistry = engine.getRouteRegistry();
    const BusRegistry& busRegistry = engine.getBusRegistry();
    vector<Bus*> allBusesSorted = sortBusesForDisplay(busRegistry.getAllBuses());
    vector<pair<string, string>> distinctStops = routeRegistry.getDistinctStops();
    vector<Route*> allRoutes = routeRegistry.getAllRoutes();

    ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>Where's My Bus - Student Dashboard</title>\n";
    html << "<link rel=\"stylesheet\" href=\"" << escapeHtml(cssHref) << "\">\n";
    html << "<noscript><meta http-equiv=\"refresh\" content=\"15\"></noscript>\n";
    html << "</head>\n<body>\n";
    html << "<div class=\"container\">\n";

    html << renderNavBar("student");

    // 1. Header
    html << "<h1>Where's My Bus</h1>\n";
    html << "<p class=\"subtitle\">Auto-refreshes every 15s &bull; Updated: " << Update::currentTimestamp() << "</p>\n";

    // 2. Legend
    html << "<div class=\"legend\">\n";
    html << "  <span class=\"chip c-green\">" << stopColorMeaning(StopColor::GREEN) << "</span>\n";
    html << "  <span class=\"chip c-blue\">" << stopColorMeaning(StopColor::BLUE) << "</span>\n";
    html << "  <span class=\"chip c-red\">" << stopColorMeaning(StopColor::RED) << "</span>\n";
    html << "  <span class=\"chip c-brown\">" << stopColorMeaning(StopColor::BROWN) << "</span>\n";
    html << "  <span class=\"chip c-purple\">" << stopColorMeaning(StopColor::PURPLE) << "</span>\n";
    html << "  <span class=\"chip c-black\">" << stopColorMeaning(StopColor::BLACK) << "</span>\n";
    html << "</div>\n";

    // 3. Stop picker
    html << "<div class=\"picker\">\n";
    html << "  <label for=\"stopSelect\"><strong>Select your stop:</strong></label>\n";
    html << "  <select id=\"stopSelect\">\n";
    html << "    <option value=\"\">-- choose your stop --</option>\n";
    for (const auto& sp : distinctStops) {
        html << "    <option value=\"" << escapeHtml(sp.first) << "\">"
             << escapeHtml(sp.second) << " (" << escapeHtml(sp.first) << ")</option>\n";
    }
    html << "  </select>\n";
    html << "</div>\n";

    // 4. One hidden panel per distinct stop
    for (const auto& sp : distinctStops) {
        const string& stopId = sp.first;
        const string& stopName = sp.second;

        html << "<section class=\"stop-panel\" data-stop=\"" << escapeHtml(stopId) << "\" hidden>\n";
        html << "  <h2>Buses for " << escapeHtml(stopName) << "</h2>\n";
        html << "  <div class=\"scroll\">\n";
        html << "    <table>\n";
        html << "      <thead>\n";
        html << "        <tr><th>Bus</th><th>Route</th><th>Status</th><th>Bus is at</th><th>Stored ETA to next stop</th></tr>\n";
        html << "      </thead>\n";
        html << "      <tbody>\n";

        for (const Bus* bus : allBusesSorted) {
            html << "        <tr>\n";
            html << "          <td><strong>" << escapeHtml(bus->getBusId()) << "</strong></td>\n";
            html << "          <td>" << escapeHtml(bus->getRouteId()) << "</td>\n";

            Route* route = routeRegistry.getRouteById(bus->getRouteId());
            bool routeVisitsStop = (route != nullptr && route->findStopById(stopId) != nullptr);

            if (!routeVisitsStop) {
                html << "          <td><span class=\"chip c-black\">Not available (doesn't serve this stop)</span></td>\n";
            } else {
                StopColor col = engine.getStatusForBusAtStop(*bus, stopId);
                html << "          <td><span class=\"chip " << colorToClass(col) << "\">"
                     << escapeHtml(stopColorMeaning(col)) << "</span></td>\n";
            }

            string curStop = engine.getCurrentStopName(*bus);
            string curStopDisplay = curStop.empty() ? "&mdash;" : escapeHtml(curStop);
            if (!curStop.empty() && bus->getState() == BusState::IN_SERVICE) {
                if (bus->isPickupJourney()) {
                    curStopDisplay += " <span class=\"chip c-blue\" style=\"font-size:0.7em;padding:1px 5px;\">Pickup</span>";
                } else {
                    curStopDisplay += " <span class=\"chip c-green\" style=\"font-size:0.7em;padding:1px 5px;\">Drop</span>";
                }
            }
            html << "          <td>" << curStopDisplay << "</td>\n";

            // Find stored ETA of first occurrence with an ETA on this route
            string etaText = "&mdash;";
            if (routeVisitsStop) {
                RouteStopNode* node = route->getHead();
                while (node != nullptr) {
                    if (node->getStopId() == stopId && node->hasEtaToNext()) {
                        etaText = to_string(node->getEtaToNextMinutes()) + " min";
                        if (node->getEtaIsProvisional()) {
                            etaText += " (provisional)";
                        }
                        break;
                    }
                    node = node->getNext();
                }
            }
            html << "          <td>" << etaText << "</td>\n";
            html << "        </tr>\n";
        }

        html << "      </tbody>\n";
        html << "    </table>\n";
        html << "  </div>\n";
        html << "</section>\n";
    }

    // 5. "All routes" boards
    html << "<h2 class=\"section-title\">All Routes</h2>\n";
    for (const Route* route : allRoutes) {
        html << "<h3>" << escapeHtml(route->getRouteId()) << " &mdash; " << escapeHtml(route->getRouteName()) << "</h3>\n";
        if (!route->getDescription().empty()) {
            html << "<p class=\"route-desc\">" << escapeHtml(route->getDescription()) << "</p>\n";
        }

        vector<Bus*> routeBuses = sortBusesForDisplay(busRegistry.getBusesForRoute(route->getRouteId()));

        html << "<div class=\"scroll\">\n";
        html << "  <table class=\"board\">\n";
        html << "    <thead>\n";
        html << "      <tr>\n";
        html << "        <th>#</th>\n";
        html << "        <th>Stop</th>\n";
        html << "        <th>ETA to next</th>\n";
        for (const Bus* b : routeBuses) {
            html << "        <th>Bus " << escapeHtml(b->getBusId()) << "</th>\n";
        }
        html << "      </tr>\n";
        html << "    </thead>\n";
        html << "    <tbody>\n";

        RouteStopNode* curr = route->getHead();
        while (curr != nullptr) {
            int pos = curr->getSequenceIndex();
            html << "      <tr>\n";
            html << "        <td>" << pos << "</td>\n";
            html << "        <td>" << escapeHtml(curr->getStopName());
            if (curr->getIsRestStop()) {
                html << " <span class=\"tag\">rest stop</span>";
            }
            html << "</td>\n";

            if (curr->hasEtaToNext()) {
                html << "        <td>" << curr->getEtaToNextMinutes() << " min";
                if (curr->getEtaIsProvisional()) {
                    html << " (provisional)";
                }
                html << "</td>\n";
            } else {
                html << "        <td>&mdash;</td>\n";
            }

            for (const Bus* b : routeBuses) {
                StopColor cellCol = b->getStatusForStop(pos);
                string cellClass = "cell " + colorToClass(cellCol);
                string cellTitle = "Bus " + b->getBusId() + ": " + stopColorMeaning(cellCol);

                html << "        <td class=\"" << cellClass << "\" title=\"" << escapeHtml(cellTitle) << "\">";
                if (b->getCurrentSequenceIndex() == pos) {
                    html << "&#9679;";
                }
                html << "</td>\n";
            }

            html << "      </tr>\n";
            curr = curr->getNext();
        }

        html << "    </tbody>\n";
        html << "  </table>\n";
        html << "</div>\n";
    }

    // 6. Footer Note
    html << "<p class=\"info-note\">&#9679; marks where a bus is right now. ETAs are stored values, not live predictions.</p>\n";

    // 7. Auto-refresh & Picker Script (15s cycle, pauses while interacting with dropdown)
    html << "<script>\n";
    html << "(function() {\n";
    html << "  var reloadTimer = null;\n";
    html << "  function startReloadTimer() {\n";
    html << "    if (reloadTimer) clearTimeout(reloadTimer);\n";
    html << "    reloadTimer = setTimeout(function(){ location.reload(); }, 15000);\n";
    html << "  }\n";
    html << "  function stopReloadTimer() {\n";
    html << "    if (reloadTimer) { clearTimeout(reloadTimer); reloadTimer = null; }\n";
    html << "  }\n";
    html << "  startReloadTimer();\n";
    html << "  var select = document.getElementById('stopSelect');\n";
    html << "  var panels = document.querySelectorAll('.stop-panel');\n";
    html << "  function showStop(id) {\n";
    html << "    panels.forEach(function(p) {\n";
    html << "      p.hidden = (p.getAttribute('data-stop') !== id);\n";
    html << "    });\n";
    html << "    select.value = id;\n";
    html << "    try { localStorage.setItem('selectedStop', id); } catch(e){}\n";
    html << "  }\n";
    html << "  select.addEventListener('focus', stopReloadTimer);\n";
    html << "  select.addEventListener('blur', startReloadTimer);\n";
    html << "  select.addEventListener('change', function() {\n";
    html << "    var val = select.value;\n";
    html << "    if (val) {\n";
    html << "      location.hash = 'stop=' + encodeURIComponent(val);\n";
    html << "    } else {\n";
    html << "      location.hash = '';\n";
    html << "    }\n";
    html << "    showStop(val);\n";
    html << "    startReloadTimer();\n";
    html << "  });\n";
    html << "  function restoreSelection() {\n";
    html << "    var m = location.hash.match(/stop=([^&]+)/);\n";
    html << "    var id = (m && m[1]) ? decodeURIComponent(m[1]) : '';\n";
    html << "    if (!id) {\n";
    html << "      try { id = localStorage.getItem('selectedStop') || ''; } catch(e){}\n";
    html << "    }\n";
    html << "    if (id) { showStop(id); }\n";
    html << "  }\n";
    html << "  window.addEventListener('load', restoreSelection);\n";
    html << "  restoreSelection();\n";
    html << "})();\n";
    html << "</script>\n";

    html << "</div>\n"; // container
    html << "</body>\n</html>\n";

    return atomicWrite(getStudentPagePath(), html.str());
}

bool HtmlGenerator::writeHodPage(const BusStatusEngine& engine) const {
    const RouteRegistry& routeRegistry = engine.getRouteRegistry();
    const BusRegistry& busRegistry = engine.getBusRegistry();
    vector<Bus*> allBusesSorted = sortBusesForDisplay(busRegistry.getAllBuses());
    vector<Update> history = engine.getHistory();

    int totalBuses = static_cast<int>(allBusesSorted.size());
    int inServiceCount = 0;
    int restingCount = 0;
    int maintenanceCount = 0;
    int notAvailableCount = 0;
    int tripsCompletedTotal = 0;

    for (const Bus* b : allBusesSorted) {
        switch (b->getState()) {
            case BusState::IN_SERVICE: inServiceCount++; break;
            case BusState::RESTING: restingCount++; break;
            case BusState::MAINTENANCE: maintenanceCount++; break;
            case BusState::NOT_AVAILABLE: notAvailableCount++; break;
        }
    }

    for (const Update& u : history) {
        if (u.getTripCompleted()) {
            tripsCompletedTotal++;
        }
    }

    ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>Department Head Report - Where's My Bus</title>\n";
    html << "<link rel=\"stylesheet\" href=\"" << escapeHtml(cssHref) << "\">\n";
    html << "<noscript><meta http-equiv=\"refresh\" content=\"15\"></noscript>\n";
    html << "<script>setTimeout(function(){ location.reload(); }, 15000);</script>\n";
    html << "</head>\n<body>\n";
    html << "<div class=\"container\">\n";
    html << renderNavBar("hod");

    // 1. Header
    html << "<h1>Department Head report</h1>\n";
    html << "<p class=\"subtitle\">Updated: " << Update::currentTimestamp() << "</p>\n";

    // 2. Metrics Cards
    html << "<div class=\"cards\">\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << totalBuses << "</div><div class=\"card-label\">Total Buses</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << inServiceCount << "</div><div class=\"card-label\">In Service</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << restingCount << "</div><div class=\"card-label\">Resting</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << maintenanceCount << "</div><div class=\"card-label\">Maintenance</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << notAvailableCount << "</div><div class=\"card-label\">Not Available</div></div>\n";
    html << "  <div class=\"card\"><div class=\"card-num\">" << tripsCompletedTotal << "</div><div class=\"card-label\">Trips Completed</div></div>\n";
    html << "</div>\n";

    // 3. Buses table
    html << "<h2>Fleet Overview</h2>\n";
    html << "<div class=\"scroll\">\n";
    html << "  <table>\n";
    html << "    <thead>\n";
    html << "      <tr><th>Bus</th><th>Route</th><th>Status</th><th>Position</th><th>Seats</th><th>Trips completed</th></tr>\n";
    html << "    </thead>\n";
    html << "    <tbody>\n";

    for (const Bus* bus : allBusesSorted) {
        html << "      <tr>\n";
        html << "        <td><strong>" << escapeHtml(bus->getBusId()) << "</strong></td>\n";
        html << "        <td>" << escapeHtml(bus->getRouteId()) << "</td>\n";

        StopColor busCol = bus->getStatusForStop(bus->getCurrentSequenceIndex());
        html << "        <td><span class=\"chip " << colorToClass(busCol) << "\">"
             << escapeHtml(stopColorMeaning(busCol)) << "</span></td>\n";

        int seq = bus->getCurrentSequenceIndex();
        if (seq == 0) {
            html << "        <td>&mdash;</td>\n";
        } else {
            string stopName = engine.getCurrentStopName(*bus);
            string dirTag = (bus->getState() == BusState::IN_SERVICE) ? (bus->isPickupJourney() ? " [Pickup]" : " [Drop]") : "";
            html << "        <td>" << escapeHtml(stopName) << " (#" << seq << ")" << dirTag << "</td>\n";
        }

        if (bus->getState() == BusState::IN_SERVICE) {
            html << "        <td>" << (bus->getHasFreeSeats() ? "Free" : "Full") << "</td>\n";
        } else {
            html << "        <td>&mdash;</td>\n";
        }

        // Count completed trips for this bus
        int busTrips = 0;
        for (const Update& u : history) {
            if (u.getBusId() == bus->getBusId() && u.getTripCompleted()) {
                busTrips++;
            }
        }
        html << "        <td>" << busTrips << "</td>\n";
        html << "      </tr>\n";
    }

    html << "    </tbody>\n";
    html << "  </table>\n";
    html << "</div>\n";

    // 4. Recent conductor updates table (newest 50)
    html << "<h2>Recent Conductor Updates</h2>\n";
    if (history.empty()) {
        html << "<p>No updates yet.</p>\n";
    } else {
        html << "<div class=\"scroll\">\n";
        html << "  <table>\n";
        html << "    <thead>\n";
        html << "      <tr><th>Time</th><th>Bus</th><th>Position</th><th>Status</th><th>Seats</th><th>Trip completed</th></tr>\n";
        html << "    </thead>\n";
        html << "    <tbody>\n";

        int limit = min(50, static_cast<int>(history.size()));
        for (int i = 0; i < limit; i++) {
            const Update& u = history[i];
            html << "      <tr>\n";
            html << "        <td>" << escapeHtml(u.getTimestamp()) << "</td>\n";
            html << "        <td>" << escapeHtml(u.getBusId()) << "</td>\n";

            int seq = u.getNewSequenceIndex();
            if (seq == 0) {
                html << "        <td>&mdash;</td>\n";
            } else {
                Route* r = routeRegistry.getRouteById(busRegistry.getBusById(u.getBusId())->getRouteId());
                RouteStopNode* node = (r != nullptr) ? r->findStopByIndex(seq) : nullptr;
                string sName = (node != nullptr) ? node->getStopName() : u.getNewStopId();
                html << "        <td>" << escapeHtml(sName) << " (#" << seq << ")</td>\n";
            }

            html << "        <td>" << escapeHtml(busStateToString(u.getNewState())) << "</td>\n";
            html << "        <td>" << (u.getNewSeatsAvailable() ? "Free" : "Full") << "</td>\n";
            if (u.getTripCompleted()) {
                html << "        <td><span class=\"tag\">trip completed</span></td>\n";
            } else {
                html << "        <td>&mdash;</td>\n";
            }
            html << "      </tr>\n";
        }

        html << "    </tbody>\n";
        html << "  </table>\n";
        html << "</div>\n";
    }

    // 5. Note
    html << "<p class=\"info-note\">The log lives in memory and restarts empty each run (the database has no history table). No delay statistics are required.</p>\n";

    html << "</div>\n"; // container
    html << "</body>\n</html>\n";

    return atomicWrite(getHodPagePath(), html.str());
}

bool HtmlGenerator::writeConductorPage(const BusStatusEngine& engine) const {
    const RouteRegistry& routeRegistry = engine.getRouteRegistry();
    const BusRegistry& busRegistry = engine.getBusRegistry();
    vector<Bus*> allBusesSorted = sortBusesForDisplay(busRegistry.getAllBuses());

    ostringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html lang=\"en\">\n<head>\n";
    html << "<meta charset=\"UTF-8\">\n";
    html << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    html << "<title>Conductor Portal - Where's My Bus</title>\n";
    html << "<link rel=\"stylesheet\" href=\"" << escapeHtml(cssHref) << "\">\n";
    html << "<noscript><meta http-equiv=\"refresh\" content=\"15\"></noscript>\n";
    html << "</head>\n<body>\n";
    html << "<div class=\"container\">\n";

    html << renderNavBar("conductor");

    html << "<h1>Conductor Station</h1>\n";
    html << "<p class=\"subtitle\">Auto-refreshes every 15s &bull; Updated: " << Update::currentTimestamp() << "</p>\n";

    // Bus picker dropdown
    html << "<div class=\"picker\">\n";
    html << "  <label for=\"conductorBusSelect\"><strong>Select Your Bus:</strong></label>\n";
    html << "  <select id=\"conductorBusSelect\">\n";
    html << "    <option value=\"\">-- choose your assigned bus --</option>\n";
    for (const Bus* b : allBusesSorted) {
        html << "    <option value=\"" << escapeHtml(b->getBusId()) << "\">Bus "
             << escapeHtml(b->getBusId()) << " (" << escapeHtml(b->getRouteId()) << ")</option>\n";
    }
    html << "  </select>\n";
    html << "</div>\n";

    // Per-bus panels (hidden until selected)
    for (const Bus* b : allBusesSorted) {
        Route* route = routeRegistry.getRouteById(b->getRouteId());
        RouteStopNode* currNode = (route != nullptr && b->getCurrentSequenceIndex() > 0)
                                      ? route->findStopByIndex(b->getCurrentSequenceIndex())
                                      : nullptr;

        RouteStopNode* prevNode = nullptr;
        RouteStopNode* nextNode = nullptr;
        if (currNode != nullptr) {
            if (b->getState() == BusState::RESTING || b->getDirection() == JourneyDirection::PICKUP) {
                // Inbound Pickup journey: moving backward along prev pointers
                nextNode = currNode->getPrev();
                prevNode = currNode->getNext();
            } else {
                // Outbound Drop journey: moving forward along next pointers
                nextNode = currNode->getNext();
                prevNode = currNode->getPrev();
            }
        }

        html << "<div class=\"conductor-panel\" data-bus=\"" << escapeHtml(b->getBusId()) << "\" hidden>\n";
        html << "  <div class=\"conductor-box\">\n";
        html << "    <div class=\"conductor-header\">\n";
        html << "      <span class=\"conductor-bus-title\">Bus " << escapeHtml(b->getBusId()) << "</span>\n";
        html << "      <span class=\"chip c-blue\">" << escapeHtml(b->getRouteId()) << "</span>\n";

        // Operational state chip
        StopColor stColor = b->getStatusForStop(b->getCurrentSequenceIndex());
        html << "      <span class=\"chip " << colorToClass(stColor) << "\">"
             << escapeHtml(busStateToString(b->getState())) << "</span>\n";

        // Seats and journey chips
        if (b->getState() == BusState::IN_SERVICE) {
            html << "      <span class=\"chip " << (b->getHasFreeSeats() ? "c-green" : "c-red") << "\">"
                 << (b->getHasFreeSeats() ? "Seats Free" : "Bus Full") << "</span>\n";
            html << "      <span class=\"chip " << (b->isPickupJourney() ? "c-blue" : "c-green") << "\">"
                 << (b->isPickupJourney() ? "Inbound Pickup Journey" : "Outbound Drop Journey") << "</span>\n";
        }
        html << "    </div>\n";

        if (route != nullptr) {
            html << "    <p class=\"route-desc\"><strong>Assigned Route:</strong> " << escapeHtml(route->getRouteName())
                 << " &mdash; " << escapeHtml(route->getDescription()) << "</p>\n";
        }

        // 3 Indicator boxes: Previous Stop, Current Stop, Next Stop
        html << "    <div class=\"stop-indicators\">\n";

        // Previous Stop
        html << "      <div class=\"stop-indicator-box\">\n";
        html << "        <div class=\"stop-indicator-label\">Previous Stop</div>\n";
        html << "        <div class=\"stop-indicator-value\">"
             << (prevNode != nullptr ? escapeHtml(prevNode->getStopName()) : "&mdash;") << "</div>\n";
        if (prevNode != nullptr) {
            html << "        <div class=\"stop-indicator-sub\">Stop #" << prevNode->getSequenceIndex() << "</div>\n";
        }
        html << "      </div>\n";

        // Current Stop
        html << "      <div class=\"stop-indicator-box\" style=\"border-color:var(--blue);background:#eff6ff;\">\n";
        html << "        <div class=\"stop-indicator-label\" style=\"color:var(--blue);\">Current Location</div>\n";
        string curName = engine.getCurrentStopName(*b);
        html << "        <div class=\"stop-indicator-value\">"
             << (curName.empty() ? "Shift Not Started" : escapeHtml(curName)) << "</div>\n";
        if (b->getCurrentSequenceIndex() > 0) {
            html << "        <div class=\"stop-indicator-sub\">Sequence #" << b->getCurrentSequenceIndex() << "</div>\n";
        }
        html << "      </div>\n";

        // Next Stop
        html << "      <div class=\"stop-indicator-box\">\n";
        html << "        <div class=\"stop-indicator-label\">Next Destination</div>\n";
        if (nextNode != nullptr) {
            html << "        <div class=\"stop-indicator-value\">" << escapeHtml(nextNode->getStopName()) << "</div>\n";
            int eta = (b->getDirection() == JourneyDirection::PICKUP) ? nextNode->getEtaToNextMinutes() : (currNode != nullptr ? currNode->getEtaToNextMinutes() : -1);
            if (eta > 0) {
                html << "        <div class=\"stop-indicator-sub\">Stored ETA: " << eta << " min</div>\n";
            } else {
                html << "        <div class=\"stop-indicator-sub\">Stop #" << nextNode->getSequenceIndex() << "</div>\n";
            }
        } else if (b->getState() == BusState::NOT_AVAILABLE) {
            html << "        <div class=\"stop-indicator-value\">Ready to Start</div>\n";
            html << "        <div class=\"stop-indicator-sub\">Select Option 0 or 1 in terminal</div>\n";
        } else {
            html << "        <div class=\"stop-indicator-value\">Final Destination</div>\n";
            html << "        <div class=\"stop-indicator-sub\">Trip complete on next advance</div>\n";
        }
        html << "      </div>\n";

        html << "    </div>\n"; // stop-indicators

        // Operational help box
        html << "    <div class=\"help-box\">\n";
        html << "      <strong>Conductor Quick Actions:</strong> Open the <code>wheresmybus.exe</code> console window &rarr; Enter <strong>2 (Conductor)</strong> &rarr; Bus <strong>"
             << escapeHtml(b->getBusId()) << "</strong>. Use Option 1 to advance stops, Option 2 to toggle seats full/free, Option 3 to report maintenance, and Option 4 to undo mistakes. Updates appear here and on student phones automatically.\n";
        html << "    </div>\n";

        html << "  </div>\n"; // conductor-box
        html << "</div>\n";   // conductor-panel
    }

    // All fleet summary table
    html << "<h2 class=\"section-title\">All Fleet Status</h2>\n";
    html << "<div class=\"scroll\">\n";
    html << "  <table>\n";
    html << "    <thead>\n";
    html << "      <tr><th>Bus</th><th>Route</th><th>Operational State</th><th>Current Location</th><th>Journey</th><th>Seats</th></tr>\n";
    html << "    </thead>\n";
    html << "    <tbody>\n";
    for (const Bus* bus : allBusesSorted) {
        html << "      <tr>\n";
        html << "        <td><strong>Bus " << escapeHtml(bus->getBusId()) << "</strong></td>\n";
        html << "        <td>" << escapeHtml(bus->getRouteId()) << "</td>\n";
        StopColor col = bus->getStatusForStop(bus->getCurrentSequenceIndex());
        html << "        <td><span class=\"chip " << colorToClass(col) << "\">"
             << escapeHtml(busStateToString(bus->getState())) << "</span></td>\n";
        string stopName = engine.getCurrentStopName(*bus);
        html << "        <td>" << (stopName.empty() ? "&mdash;" : escapeHtml(stopName)) << "</td>\n";
        html << "        <td>" << (bus->getState() == BusState::IN_SERVICE ? (bus->isPickupJourney() ? "<span class=\"chip c-blue\">Pickup</span>" : "<span class=\"chip c-green\">Drop</span>") : "&mdash;") << "</td>\n";
        html << "        <td>" << (bus->getState() == BusState::IN_SERVICE ? (bus->getHasFreeSeats() ? "Seats Free" : "FULL") : "&mdash;") << "</td>\n";
        html << "      </tr>\n";
    }
    html << "    </tbody>\n";
    html << "  </table>\n";
    html << "</div>\n";

    // Conductor Javascript (15s auto-reload + dropdown selection persistence)
    html << "<script>\n";
    html << "(function() {\n";
    html << "  var reloadTimer = null;\n";
    html << "  function startReloadTimer() {\n";
    html << "    if (reloadTimer) clearTimeout(reloadTimer);\n";
    html << "    reloadTimer = setTimeout(function(){ location.reload(); }, 15000);\n";
    html << "  }\n";
    html << "  function stopReloadTimer() {\n";
    html << "    if (reloadTimer) { clearTimeout(reloadTimer); reloadTimer = null; }\n";
    html << "  }\n";
    html << "  startReloadTimer();\n";
    html << "  var select = document.getElementById('conductorBusSelect');\n";
    html << "  var panels = document.querySelectorAll('.conductor-panel');\n";
    html << "  function showBus(id) {\n";
    html << "    panels.forEach(function(p) {\n";
    html << "      p.hidden = (p.getAttribute('data-bus') !== id);\n";
    html << "    });\n";
    html << "    select.value = id;\n";
    html << "    try { localStorage.setItem('selectedConductorBus', id); } catch(e){}\n";
    html << "  }\n";
    html << "  select.addEventListener('focus', stopReloadTimer);\n";
    html << "  select.addEventListener('blur', startReloadTimer);\n";
    html << "  select.addEventListener('change', function() {\n";
    html << "    var val = select.value;\n";
    html << "    if (val) {\n";
    html << "      location.hash = 'bus=' + encodeURIComponent(val);\n";
    html << "    } else {\n";
    html << "      location.hash = '';\n";
    html << "    }\n";
    html << "    showBus(val);\n";
    html << "    startReloadTimer();\n";
    html << "  });\n";
    html << "  function restoreSelection() {\n";
    html << "    var m = location.hash.match(/bus=([^&]+)/);\n";
    html << "    var id = (m && m[1]) ? decodeURIComponent(m[1]) : '';\n";
    html << "    if (!id) {\n";
    html << "      try { id = localStorage.getItem('selectedConductorBus') || ''; } catch(e){}\n";
    html << "    }\n";
    html << "    if (id) { showBus(id); }\n";
    html << "  }\n";
    html << "  window.addEventListener('load', restoreSelection);\n";
    html << "  restoreSelection();\n";
    html << "})();\n";
    html << "</script>\n";

    html << "</div>\n";
    html << "</body>\n</html>\n";

    return atomicWrite(getConductorPagePath(), html.str());
}
