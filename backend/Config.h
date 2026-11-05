#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

using namespace std;

// Config loads key=value configuration pairs from a text file
class Config {
private:
    unordered_map<string, string> settings;
    string lastError;

    static string trim(const string& str);

public:
    Config() = default;

    bool load(const string& path);
    bool has(const string& key) const;
    string get(const string& key, const string& fallback = "") const;
    string getLastError() const;
};

#endif // CONFIG_H
