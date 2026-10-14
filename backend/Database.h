#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

using namespace std;

// Database manages communication with MySQL by launching mysql.exe as a child process
// Never links external MySQL libraries
class Database {
private:
    string host;
    unsigned int port;
    string user;
    string password;
    string dbName;
    string clientPath;
    bool connected;
    string lastError;

    bool executeQuery(const string& sql, vector<vector<string>>& rowsOut);
    static string inQuotes(const string& path);
    static string escapeCnfValue(const string& value);

public:
    Database(string host, unsigned int port, string user, string password, string dbName);

    void setClientPath(const string& path);
    bool connect();
    bool isConnected() const;
    bool query(const string& sql, vector<vector<string>>& rowsOut);
    string escape(const string& text) const;
    string getLastError() const;
};

#endif // DATABASE_H
