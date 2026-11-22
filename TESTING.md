# Testing & Validation – Where's My Bus

This document lists the test cases used to validate **Where's My Bus** against its objectives.
Run every test on the **demo laptop**. Fill in the *Actual Result* and *Status* columns.
Save screenshots or console output in `tests/evidence/` and give each file the test ID as its name (for example `ST-02.png`).

Status values: **Pass** · **Fail** · **Blocked** (could not be run)

---

## 1. Test Environment

| Item | Value |
|---|---|
| OS | Windows ___ |
| Compiler | g++ ___ (built with `build.bat`) |
| Database | MySQL ___ (`where_my_bus`, loaded with `load_database.bat`) |
| Python | ___ (`start_server.bat` → `server.py`, port 8080) |
| Browsers / devices | Laptop: ___ · Phone on same Wi-Fi: ___ |
| Commit tested | `git rev-parse --short HEAD` → ___ |
| Tested by / date | ___ |

---

## 2. Objective Traceability

| Objective | Covered by |
|---|---|
| O1 – Colour status of every bus at a chosen stop, no login | ST-02, ST-03, ST-04, ST-07, ST-09, UT-06 |
| O2 – One-tap conductor update with route validation | ST-01, ST-02, ST-06, NT-01, NT-02, NT-05, NT-06 |
| O3 – Drop and pickup journeys | UT-04, ST-08 |
| O4 – HOD report | ST-10 |
| O5 – Low-cost, data from MySQL, hand-written data structures | IT-01, IT-02, UT-01 – UT-05 |

---

## 3. Unit Tests (data structures and classes)

| ID | Component | Steps | Expected Result | Actual Result | Status |
|---|---|---|---|---|---|
| UT-01 | `Queue<T>` | Enqueue 1, 2, 3; dequeue three times; dequeue once more on the empty queue | Returns 1, 2, 3 in order; empty queue is reported safely (no crash) | | |
| UT-02 | `Stack<T>` | Push A, B, C; pop three times; pop once more on the empty stack | Returns C, B, A; empty stack is reported safely | | |
| UT-03 | `BinaryHeap` + `BusDisplayPriority` | Sort a mix of in-service, resting, maintenance and off-duty buses at different stops | Active buses first; among them, the bus further along the route first; ties broken by smaller bus ID | | |
| UT-04 | `Route` (doubly linked list) | Build a route; walk head → tail with `next`, then tail → head with `prev` | Both walks visit the same stops in opposite order; `totalStops` matches | | |
| UT-05 | `Route::addStopToEnd` | Add a node whose sequence index is not `totalStops + 1` | Node is rejected; route is unchanged | | |
| UT-06 | `BusRegistry` | Add a bus; add another bus with the same ID; look up a missing ID | Duplicate rejected; missing ID returns `nullptr` | | |
| UT-07 | `Bus::getStatusForStop` | Check a stop ahead of the bus and a stop already passed, for both DROP and PICKUP, with seats free and full | Ahead + free = Green; ahead + full = Blue; passed = Red; direction respected | | |

---

## 4. Integration Tests

| ID | Path tested | Steps | Expected Result | Actual Result | Status |
|---|---|---|---|---|---|
| IT-01 | C++ → MySQL | Start the program with a correct `config.txt` | Connection succeeds (test query `SELECT 1`) | | |
| IT-02 | MySQL → DataLoader → registries | Run `SELECT COUNT(*) FROM routes;` and `SELECT COUNT(*) FROM buses;` in MySQL, then start the program | Number of routes and buses loaded equals the SQL counts | | |
| IT-03 | Config error handling | Put a wrong password in `config.txt` and start | Clear error message; program does not crash | | |
| IT-04 | `server.py` → `state.json` | Open `http://localhost:8080/api/status` | Returns the JSON state with a `revision` field | | |
| IT-05 | `server.py` fallback | Rename `output/state.json` temporarily, open `/api/status` | Returns the "initializing" fallback JSON instead of an error page | | |
| IT-06 | Browser → server → WebBridge → engine | Send one conductor action from the conductor page | Action is applied; `revision` in `state.json` increases by at least 1 | | |

---

## 5. System Tests (end-to-end, through the web portals)

| ID | Scenario | Steps | Expected Result | Actual Result | Status |
|---|---|---|---|---|---|
| ST-01 | Start shift at college | Conductor logs in as a bus → Start shift | Bus state In service at stop 1; shows Green where seats are free | | |
| ST-02 | REACHED! | Tap REACHED! once | Bus moves to the next stop only; student page updates without manual reload | | |
| ST-03 | Bus full | Mark seats full | Bus shows Blue at stops it has not reached | | |
| ST-04 | Maintenance | Set maintenance on | Bus shows Purple; HOD maintenance count increases | | |
| ST-05 | Off duty | Go off duty | Bus shows Black; position reset | | |
| ST-06 | Undo | Tap REACHED!, then Undo | Bus returns to the previous stop and previous state | | |
| ST-07 | Start at rest stop | Start shift at rest stop | Bus shows Brown (Resting) | | |
| ST-08 | Full pickup trip | Run a bus from the rest stop back to Graphic Era | Every stop reached in reverse order with Pickup tag; trip completes at Graphic Era | | |
| ST-09 | Missed bus | Student selects a stop the bus has already passed | That bus shows Red for this stop | | |
| ST-10 | HOD report | Open HOD page after ST-01 to ST-08 | Totals (in service, resting, maintenance, not available, trips completed) match the actions performed | | |
| ST-11 | Multi-device sync | Student page open on a phone (LAN IP) while conductor taps on the laptop | Phone updates on its own within a few seconds | | |

---

## 6. Negative / Invalid Input Tests

| ID | Scenario | Steps | Expected Result | Actual Result | Status |
|---|---|---|---|---|---|
| NT-01 | Stop jump | Try to move a bus to any stop other than the next one | Not possible; only the next stop in the route is accepted | | |
| NT-02 | Last stop | Tap REACHED! at the final stop of the route | Trip completes or a clear message is shown; no crash, no invalid position | | |
| NT-03 | Action before shift | Tap REACHED! on a bus that has not started a shift | Rejected with a clear message; state unchanged | | |
| NT-04 | Undo with no history | Tap Undo straight after starting the program | Clear message; no crash | | |
| NT-05 | Wrong conductor login | Log in with a wrong password | Login refused with a clear message | | |
| NT-06 | Bad API request | POST to `/api/action` with an empty `busId` or `action` | Error response; no state change | | |
| NT-07 | Backend not running | Close `wheresmybus.exe`, then send a conductor action | Server tries to launch the backend; if there is no reply within 4 s, the server returns 504 | | |

---

## 7. Non-Functional Tests

| ID | Check | Steps | Expected Result | Actual Result | Status |
|---|---|---|---|---|---|
| NF-01 | Compilation | Build with warnings on (`-Wall -Wextra`) | No warnings | | |
| NF-02 | Memory | Run a full shift with a memory checker, then exit | No leaks reported | | |
| NF-03 | Atomic page writes | Tap REACHED! rapidly 10 times while a page is open | Page never shows half-written or broken HTML | | |
| NF-04 | Hardware independence | Run the full demo with no GPS device or paid API | All features work | | |

---

## 8. Summary

| Group | Total | Pass | Fail | Blocked |
|---|---|---|---|---|
| Unit | 7 | | | |
| Integration | 6 | | | |
| System | 11 | | | |
| Negative | 7 | | | |
| Non-functional | 4 | | | |
| **Total** | **35** | | | |

**Phase-II baseline:** 8 of 9 test groups passed. The failing group was pickup tracking, caused by the forward-only route list. ST-08 and UT-04 re-test this after the change to a doubly linked list.

**Known limitations (not counted as failures):** ETAs are stored per segment and are not computed from live traffic. Accuracy depends on the conductor tapping at every stop.
