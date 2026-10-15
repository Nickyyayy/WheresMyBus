#include "Config.h"
#include "Database.h"
#include "DataLoader.h"
#include "RouteRegistry.h"
#include "BusRegistry.h"
#include "Queue.h"
#include "Stack.h"
#include "BusStatusEngine.h"
#include "HtmlGenerator.h"
#include "ConsoleMenu.h"

#include <iostream>
#include <string>
#include <cstdlib>

#if defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#define HAS_STD_FILESYSTEM 1
#else
#define HAS_STD_FILESYSTEM 0
#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#endif

using namespace std;

const string CONFIG_FILE = "config.txt";
const string OUTPUT_DIR = "output";
const string CSS_HREF = "../frontend/style.css";

void waitForEnter() {
    cout << "\nPress Enter to close..." << endl;
    string dummy;
    getline(cin, dummy);
}

int main() {
    try {
        cout << "Starting Where's My Bus..." << endl;

        // 1. Load config.txt
        Config config;
        if (!config.load(CONFIG_FILE)) {
            cout << "Could not find " << CONFIG_FILE << "." << endl;
            cout << "Please copy config.example.txt to config.txt and configure your MySQL settings." << endl;
            waitForEnter();
            return 1;
        }

        // 2. Validate credentials
        string user = config.get("db_user");
        string pass = config.get("db_password");
        if (user.empty() || user.rfind("PLACEHOLDER", 0) == 0 || pass.rfind("PLACEHOLDER", 0) == 0) {
            cout << "Please open config.txt and fill in db_user and db_password." << endl;
            waitForEnter();
            return 1;
        }

        // 3. Parse port
        unsigned int port = 3306;
        string portStr = config.get("db_port", "3306");
        try {
            port = static_cast<unsigned int>(stoul(portStr));
        } catch (...) {
            cout << "Invalid db_port specified in config.txt. Defaulting to 3306." << endl;
            port = 3306;
        }

        string host = config.get("db_host", "localhost");
        string dbName = config.get("db_name", "where_my_bus");
        string mysqlPath = config.get("mysql_path", "mysql");

        // 4. Ensure output directory exists
#if HAS_STD_FILESYSTEM
        try {
            fs::create_directories(OUTPUT_DIR);
        } catch (...) {}
#else
#ifdef _WIN32
        _mkdir(OUTPUT_DIR.c_str());
#else
        mkdir(OUTPUT_DIR.c_str(), 0755);
#endif
#endif

        // 5. Connect to MySQL database
        Database db(host, port, user, pass, dbName);
        db.setClientPath(mysqlPath);

        if (!db.connect()) {
            cout << "Database connection failed:" << endl;
            cout << db.getLastError() << endl << endl;
            cout << "Troubleshooting tips:" << endl;
            cout << " - If 'mysql' is not recognized, set the full path to mysql.exe in config.txt (mysql_path=...)" << endl;
            cout << " - If access was denied, check db_user and db_password in config.txt" << endl;
            cout << " - If database is unknown, run load_database.bat or load sql/a.sql and sql/buses.sql" << endl;
            waitForEnter();
            return 1;
        }

        // 6. Load domain data from MySQL
        RouteRegistry routeRegistry;
        BusRegistry busRegistry;
        DataLoader loader(db);

        if (!loader.loadRoutes(routeRegistry)) {
            cout << "Failed to load routes: " << loader.getLastError() << endl;
            waitForEnter();
            return 1;
        }

        if (!loader.loadBuses(busRegistry, routeRegistry)) {
            cout << "Failed to load buses: " << loader.getLastError() << endl;
            waitForEnter();
            return 1;
        }

        cout << "Loaded " << routeRegistry.getRouteCount() << " routes and "
             << busRegistry.getBusCount() << " buses from MySQL." << endl;

        // 7. Initialize operational engine and update structures
        Queue<Update> pendingUpdates;
        Stack<Update> updateHistory;
        BusStatusEngine engine(routeRegistry, busRegistry, pendingUpdates, updateHistory);

        // 8. Generate initial HTML dashboards
        HtmlGenerator html(OUTPUT_DIR, CSS_HREF);
        if (!html.writeStudentPage(engine) || !html.writeHodPage(engine)) {
            cout << "Warning: Failed to generate initial web pages in output directory." << endl;
        }

        string absStudentPath = html.getStudentPagePath();
#if HAS_STD_FILESYSTEM
        try {
            absStudentPath = fs::absolute(html.getStudentPagePath()).string();
        } catch (...) {}
#else
#ifdef _WIN32
        char fullPathBuf[MAX_PATH];
        DWORD len = GetFullPathNameA(html.getStudentPagePath().c_str(), MAX_PATH, fullPathBuf, nullptr);
        if (len > 0) {
            absStudentPath = string(fullPathBuf);
        }
#endif
#endif
        cout << "Student page: " << absStudentPath << endl;

        // 9. Open browser on Windows if configured
#ifdef _WIN32
        string openBrowser = config.get("open_browser", "yes");
        if (openBrowser != "no") {
            string openCmd = "start \"\" \"" + absStudentPath + "\"";
            system(openCmd.c_str());
        }
#endif

        // 10. Run terminal menu
        ConsoleMenu menu(engine, html);
        menu.run();

        return 0;
    } catch (const exception& e) {
        cout << "Fatal error: " << e.what() << endl;
        waitForEnter();
        return 1;
    }
}
