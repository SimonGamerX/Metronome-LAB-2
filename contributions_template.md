# Group Member Contributions Report
## Lab 2 – Developing a Metronome Thing Using Your Raspberry Pi
**Course:** B8019 - Programming the Internet of Things  
**Semester:** Spring 2025  
**Instructor:** Professor Sumi Helal  
**Teaching Assistant:** Junjie Li (lijunjie@ufl.edu)  
**Date:** March 2026  

---

### Group Details
- **Group Name / Number:** [Insert Group Name/Number]
- **Members:**
  1. **Member 1:** [Full Name] - [Student ID / Email]
  2. **Member 2:** [Full Name] - [Student ID / Email]
  3. *(Add additional members if applicable)*

---

### Relative Contributions Overview

| Member Name | Percentage Contribution (%) | Primary Responsibilities |
| :--- | :---: | :--- |
| [Member 1 Name] | 50% | Embedded C++ application, WiringPi GPIO configuration, Hardware breadboard assembly, Timing algorithm in `metronome.cpp` |
| [Member 2 Name] | 50% | RESTful API endpoint integration (`cpprestsdk`), Web dashboard UI development (`index.html`, `style.css`, `app.js`), Testing & verification |
| **Total** | **100%** | |

---

### Detailed Breakdown of Work

#### 1. Hardware Circuit & Breadboard Setup
- Connected Red LED (GPIO 17) and Green LED (GPIO 27) with appropriate series current-limiting resistors.
- Connected Mode Pushbutton (GPIO 23) and Tap Pushbutton (GPIO 24) using software pull-down resistors (`PUD_DOWN`).
- Connected Raspberry Pi to development workstation.
- **Contributed by:** [Member Name(s)]

#### 2. Part 1 — Embedded Application & RESTful Service (C++)
- Implemented `metronome.cpp` with FIFO rolling window of size 4 (`beat_samples = 4`) and BPM calculation derived from tap deltas.
- Implemented multi-threaded LED blinking and button edge-detection loop with software debouncing (50ms).
- Implemented REST CRUD endpoints (`/bpm/`, `/bpm/min/`, `/bpm/max/`) with `cpprestsdk` and thread-safe atomic metrics.
- Configured CMake build system.
- **Contributed by:** [Member Name(s)]

#### 3. Part 2 — HTML Dashboard Application
- Created responsive web user interface featuring live metronome pendulum visualization and Play/Learn LED indicators.
- Implemented asynchronous API interactions using JavaScript `fetch` for GET, PUT, and DELETE operations.
- Added auto-refresh polling, custom tempo slider, preset buttons, and live activity log.
- **Contributed by:** [Member Name(s)]

#### 4. Testing, Integration, and Demonstration Preparation
- Verified end-to-end functionality between hardware button taps and web dashboard updates.
- Verified REST API CRUD functionality via `curl` and browser.
- Prepared and tested in-class demonstration workflow.
- **Contributed by:** [Member Name(s)]

---

### Signatures
- **[Member 1 Name]:** ____________________ &nbsp;&nbsp;&nbsp;&nbsp; **Date:** ____________
- **[Member 2 Name]:** ____________________ &nbsp;&nbsp;&nbsp;&nbsp; **Date:** ____________
