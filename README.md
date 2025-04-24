# Event Organizer 📅 (C++20 Terminal App)

A self‑contained, in‑memory calendar system that demonstrates clean C++ design, data‑structure choice, and a fully interactive command‑line UI.  
Ideal for learning modern C++ idioms, practising SOLID principles, or as a starter kit for a larger scheduling tool.

---

## Table of Contents
1. [Key Features](#key-features)
2. [Demo](#demo)
3. [Build & Run](#build--run)
4. [Command Reference](#command-reference)
5. [Project Architecture](#project-architecture)
6. [Data Flow](#data-flow)
7. [Extending the App](#extending-the-app)
8. [Troubleshooting](#troubleshooting)
9. [Contributing](#contributing)
10. [License](#license)

---

## Key Features

| Category     | Details |
|--------------|---------|
| **Accounts** | Register, login, logout; auto‑incrementing user IDs; clear‑text passwords for demo (swap in bcrypt/Argon2). |
| **Events**   | Create public or private events; future‑date validation; free‑text descriptions; automatic ID assignment. |
| **RSVP**     | Any authenticated user with access can RSVP; stored as a `std::set<int>` of user IDs. |
| **Search**   | Case‑insensitive keyword search across titles and descriptions. |
| **Range Queries** | Fast upcoming‑event listing via a Binary Search Tree keyed by minute‑precision `DateTime`. |
| **Access Control** | Private events visible only to creator or attendees; enforced in all pathways. |
| **UI**       | Box‑drawing menus, tables, and cards for a pleasant text UX. |
| **Dependencies** | **None** beyond the C++20 standard library—works on Linux, macOS, Windows (MSVC/Clang/GCC). |


---

## Build & Run

### Prerequisites
* **C++20** compliant compiler (GCC 10+, Clang 12+, MSVC 19.29+).
* Make/CMake optional—single‑file build works out‑of‑the‑box.

### One‑liner
```bash
g++ -std=c++20 -Wall -O2 main.cpp -o event-organizer
./event-organizer
