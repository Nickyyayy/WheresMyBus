#include "Database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>

#if defined(__has_include) && __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#define HAS_STD_FILESYSTEM 1
#else
#define HAS_STD_FILESYSTEM 0
#endif

using namespace std;

Database::Database(string host, unsigned int port, string user, string password, string dbName)
    : host(move(host)),
      port(port),
      user(move(user)),
      password(move(password)),
      dbName(move(dbName)),
      clientPath("mysql"),
      connected(false),
      lastError("") {}

void Database::setClientPath(const string& path) {
    if (!path.empty()) {
        clientPath = path;
    }
}

string Database::inQuotes(const string& path) {
    return "\"" + path + "\"";
}

string Database::escapeCnfValue(const string& value) {
    string result;
    for (char c : value) {
        if (c == '\\' || c == '"') {
            result.push_back('\\');
        }
        result.push_back(c);
    }
    return result;
}

string Database::escape(const string& text) const {
    string result;
    for (char c : text) {
        if (c == '\\') {
            result += "\\\\";
        } else if (c == '\'') {
            result += "\\'";
        } else {
            result.push_back(c);
        }
    }
    return result;
}

bool Database::connect() {
    connected = false;
    vector<vector<string>> testRows;
    if (executeQuery("SELECT 1;", testRows)) {
        connected = true;
        return true;
    }
    return false;
}

bool Database::isConnected() const {
    return connected;
}

bool Database::query(const string& sql, vector<vector<string>>& rowsOut) {
    if (!connected) {
        lastError = "Database is not connected. Call connect() first.";
        return false;
    }
    return executeQuery(sql, rowsOut);
}

string Database::getLastError() const {
    return lastError;
}

bool Database::executeQuery(const string& sql, vector<vector<string>>& rowsOut) {
    rowsOut.clear();
    lastError = "";

    // Determine temporary folder
    string tempDir = ".";
#if HAS_STD_FILESYSTEM
    try {
        tempDir = fs::temp_directory_path().string();
    } catch (...) {
        tempDir = ".";
    }
#else
#ifdef _WIN32
    const char* t = getenv("TEMP");
    if (!t) t = getenv("TMP");
    if (t) tempDir = string(t);
#else
    const char* t = getenv("TMPDIR");
    if (t) tempDir = string(t);
    else tempDir = "/tmp";
#endif
#endif

    // Replace backslashes with slashes for consistency
    for (char& c : tempDir) {
        if (c == '\\') c = '/';
    }
    if (!tempDir.empty() && tempDir.back() == '/') {
        tempDir.pop_back();
    }

    string cnfPath = tempDir + "/wheresmybus_client.cnf";
    string sqlPath = tempDir + "/wheresmybus_query.sql";
    string outPath = tempDir + "/wheresmybus_out.txt";
    string errPath = tempDir + "/wheresmybus_err.txt";

    // 1. Write the client option file with credentials
    {
        ofstream cnfFile(cnfPath, ios::trunc);
        if (!cnfFile.is_open()) {
            lastError = "Failed to create temporary MySQL config file: " + cnfPath;
            return false;
        }
        cnfFile << "[client]\n";
        cnfFile << "host=\"" << escapeCnfValue(host) << "\"\n";
        cnfFile << "port=" << port << "\n";
        cnfFile << "user=\"" << escapeCnfValue(user) << "\"\n";
        cnfFile << "password=\"" << escapeCnfValue(password) << "\"\n";
    }

    // 2. Write SQL text to query file
    {
        ofstream sqlFile(sqlPath, ios::trunc);
        if (!sqlFile.is_open()) {
            lastError = "Failed to create temporary SQL query file: " + sqlPath;
            remove(cnfPath.c_str());
            return false;
        }
        sqlFile << sql << "\n";
    }

    // 3. Assemble command line
    // "<clientPath>" --defaults-extra-file="<cnf>" --batch --raw --skip-column-names --default-character-set=utf8mb4 --database="<db>" < "<sql>" > "<out>" 2> "<err>"
    string cmd = inQuotes(clientPath) +
                 " --defaults-extra-file=" + inQuotes(cnfPath) +
                 " --batch --raw --skip-column-names --default-character-set=utf8mb4" +
                 " --database=" + inQuotes(dbName) +
                 " < " + inQuotes(sqlPath) +
                 " > " + inQuotes(outPath) +
                 " 2> " + inQuotes(errPath);

#ifdef _WIN32
    // On Windows, wrap entire command in extra double quotes so cmd.exe does not strip them
    string systemCmd = "\"" + cmd + "\"";
#else
    string systemCmd = cmd;
#endif

    // 4. Run command
    int exitCode = system(systemCmd.c_str());

    // 5. Read stdout and stderr files
    string stdOutContent;
    {
        ifstream outFile(outPath);
        if (outFile.is_open()) {
            stringstream ss;
            ss << outFile.rdbuf();
            stdOutContent = ss.str();
        }
    }

    string stdErrContent;
    {
        ifstream errFile(errPath);
        if (errFile.is_open()) {
            stringstream ss;
            ss << errFile.rdbuf();
            stdErrContent = ss.str();
        }
    }

    // 6. Delete all four scratch files
    remove(cnfPath.c_str());
    remove(sqlPath.c_str());
    remove(outPath.c_str());
    remove(errPath.c_str());

    // Trim stderr text
    size_t firstErr = stdErrContent.find_first_not_of(" \t\r\n");
    string trimmedErr;
    if (firstErr != string::npos) {
        size_t lastErr = stdErrContent.find_last_not_of(" \t\r\n");
        trimmedErr = stdErrContent.substr(firstErr, (lastErr - firstErr + 1));
    }

    if (exitCode != 0) {
        if (trimmedErr.empty()) {
            lastError = "the mysql program failed (exit code " + to_string(exitCode) + ")\n(command tried: " + cmd + ")";
        } else {
            lastError = trimmedErr + "\n(command tried: " + cmd + ")";
        }
        return false;
    }

    // 7. Parse stdout: rows separated by newline, columns by TAB (\t)
    stringstream ss(stdOutContent);
    string line;
    while (getline(ss, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }

        vector<string> row;
        size_t start = 0;
        while (start <= line.size()) {
            size_t tabPos = line.find('\t', start);
            if (tabPos == string::npos) {
                string cell = line.substr(start);
                row.push_back(cell == "NULL" ? "" : cell);
                break;
            } else {
                string cell = line.substr(start, tabPos - start);
                row.push_back(cell == "NULL" ? "" : cell);
                start = tabPos + 1;
            }
        }
        rowsOut.push_back(move(row));
    }

    return true;
}
