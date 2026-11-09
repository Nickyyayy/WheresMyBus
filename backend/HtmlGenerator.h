#ifndef HTML_GENERATOR_H
#define HTML_GENERATOR_H

#include "BusStatusEngine.h"
#include <string>

using namespace std;

// HtmlGenerator generates self-refreshing student dashboard and HOD report web pages
class HtmlGenerator {
private:
    string outputDir;
    string cssHref;

    static string escapeHtml(const string& text);
    static string colorToClass(StopColor color);
    static string renderNavBar(const string& activePage);
    bool atomicWrite(const string& targetPath, const string& content) const;

public:
    HtmlGenerator(string outputDir, string cssHref);

    bool writeHomePage(const BusStatusEngine& engine) const;
    bool writeStudentPage(const BusStatusEngine& engine) const;
    bool writeConductorPage(const BusStatusEngine& engine) const;
    bool writeHodPage(const BusStatusEngine& engine) const;
    bool writeStateJson(const BusStatusEngine& engine, const string& lastMessage = "") const;

    string getHomePagePath() const;
    string getStudentPagePath() const;
    string getConductorPagePath() const;
    string getHodPagePath() const;
    string getStateJsonPath() const;
};

#endif // HTML_GENERATOR_H
