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
    bool atomicWrite(const string& targetPath, const string& content) const;

public:
    HtmlGenerator(string outputDir, string cssHref);

    bool writeStudentPage(const BusStatusEngine& engine) const;
    bool writeHodPage(const BusStatusEngine& engine) const;

    string getStudentPagePath() const;
    string getHodPagePath() const;
};

#endif // HTML_GENERATOR_H
