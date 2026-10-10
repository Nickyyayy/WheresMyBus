#ifndef ROUTE_VALIDATION_H
#define ROUTE_VALIDATION_H

#include "Route.h"
#include <string>
#include <vector>

using namespace std;

// Validates that the loaded stops along a route match an expected list of stop IDs in order
bool isValidStopSequence(const Route* route, const vector<string>& claimedStopIds);

// Verifies that a given sequenceIndex exists on the route and corresponds to stopId
bool isValidPosition(const Route* route, const string& stopId, int sequenceIndex);

// Checks foundational route structural constraints (>=2 stops, head == startStopId, contains restStopId)
bool isValidRouteStructure(const Route* route, string& problemOut);

#endif // ROUTE_VALIDATION_H
