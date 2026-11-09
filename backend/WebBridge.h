#ifndef WEB_BRIDGE_H
#define WEB_BRIDGE_H

#include "BusStatusEngine.h"
#include "HtmlGenerator.h"
#include <string>

using namespace std;

// WebBridge handles the event listener executing conductor actions from the web interface
class WebBridge {
private:
    BusStatusEngine& engine;
    HtmlGenerator& html;
    string requestFile;
    string responseFile;

    static string parseJsonField(const string& json, const string& key);
    static void writeResponse(const string& path, bool ok, const string& message);
    ActionResult executeAction(const string& busId, const string& action);

public:
    WebBridge(BusStatusEngine& engine, HtmlGenerator& html,
              string requestFile = "command_request.json",
              string responseFile = "command_response.json");

    void refreshAll(const string& message = "");
    void run(); // Main engine loop
};

#endif // WEB_BRIDGE_H
