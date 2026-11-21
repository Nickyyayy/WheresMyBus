# Where's My Bus

> **Hardware-Independent Transit Tracking & Fleet Supervision System**  
> Developed for Academic Courses **TCS 302 / TCS 307** by Team **"Null Pointer"**.

---

## 1. Executive Summary & Core Philosophy

**Where's My Bus** is a transit tracking and fleet management system engineered to operate **without requiring GPS receivers, IoT tracking modules, cellular SIM cards, or dedicated vehicle hardware**.

### The Conventional Problem
Conventional fleet tracking systems rely on dedicated GPS hardware installed in every bus. These devices compute geographical coordinates and stream them over cellular networks to cloud servers. While functional, this hardware-dependent model introduces:
- Substantial upfront capital expenditure per vehicle.
- Recurring cellular data and cloud platform subscription costs.
- Hardware maintenance overhead, signal dead zones, and installation downtime.

### The Hardware-Independent Solution
College transit buses follow **strict, predefined routes** with fixed stops. Instead of computing physical latitude/longitude coordinates, **Where's My Bus** uses a **conductor-driven state machine model**:

```text
Predefined Route in MySQL:
[Graphic Era (College)] ──> [Clement Town] ──> [Subhash Nagar] ──> [ISBT] ──> [Kargi Chowk (Rest Stop)]

Operational Flow:
1. Conductor starts shift at origin (Bus position = Graphic Era)
2. Bus travels to next stop
3. Conductor taps "Reached Next Stop"
4. C++ Core advances bus position to Clement Town (Index 1 ──> 2)
5. C++ Core recalculates relative stop colors and segment ETAs
6. Web dashboards update automatically across all student and supervisory screens
```

The conductor does **not** type stop names or coordinates. The software enforces route topology and advances the vehicle strictly along its verified path.

### Intentional Design Tradeoff
Because position tracking is driven by conductor confirmations rather than continuous satellite telemetry, the reported state reflects the conductor's timely submissions. If a conductor delays an update, the system reports the last confirmed position. This is a deliberate, documented architectural tradeoff designed for deterministic transit corridors.

---

## 2. System Architecture & Data Flow

The system is built as a multi-tier pipeline combining a deterministic C++ core, a relational database, and a zero-dependency web interface.

```
┌───────────────────────┐
│     MySQL Server      │ ── (Schema, stops, routes, buses, stored ETAs)
└───────────┬───────────┘
            │ CLI Subprocess Pipe (No fragile external connector DLLs)
            ▼
┌───────────────────────┐
│   C++ Transit Core    │
│  (wheresmybus.exe)    │
│                       │
│  • DataLoader         │ ── Hydrates in-memory registries from MySQL
│  • Linked List Routes │ ── Sequential stop validation & turnaround navigation
│  • Queue Buffer       │ ── Buffers incoming conductor action requests
│  • Memento Stack      │ ── Multi-bus transactional update & selective undo
│  • Binary Heap        │ ── O(log N) display priority sorting for active fleet
│  • WebBridge Daemon   │ ── Dispatches JSON actions & updates atomic state
└───────────┬───────────┘
            │ Atomic file writes (state.json, student.html, conductor.html, hod.html)
            ▼
┌───────────────────────┐
│   Python Web Server   │ ── Built-in HTTP server (Port 8080)
│      (server.py)      │    Handles cross-device requests over LAN/Wi-Fi
└───────────┬───────────┘
            │ HTTP / JSON API
            ▼
┌────────────────────────────────────────────────────────────────────────┐
│                          User Interfaces                               │
│                                                                        │
│   [Student Portal]         [Conductor Portal]         [HOD Overview]   │
│   • Read-only stop query   • Shift management         • Fleet metrics  │
│   • 6-color status chips   • "Reached Next Stop"      • Problem alerts │
│   • Seat availability      • Free/Full seat toggle    • Turnaround log │
│   • Stored segment ETAs    • Maintenance reporter     • Audit trail    │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 3. User Roles & Permission Model

The platform strictly isolates privileges across three distinct roles:

```text
                       WHERE'S MY BUS
                              │
          ┌───────────────────┼───────────────────┐
          ▼                   ▼                   ▼
     [ STUDENT ]        [ CONDUCTOR ]          [ HOD ]
     Public Role        Operational Role    Supervisory Role
     View-Only          Per-Bus Updates     Privileged Fleet Audit
```

### 1. Student Portal (`student.html`)
- **Access Level**: Public, read-only (zero credentials required).
- **Core Functionality**:
  - Filter by any campus transit stop.
  - View all approaching buses servicing that stop.
  - View color status badges (approaching, passed, resting, maintenance).
  - Check real-time seat availability (`Free` vs `Full`).
  - View stored segment arrival times (ETAs).

### 2. Conductor Portal (`conductor.html`)
- **Access Level**: Operational (bus operators).
- **Core Functionality**:
  - Select assigned vehicle from fleet selector.
  - **Start Shift at College** (initiates outbound Drop journey from Campus).
  - **Start Shift at Rest Stop** (initiates inbound Pickup journey from Turnaround stop).
  - **Reached Next Stop**: Advances the vehicle to the next sequential node on its route.
  - **Seat Availability Toggle**: Switch between *Available, seats free* and *Available, bus full*.
  - **Maintenance Reporter**: Flag vehicle breakdown/maintenance or mark fixed.
  - **Undo Last Action**: Reverts the last operation for that specific bus using the Memento stack.
  - **End Shift**: Marks vehicle off-duty upon completing turnaround cycles.

### 3. Department Head / HOD Portal (`hod.html`)
- **Access Level**: Privileged administrative oversight.
- **Core Functionality**:
  - Real-time fleet health counters (`Total Fleet`, `In Service`, `Resting`, `Under Maintenance`, `Off Duty`).
  - Complete operational roster showing current stop, route, direction, and seat status for every bus.
  - Chronological transaction audit trail of all conductor updates with timestamps.

---

## 4. Color-Coded Status Specification

For every stop queried by a student, the status of every bus is determined using deterministic state-transition rules:

| Color Badge | Status Name | Operational Definition & Criteria |
| :---: | :--- | :--- |
| 🟢 **Green** | **Available, Seats Free** | Bus is actively running on the route (`IN_SERVICE`), has not yet reached this stop, and seats are reported free. |
| 🔵 **Blue** | **Available, Bus Full** | Bus is actively running on the route (`IN_SERVICE`), has not yet reached this stop, but is currently full. |
| 🔴 **Red** | **Departed / Passed** | Bus has already passed this stop on its current journey. |
| 🟤 **Brown** | **Resting at Stop** | Bus is currently positioned at its designated turnaround/rest stop (`RESTING`). |
| 🟣 **Purple** | **Maintenance** | Conductor has reported vehicle breakdown or maintenance (`MAINTENANCE`). |
| ⚫ **Black** | **Not Available** | Bus is off-duty (`NOT_AVAILABLE`) or does not service this stop. |

---

## 5. Course Data Structures & Technical Implementation

This project implements core computer science data structures and object-oriented paradigms from scratch or via standard C++ container adapters:

### 1. Singly Linked List (`Route` & `RouteStopNode`)
- **Purpose**: Represents the ordered physical stops comprising a transit route.
- **Implementation**: Hand-written singly linked list where each `RouteStopNode` encapsulates stop ID, stop name, stop type (`COLLEGE`, `STOP`, `REST`), sequential index, and segment ETA.
- **Key Algorithms**:
  - Sequential advancement (`findStopByIndex`, `findStopById`).
  - Bidirectional journey resolution (`findLastStopById` handles loop turnaround routes).
  - Zero-warning clean heap destruction via iterative node deallocation.

### 2. Queue (`Queue<T>`)
- **Purpose**: Buffers incoming asynchronous conductor actions.
- **Implementation**: Container adapter backed by `std::queue<T>`.
- **Properties**: Ensures strict FIFO (First-In, First-Out) execution order for status transitions, decoupling the web bridge from the status engine.

### 3. Stack & Memento Pattern (`Stack<T>`)
- **Purpose**: Maintains update history and enables selective multi-bus undo.
- **Implementation**: Container adapter backed by `std::stack<T>`.
- **Multi-Bus Selective Undo Algorithm**:
  When a conductor triggers "Undo" for Bus $B$:
  1. Inspects the stack without mutating it using `toVector()`.
  2. Finds the target bus update at depth $k$.
  3. Pops and temporarily stashes $k-1$ updates belonging to other buses.
  4. Pops the target update and restores Bus $B$'s previous state (position, direction, seats, status).
  5. Pushes the $k-1$ stashed updates back in exact chronological order.
  This allows one conductor to undo an erroneous update without corrupting other buses' records.

### 4. Binary Heap / Priority Queue (`BinaryHeap<T, Comp>`)
- **Purpose**: Sorts the fleet for student and HOD display in $O(N \log N)$ time.
- **Implementation**: Container adapter backed by `std::priority_queue<T, std::vector<T>, Comp>`.
- **Custom Display Priority Rules (`BusDisplayPriority`)**:
  1. **Active Fleet First**: Buses `IN_SERVICE` or `RESTING` outrank inactive/maintenance buses.
  2. **Route Progress**: Buses furthest along their route sequence index outrank earlier buses.
  3. **Alphanumeric Tiebreak**: Natural numeric ordering on bus IDs (e.g., Bus 9 before Bus 21 before Bus 118).

### 5. Hash Table Registries (`BusRegistry` & `RouteRegistry`)
- **Purpose**: Fast $O(1)$ lookups for buses and routes by ID.
- **Implementation**: Custom registries wrapping `std::unordered_map` with auxiliary order tracking to guarantee deterministic display ordering across compiler runs.

### 6. Object-Oriented Programming (OOP) Principles
- **Inheritance & Polymorphism**:
  - Abstract base class `User` defining common user attributes.
  - Concrete subclasses: `Student`, `Conductor`, `DepartmentHead`.
- **Encapsulation**: Strict private member variables with accessor/mutator methods across all domain classes (`Bus`, `Route`, `Stop`, `Update`).
- **Operator Overloading**: Dedicated `operator<<` overload for `Bus` objects enabling standardized stream logging.
- **Templates**: Type-parameterized data structures (`Queue<T>`, `Stack<T>`, `BinaryHeap<T, Compare>`).

---

## 6. Route Topology & Turnaround Loop Model

College routes follow a **Turnaround Loop Model** with two distinct operational phases:

```text
OUTBOUND (DROP JOURNEY):
[Graphic Era (Campus)] ──> [Intermediate Stops] ──> [Rest Stop (Turnaround)]
Direction = DROP, State transitions: NOT_AVAILABLE ──> IN_SERVICE ──> RESTING

INBOUND (PICKUP JOURNEY):
[Rest Stop (Turnaround)] ──> [Intermediate Stops] ──> [Graphic Era (Campus)]
Direction = PICKUP, State transitions: RESTING ──> IN_SERVICE ──> NOT_AVAILABLE
```

1. **Drop Journey**: In the morning/afternoon, the bus leaves Graphic Era (Stop 1) and travels outward to the designated Rest Stop. Upon arrival, the bus enters the `RESTING` state.
2. **Pickup Journey**: The conductor starts the return trip from the Rest Stop. The bus travels in reverse/inbound sequence back toward Graphic Era. Upon reaching the campus stop, the shift concludes.

---

## 7. Project File Structure

```
WheresMyBus/
├── README.md                 # Complete project documentation
├── Makefile                  # Build script for Linux / macOS / MSYS2
├── build.bat                 # One-click Windows compilation script
├── load_database.bat         # Automated MySQL schema & fleet loader
├── start_server.bat          # One-click launcher for the Python web platform
├── server.py                 # Multi-device HTTP server with REST action endpoints
├── config.example.txt        # Configuration template
├── config.txt                # Local environment configuration (MySQL credentials)
├── .gitignore                # Git exclusions (binaries, local configs)
│
├── backend/                  # C++17 Core Engine
│   ├── main.cpp              # Entry point and initialization sequence
│   ├── BinaryHeap.h          # STL-backed Binary Heap adapter
│   ├── Stack.h               # STL-backed Stack adapter with Memento support
│   ├── Queue.h               # STL-backed Queue adapter
│   ├── Bus.h / .cpp          # Bus domain entity, states, operator<<
│   ├── BusPriority.h         # Fleet priority comparator and heap sorting
│   ├── BusRegistry.h / .cpp  # Deterministic hash registry for bus fleet
│   ├── Route.h / .cpp        # Singly linked list for route management
│   ├── RouteStopNode.h / .cpp# Linked list node representing a route stop
│   ├── RouteRegistry.h / .cpp# Hash registry for route entities
│   ├── RouteValidation.h / .cpp# Route topology and sequence validation
│   ├── Stop.h / .cpp         # Transit stop entity (COLLEGE, STOP, REST)
│   ├── Update.h / .cpp       # Conductor transaction record and timestamps
│   ├── User.h / .cpp         # User polymorphic hierarchy (Student, Conductor, HOD)
│   ├── BusStatusEngine.h / .cpp # Core state machine and action pipeline
│   ├── Config.h / .cpp       # Key-value file parser
│   ├── Database.h / .cpp     # Robust MySQL CLI subprocess connector
│   ├── DataLoader.h / .cpp   # Database hydrator for domain registries
│   ├── HtmlGenerator.h / .cpp# Atomic HTML and JSON state writer
│   ├── ConsoleMenu.h / .cpp  # Interactive terminal menu (CLI fallback)
│   └── WebBridge.h / .cpp    # IPC daemon bridging Python HTTP server & C++ core
│
├── frontend/
│   └── style.css             # Unified responsive stylesheet, color chips & grid
│
├── sql/
│   ├── a.sql                 # MySQL schema, stops, routes, and segment ETAs
│   ├── buses.sql             # Fleet registration data (26 buses across 9 routes)
│   └── show_display.sql      # Diagnostic verification queries
│
└── output/                   # Generated web portal files
    ├── .gitkeep              # Directory placeholder
    ├── index.html            # Unified portal landing page
    ├── student.html          # Live student dashboard (auto-refreshing)
    ├── conductor.html        # Interactive conductor action console
    ├── hod.html              # Supervisory fleet health & audit report
    └── state.json            # Live state payload consumed by frontend auto-sync
```

---

## 8. Prerequisites & Installation Guide

### Prerequisites
1. **Operating System**: Windows 10/11 (Primary), Linux, or macOS.
2. **C++ Compiler**: GCC (`g++` 9.0+) with C++17 support (e.g., MinGW-w64 on Windows).
3. **Database**: MySQL Server 8.0+ or MariaDB 10.x.
4. **Python**: Python 3.8+ (for local web server).

---

### Step 1: Database Setup

Make sure your MySQL service is running, then populate the `where_my_bus` database using the automated script:

```cmd
load_database.bat
```

*Alternatively, execute `sql/a.sql` followed by `sql/buses.sql` inside MySQL Workbench or the MySQL CLI:*
```bash
mysql -u root -p < sql/a.sql
mysql -u root -p < sql/buses.sql
```

---

### Step 2: Configure Settings

Copy `config.example.txt` to `config.txt` and configure your credentials:

```ini
db_host=localhost
db_port=3306
db_user=root
db_password=your_mysql_password
db_name=where_my_bus
mysql_path=C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe
open_browser=no
```

> **Note on `mysql_path`**: If `mysql` is already accessible in your system `PATH`, you can simply set `mysql_path=mysql`.

---

### Step 3: Compile the C++ Engine

#### Windows (One-Click)
Double-click `build.bat` or run:
```cmd
build.bat
```

#### Linux / macOS / MSYS2
```bash
make
```

#### Manual Single-Line Compilation
The application compiles cleanly with zero external library links:
```cmd
g++ -std=c++17 -Wall -Wextra -Ibackend backend\*.cpp -o wheresmybus.exe
```

---

### Step 4: Run the Application

#### Option A: Web Platform (Recommended)
Double-click `start_server.bat` or execute:
```cmd
python server.py
```
This automatically launches the C++ backend daemon and starts the Python web server on port 8080.

- **Local Access**: Open `http://localhost:8080` in your browser.
- **Mobile / Cross-Device Access**: Connect your phone or tablet to the same Wi-Fi/hotspot and navigate to `http://<YOUR_LOCAL_IP>:8080` (printed in the terminal).

#### Option B: Terminal Console Menu (CLI Mode)
If you prefer running the interactive text menu directly inside the terminal:
```cmd
wheresmybus.exe --cli
```

---

## 9. Troubleshooting & FAQ

### 1. `Database connection failed`
- **Cause**: MySQL service is stopped or credentials in `config.txt` are incorrect.
- **Fix**: Verify your MySQL service is running via Windows Services (`services.msc`). Check `db_user` and `db_password` in `config.txt`.

### 2. `'mysql' is not recognized as an internal or external command`
- **Cause**: MySQL CLI binary is not in Windows system `PATH`.
- **Fix**: Provide the full absolute path in `config.txt`, e.g.:
  `mysql_path=C:\Program Files\MySQL\MySQL Server 8.0\bin\mysql.exe`

### 3. `Port 8080 is already in use`
- **Cause**: Another service or previous Python server instance is occupying port 8080.
- **Fix**: Open `server.py`, change `PORT = 8080` to `PORT = 8085`, and restart.

### 4. Build warning or compiler error with G++
- **Cause**: Outdated compiler version lacking C++17 support.
- **Fix**: Install modern MinGW-w64 (GCC 9.0+) or verify with `g++ --version`.

---

## 10. Academic Credits & Team

- **Course**: TCS 302 / TCS 307
- **Project Team**: **"Null Pointer"**
- **Core Focus**: Applied Data Structures, Algorithms, Systems Architecture, and Hardware-Independent Transit Modeling.
