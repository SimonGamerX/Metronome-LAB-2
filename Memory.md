# Project Memory & Revision Log

> **Purpose**: This file acts as the persistent project memory across sessions. It documents the project specifications, architectural decisions, hardware setups, current implementation status, and a verification checklist for ongoing revisioning.

---

## 1. Project Context & Objectives
- **Course**: B8019 - Programming the Internet of Things
- **Assignment**: Lab 2 – Developing a Metronome Thing Using Your Raspberry Pi
- **Instructors**: Prof. Sumi Helal & TA Junjie Li (`lijunjie@ufl.edu`)
- **Core Goal**: 
  - Build an embedded IoT digital metronome on Raspberry Pi using C++, `wiringPi`, and `cpprestsdk`.
  - Create a companion web dashboard (HTML/CSS/JS) to interact with the device's RESTful API.
  - Final deliverables: Submission zip with `part1/`, `part2/`, and a group contributions PDF.

---

## 2. Directory Structure & File Roles

```text
metronome/
├── Memory.md                       <- Persistent project memory & revision log
├── contributions_template.md       <- Group contribution report template for submission
│
├── metronome/                      <- PROFESSOR'S ORIGINAL STARTER SKELETON (UNTOUCHED)
│   ├── CMakeLists.txt              <- Original starter CMakeLists
│   ├── main.cpp                    <- Original starter main.cpp (dummy 1s blink & /answer)
│   ├── metronome.hpp               <- Original starter metronome.hpp (beat_samples = 3)
│   ├── rest.hpp                    <- Original starter rest.hpp
│   ├── Lab 2 – Developing... (1).pdf <- Lab specification PDF
│   └── Module 7- Lab 2... .pptx    <- Lecture slides guide
│
├── part1/                          <- COMPLETED IMPLEMENTATION (FOR COMPILATION & SUBMISSION)
│   ├── CMakeLists.txt              <- Multi-arch resilient CMakeLists linking metronome.cpp
│   ├── main.cpp                    <- Full GPIO, debouncing, threading & REST API
│   ├── metronome.hpp               <- Updated with beat_samples = 4
│   ├── metronome.cpp               <- Full tap timing & BPM calculation logic
│   └── rest.hpp                    <- Endpoint maker + CORS response helper
│
└── part2/                          <- COMPLETED HTML DASHBOARD (FOR SUBMISSION)
    ├── index.html                  <- Web dashboard layout & controls
    ├── style.css                   <- Dashboard styling & mechanical metronome skin
    └── app.js                      <- API client, pendulum animation & audio clicks
```

---

## 3. Hardware Setup & Pin Mapping

| Component | Pin (BCM GPIO) | Physical Header Pin | Function / Logic |
| :--- | :---: | :---: | :--- |
| **Red LED** | `GPIO 17` | Pin 11 | Pulses on tap during **Learn Mode**; OFF in Play Mode |
| **Green LED** | `GPIO 27` | Pin 13 | Blinks at current BPM (50% duty cycle) in **Play Mode**; OFF when BPM = 0 or in Learn Mode |
| **Mode Button (PB2)** | `GPIO 23` | Pin 16 | Toggles between Play Mode and Learn Mode (rising-edge triggered) |
| **Tap Button (PB1)** | `GPIO 24` | Pin 18 | Records tap timestamps in Learn Mode (rising-edge triggered) |
| **Pull-down Resistors** | Internal | — | Software enabled via `pullUpDnControl(pin, PUD_DOWN)` for both buttons |

---

## 4. Architecture & Key Technical Decisions

### Part One — Embedded Application (`part1/`)
- **Timing & Rolling Window (`metronome.hpp`, `metronome.cpp`)**:
  - Sample size set to `beat_samples = 4` (storing the 4 most recent taps).
  - Satisfies rule: *"At least 4 taps are required to create a valid BPM measurement. Notice you should utilize only periods between two presses."*
  - 4 taps produce 3 interval periods ($\Delta_1, \Delta_2, \Delta_3$).
  - BPM is calculated as:
    $$\text{BPM} = \text{round}\left(\frac{60000}{\text{avg\_delta\_ms}}\right)$$
  - Returns `0` if fewer than 4 taps are recorded, keeping the previous tempo intact.
- **REST API Endpoints (`main.cpp`, `rest.hpp`)**:
  - Listens on `http://0.0.0.0:8080`.
  - Routes support both slash and non-slash variants:
    - `GET /bpm/`, `PUT /bpm/`: Current BPM. PUT accepts raw integer or `{"bpm": X}`. Updates min/max if BPM > 0.
    - `GET /bpm/min/`, `DELETE /bpm/min/`: Lowest BPM set by user (not 0). DELETE resets to 0 ("none").
    - `GET /bpm/max/`, `DELETE /bpm/max/`: Highest BPM set by user. DELETE resets to 0 ("none").
  - `rest::reply_json()` explicitly adds CORS headers (`Access-Control-Allow-Origin: *`, etc.) to every response to prevent browser fetch blocking.
- **Threading & Synchronization (`main.cpp`)**:
  - `g_current_bpm`, `g_min_bpm`, `g_max_bpm`, `g_is_learn_mode`, `g_red_pulse` use `std::atomic` / `std::mutex` for thread safety across REST handlers, GPIO polling, and blinking thread.
  - Dedicated blinking thread operates with 10ms responsive sleep slices for instant reaction to tempo changes and tap pulses.
  - Button polling runs at 10ms with 50ms software debouncing and edge-detection.
- **Build System (`CMakeLists.txt`)**:
  - Multiarch compatible search for `cpprestsdk` (supports 64-bit `aarch64`, 32-bit `armhf`, Debian 12 Bookworm, and legacy Bullseye).
  - Compiles `main.cpp` and `metronome.cpp`.
  - Links `cpprestsdk::cpprest`, `wiringPi`, and `boost_system`.

### Part Two — Web Dashboard (`part2/`)
- **Files**: `index.html`, `style.css`, `app.js` (100% self-contained vanilla stack, no external CDN dependencies).
- **Features**:
  - Configurable Raspberry Pi URL (`http://<ip>:8080`) stored in `localStorage`.
  - Animated pendulum metronome visualizer synchronized to `currentBpm`.
  - Built-in optional audio tick sound via Web Audio API.
  - Real-time display for Current BPM, Min BPM, and Max BPM.
  - Controls: BPM slider (20–240), numeric input, and tempo preset buttons (Largo, Adagio, Andante, Moderato, Allegro, Presto).
  - Buttons to trigger `GET`, `PUT /bpm/`, `DELETE /bpm/min/`, and `DELETE /bpm/max/`.
  - Auto-refresh toggle (polls every 2 seconds) to sync with physical button taps on the Pi.
  - Live REST activity log console.

---

## 5. Ongoing Revision & Quality Checklist
*Review this checklist whenever reviewing or making additions:*

- [x] **Original starter preserved**: `metronome/` contains the professor's original code untouched.
- [x] **CMake Multiarch Search**: `part1/CMakeLists.txt` resolves `cpprestsdk` across both 32-bit and 64-bit Raspberry Pi OS.
- [x] **4-Tap Minimum Requirement**: Verified in `part1/metronome.cpp` line 44 (`m_beat_count < beat_samples` returns 0).
- [x] **Period-Based Calculation**: Verified in `part1/metronome.cpp` (uses deltas between successive taps).
- [x] **Min/Max Rule**: Verified in `part1/main.cpp` (`min` only tracks values $> 0$; DELETE resets to 0).
- [x] **LED States**: Verified in `part1/main.cpp` (Green LED OFF when BPM = 0; Red pulses only on tap during Learn mode).
- [x] **CORS Support**: Verified in `part1/rest.hpp` (OPTIONS + all responses carry `Access-Control-Allow-Origin: *`).
- [x] **Debouncing**: Verified in `part1/main.cpp` (50ms debounce window on rising edges).
- [x] **Zero Build Artifacts in `part1`**: Verified (clean source code).
- [x] **Offline Capability**: Verified (no external CDN links in `part2/index.html`).

---

## 6. Revision History

| Revision | Timestamp | Author | Summary of Changes |
| :---: | :---: | :---: | :--- |
| **v1.0** | 2026-10-12 | Antigravity AI | Initial full implementation: completed `part1/`, created `part2/` dashboard, `contributions_template.md`. |
| **v1.1** | 2026-10-12 | Antigravity AI | Restored `metronome/` folder to the professor's 100% original untouched skeleton so the user can easily compare original vs implemented code. |
| **v1.2** | 2026-10-14 | Antigravity AI | Resolved CMake `find_package(cpprestsdk)` issue on Raspberry Pi OS (Debian 12 Bookworm) by adding multi-arch prefix fallback paths. |
