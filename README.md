# WheresMyBus
Hardware Independent Bus Tracking System, created as a Btech 2nd year PBL assignment. A Transport Management System designed to handle transit anxiety at a lower cost than alternatives.

📌 Project Overview

In a traditional bus-tracking system, GPS hardware is often required to determine the real-time location of a bus. This project provides an alternative approach using software-based manual location updates.

The system maintains predefined bus routes and their stops. Each bus is assigned to a route, and its progress is updated by the conductor.


Bus/Conductor Login -> System identifies the Bus -> Assigned Route is loaded -> Current / Next Stop is displayed -> Conductor reaches the stop 
-> Presses "REACHED!" -> System moves to next stop -> Students see updated bus status



🎯 Objectives
Provide a simple college bus tracking system without GPS hardware.
Allow students to check which buses are available at a particular stop.
Allow conductors to update bus progress with a single button.
Prevent conductors from manually selecting incorrect stops.
Store routes, stops, buses and ETA information in a MySQL database.
Demonstrate practical implementation of C++, OOP, Data Structures and Algorithms.
Provide supervisors with system and bus statistics.




👥 User Types ->

The system has three interfaces, but only two require login.

1. 👨‍🎓 Student

Students do not need to log in.

They can:

View all available stops.
Select a stop.
See the buses serving that stop.
Check the current availability/status of buses.

2. 🚌 Bus / Conductor

The conductor logs in as the bus, rather than using a personal account.

For example:

Bus Number: 68
Route: R001

The dashboard displays the bus's next stop.

The conductor only needs to press:

        ┌───────────────┐
        │   REACHED!    │
        └───────────────┘

The system automatically advances the bus.

3. 👨‍💼 Supervisor / Department Head

The supervisor can access administrative information and statistics about the buses and routes.




🚦 Bus Status System

Students need a quick way to understand the status of a bus.

Status	Meaning
⚫ Black	Not available at this time
🟢 Green	Available and seats are free
🔵 Blue	Available but full
🔴 Red	Missed / already went ahead

The system can also represent operational states such as:

Resting
Damaged / malfunctioning
At service



🔐 Route Validation

Because the conductor only presses REACHED!, the software controls the route progression.

Route validation ensures that:

The bus has a valid assigned route.
The current stop belongs to that route.
The next stop is valid.
The bus cannot arbitrarily jump to another stop.

This helps maintain consistency between the bus's actual recorded progress and its predefined route.




🗄️ Database

The project uses MySQL for persistent data storage.




🖥️ Technology Stack

C++ : Used for the main application logic and DSA implementation.

OOP : Used to model buses, routes, stops and users as objects.

DSA : Used for route traversal, lookup and network processing.

HTML5 / CSS3 / JavaScript : Used for the web-based user interfaces.

MySQL : Used to store routes, stops, buses and ETA information.

Git / GitHub : Used for source-code management and collaboration.




📈 Future Scope

The system can later be expanded with:

Live traffic-based ETA
Automatic delay detection
More advanced route recommendations
Route visualization
Additional buses and routes
More detailed historical statistics
Mobile-friendly interfaces