#include "Config.h"
#include <fstream>

using namespace std;

string Config::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

bool Config::load(const string& path) {
    settings.clear();
    lastError = "";

    ifstream file(path);
    if (!file.is_open()) {
        lastError = "Could not open configuration file at: " + path;
        return false;
    }

    string line;
    while (getline(file, line)) {
        string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue; // Skip empty lines and full-line comments
        }

        size_t equalPos = trimmed.find('=');
        if (equalPos == string::npos) {
            continue; // Not a key=value pair
        }

        string key = trim(trimmed.substr(0, equalPos));
        string value = trim(trimmed.substr(equalPos + 1));

        // Strip inline comments only if key is not db_password (passwords may contain '#')
        if (key != "db_password") {
            size_t commentPos = value.find(" #");
            if (commentPos != string::npos) {
                value = trim(value.substr(0, commentPos));
            } else if (!value.empty() && value[0] == '#') {
                value = "";
            }
        }

        if (!key.empty()) {
            settings[key] = value;
        }
    }

    return true;
}

bool Config::has(const string& key) const {
    return settings.find(key) != settings.end();
}

string Config::get(const string& key, const string& fallback) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return fallback;
}

string Config::getLastError() const {
    return lastError;
}
