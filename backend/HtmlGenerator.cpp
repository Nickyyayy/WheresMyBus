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

string HtmlGenerator::getStudentPagePath() const {
    return outputDir + "/student.html";
}

string HtmlGenerator::getHodPagePath() const {
    return outputDir + "/hod.html";
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
    html << "<script>setTimeout(function(){ location.reload(); }, 2000);</script>\n";
    html << "<noscript><meta http-equiv=\"refresh\" content=\"2\"></noscript>\n";
    html << "</head>\n<body>\n";
    html << "<div class=\"container\">\n";

    // 1. Header
    html << "<h1>Where's My Bus</h1>\n";
    html << "<p class=\"subtitle\">Updated: " << Update::currentTimestamp() << "</p>\n";

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
            html << "          <td>" << (curStop.empty() ? "&mdash;" : escapeHtml(curStop)) << "</td>\n";

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

    // 7. Picker Script
    html << "<script>\n";
    html << "(function() {\n";
    html << "  var select = document.getElementById('stopSelect');\n";
    html << "  var panels = document.querySelectorAll('.stop-panel');\n";
    html << "  function showStop(id) {\n";
    html << "    panels.forEach(function(p) {\n";
    html << "      p.hidden = (p.getAttribute('data-stop') !== id);\n";
    html << "    });\n";
    html << "    select.value = id;\n";
    html << "  }\n";
    html << "  select.addEventListener('change', function() {\n";
    html << "    var val = select.value;\n";
    html << "    if (val) {\n";
    html << "      location.hash = 'stop=' + encodeURIComponent(val);\n";
    html << "    } else {\n";
    html << "      location.hash = '';\n";
    html << "    }\n";
    html << "    showStop(val);\n";
    html << "  });\n";
    html << "  function loadFromHash() {\n";
    html << "    var m = location.hash.match(/stop=([^&]+)/);\n";
    html << "    if (m && m[1]) {\n";
    html << "      var id = decodeURIComponent(m[1]);\n";
    html << "      showStop(id);\n";
    html << "    }\n";
    html << "  }\n";
    html << "  window.addEventListener('load', loadFromHash);\n";
    html << "  loadFromHash();\n";
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
    html << "</head>\n<body>\n";
    html << "<div class=\"container\">\n";

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
            html << "        <td>" << escapeHtml(stopName) << " (#" << seq << ")</td>\n";
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
