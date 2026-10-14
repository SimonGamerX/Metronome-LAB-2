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
│   ├── CMakeLists.txt              <- Multi-arch CMakeLists linking wiringPi & cpprestsdk
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
- **WiringPi Support**:
  - WiringPi installed from git (`git clone https://github.com/WiringPi/WiringPi.git` $\to$ `./build`).
  - `CMakeLists.txt` includes `/usr/local/include` and `/usr/local/lib` and links `wiringPi`.
- **REST API Endpoints (`main.cpp`, `rest.hpp`)**:
  - Uses `web::http::methods::DEL` instead of `DELETE` (required by `cpprestsdk` to avoid Win32 macro conflict).
  - Listens on `http://0.0.0.0:8080`.
  - Routes support both slash and non-slash variants:
    - `GET /bpm/`, `PUT /bpm/`: Current BPM.
    - `GET /bpm/min/`, `DEL /bpm/min/`: Lowest BPM set by user (not 0).
    - `GET /bpm/max/`, `DEL /bpm/max/`: Highest BPM set by user.
  - `rest::reply_json()` explicitly adds CORS headers (`Access-Control-Allow-Origin: *`, etc.) to every response to prevent browser fetch blocking.
- **Timing & Rolling Window (`metronome.hpp`, `metronome.cpp`)**:
  - Sample size set to `beat_samples = 4` (storing the 4 most recent taps).
  - Satisfies rule: *"At least 4 taps are required to create a valid BPM measurement. Notice you should utilize only periods between two presses."*
  - 4 taps produce 3 interval periods ($\Delta_1, \Delta_2, \Delta_3$).
  - BPM is calculated as:
    $$\text{BPM} = \text{round}\left(\frac{60000}{\text{avg\_delta\_ms}}\right)$$
  - Returns `0` if fewer than 4 taps are recorded, keeping the previous tempo intact.

---

## 5. Revision History

| Revision | Timestamp | Author | Summary of Changes |
| :---: | :---: | :---: | :--- |
| **v1.0** | 2026-10-12 | Antigravity AI | Initial full implementation: completed `part1/`, created `part2/` dashboard, `contributions_template.md`. |
| **v1.1** | 2026-10-12 | Antigravity AI | Restored `metronome/` folder to the professor's 100% original untouched skeleton. |
| **v1.2** | 2026-10-14 | Antigravity AI | Resolved CMake `find_package(cpprestsdk)` issue on Raspberry Pi OS Bookworm. |
| **v1.4** | 2026-10-14 | Antigravity AI | Configured `main.cpp` and `CMakeLists.txt` to link git-built `wiringPi`. |
| **v1.5** | 2026-10-14 | Antigravity AI | Fixed C++ REST SDK methods: changed `methods::DELETE` to `methods::DEL` and fixed lambda const qualifier issue in `main.cpp`. |
